#pragma once

#include <quill/Logger.h>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/json/value.hpp>

#include "context.hpp"

namespace stream {

namespace asio = boost::asio;
namespace beast = boost::beast;

class WebsocketBaseStream : private boost::noncopyable {
 private:
  asio::io_context io_ctx_{};
  asio::ssl::context ssl_ctx_{asio::ssl::context::tlsv12_client};
  asio::ip::tcp::resolver resolver_{io_ctx_};
  asio::ip::tcp::endpoint endpoint_;

  beast::websocket::stream<beast::ssl_stream<asio::ip::tcp::socket>> ws_{
      io_ctx_, ssl_ctx_};
  beast::flat_buffer buffer_;

  models::CoinContext& coin_ctx_;

  quill::Logger* main_logger_;

 public:
  WebsocketBaseStream() = delete;
  explicit WebsocketBaseStream(models::CoinContext& coin_ctx,
                               quill::Logger* const& main_logger);

  void connect_domain();
  void ssl_handshake();
  void websocket_handshake();
  void websocket_control_callback();

  boost::json::object read();
  void clear_buffer();
  void write(const std::string& msg);

  ~WebsocketBaseStream();
};

}  // namespace stream