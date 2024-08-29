#include "binance.hpp"

#include <quill/detail/LogMacros.h>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>

#include "base_stream.hpp"

namespace stream {

namespace {

void fill_bid(const boost::json::object& obj, models::CoinContext& coin_ctx) {
  const auto bids = obj.at("b").as_array();
  if (bids.size()) {
    coin_ctx.bid_pure = std::stold(bids.at(0).at(0).as_string().c_str());
    coin_ctx.bid = coin_ctx.bid_pure * (1. + coin_ctx.comm_taker * 0.01);
  }
}

void fill_ask(const boost::json::object& obj, models::CoinContext& coin_ctx) {
  const auto asks = obj.at("a").as_array();
  if (asks.size()) {
    coin_ctx.ask_pure = std::stold(asks.at(0).at(0).as_string().c_str());
    coin_ctx.ask = coin_ctx.ask_pure * (1. - coin_ctx.comm_maker * 0.01);
  }
}

}  // namespace

void RunBinanceStream(models::CoinContext& coin_ctx,
                      quill::Logger* const& main_logger) {
  WebsocketBaseStream ws(coin_ctx, main_logger);
  ws.connect_domain();
  ws.ssl_handshake();
  ws.websocket_handshake();
  ws.websocket_control_callback();

  while (true) {
    const auto obj = ws.read();
    if (obj.contains("e") && obj.at("e") == "depthUpdate") {
      fill_bid(obj, coin_ctx);
      fill_ask(obj, coin_ctx);

      if (coin_ctx.bid > 0 && coin_ctx.ask > 0) {
        LOG_DEBUG(coin_ctx.logger, "[bid={}; ask={}; exchange={}]",
                  coin_ctx.bid, coin_ctx.ask, coin_ctx.exchange);
      }

      const auto timestamp = obj.at("E").as_int64();
      coin_ctx.ask_time = TimePoint(std::chrono::milliseconds(timestamp));
      coin_ctx.bid_time = coin_ctx.ask_time;
    } else {
      LOG_WARNING(main_logger, "{} unknown msg received: {}", coin_ctx.to_str(),
                  boost::json::serialize(obj));
    }
    ws.clear_buffer();
  }
}

}  // namespace stream
