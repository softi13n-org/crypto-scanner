#include "base_stream.hpp"

#include <quill/detail/LogMacros.h>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/json/parse.hpp>

namespace stream {

WebsocketBaseStream::WebsocketBaseStream(models::CoinContext& coin_ctx,
                                         quill::Logger* const& main_logger)
    : coin_ctx_(coin_ctx), main_logger_(main_logger) {
  LOG_INFO(main_logger_,
           "Starting stream. [coin={:^6}; exchange={:^10}; domain={:^20}; "
           "port={:^4}; target={:^30}]",
           coin_ctx_.coin, coin_ctx_.exchange, coin_ctx_.domain, coin_ctx_.port,
           coin_ctx_.target);
}

void WebsocketBaseStream::connect_domain() {
  const auto results = resolver_.resolve(coin_ctx_.domain, coin_ctx_.port);
  LOG_DEBUG(main_logger_, "Success resolve domain! {}", coin_ctx_.to_str());
  endpoint_ = asio::connect(get_lowest_layer(ws_), results);
  LOG_DEBUG(main_logger_, "Success connect domain! {}", coin_ctx_.to_str());
}

void WebsocketBaseStream::ssl_handshake() {
  // Set SNI Hostname (many hosts need this to handshake successfully)
  if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(),
                                coin_ctx_.domain.c_str()))
    throw beast::system_error(
        beast::error_code(static_cast<int>(::ERR_get_error()),
                          asio::error::get_ssl_category()),
        "Failed to set SNI Hostname");
  LOG_DEBUG(main_logger_, "Success set SNI Hostname! {}", coin_ctx_.to_str());

  // Perform the SSL handshake
  ws_.next_layer().handshake(asio::ssl::stream_base::client);
  LOG_DEBUG(main_logger_, "Success SSL handshake! {}", coin_ctx_.to_str());
}

void WebsocketBaseStream::websocket_handshake() {
  // Set a decorator to change the User-Agent of the handshake
  ws_.set_option(beast::websocket::stream_base::decorator(
      [](beast::websocket::request_type& req) {
        req.set(
            beast::http::field::user_agent,
            std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-coro");
      }));
  LOG_DEBUG(main_logger_, "Success change User-Agent! {}", coin_ctx_.to_str());

  LOG_INFO(main_logger_, "Starting websocket handshake. {}",
           coin_ctx_.to_str());
  ws_.handshake(coin_ctx_.domain + ':' + coin_ctx_.port, coin_ctx_.target);
  LOG_DEBUG(main_logger_, "Success websocket handshake! {}",
            coin_ctx_.to_str());
}

void WebsocketBaseStream::websocket_control_callback() {
  ws_.control_callback([this](const beast::websocket::frame_type& kind,
                              const boost::beast::string_view& payload) {
    if (kind == beast::websocket::frame_type::ping) {
      LOG_INFO(this->coin_ctx_.logger, "Received ping frame! {}",
               coin_ctx_.to_str());
      this->ws_.pong(beast::websocket::ping_data(payload));
      LOG_INFO(this->coin_ctx_.logger, "Success send pong frame! {}",
               coin_ctx_.to_str());
    } else if (kind == beast::websocket::frame_type::pong) {
      LOG_WARNING(this->main_logger_, "Received pong frame! {}",
                  coin_ctx_.to_str());
    } else if (kind == beast::websocket::frame_type::close) {
      LOG_WARNING(this->main_logger_, "Received close frame! {}",
                  coin_ctx_.to_str());
    }
  });
}

boost::json::object WebsocketBaseStream::read() {
  ws_.read(buffer_);
  const auto str = beast::buffers_to_string(buffer_.data());
  LOG_INFO(coin_ctx_.logger, "[{:^10}] Received msg: {}", coin_ctx_.exchange,
           str);
  return boost::json::parse(str).as_object();
}

void WebsocketBaseStream::clear_buffer() { buffer_.clear(); }

void WebsocketBaseStream::write(const std::string& msg) {
  LOG_INFO(coin_ctx_.logger, "[{:^10}] Starting send msg: {}",
           coin_ctx_.exchange, msg);
  ws_.write(asio::buffer(msg));
}

WebsocketBaseStream::~WebsocketBaseStream() {
  ws_.close(beast::websocket::close_code::normal);
  LOG_INFO(main_logger_, "Success closed websocket! {}", coin_ctx_.to_str());
}

}  // namespace stream