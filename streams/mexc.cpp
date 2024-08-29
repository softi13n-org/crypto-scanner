#include "mexc.hpp"

#include <chrono>

#include <quill/detail/LogMacros.h>
#include <boost/algorithm/string/replace.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>

#include "base_stream.hpp"

namespace stream {

namespace {

const std::string kInitMsgTemplate = R"({
  "method": "sub.depth.full",
  "param": {
    "symbol": "{}_USDT",
    "limit": 20
  }
})";

const std::string kPingMsg = R"({
  "method": "ping"
})";

void fill_bid(const boost::json::object& obj, models::CoinContext& coin_ctx) {
  const auto bids = obj.at("data").at("bids").as_array();
  if (bids.size()) {
    if (bids.at(0).at(0).if_double()) {
      coin_ctx.bid_pure = bids.at(0).at(0).as_double();
    } else {
      coin_ctx.bid_pure = bids.at(0).at(0).as_int64();
    }
    coin_ctx.bid = coin_ctx.bid_pure * (1. + coin_ctx.comm_taker * 0.01);
  }
}

void fill_ask(const boost::json::object& obj, models::CoinContext& coin_ctx) {
  const auto asks = obj.at("data").at("asks").as_array();
  if (asks.size()) {
    if (asks.at(0).at(0).if_double()) {
      coin_ctx.ask_pure = asks.at(0).at(0).as_double();
    } else {
      coin_ctx.ask_pure = asks.at(0).at(0).as_int64();
    }
    coin_ctx.ask = coin_ctx.ask_pure * (1. - coin_ctx.comm_maker * 0.01);
  }
}

bool check_deadline(std::chrono::steady_clock::time_point& prev_tp) {
  const auto& curr_tp = std::chrono::steady_clock::now();
  if (std::chrono::duration_cast<std::chrono::seconds>(curr_tp - prev_tp)
          .count() > 20) {
    prev_tp = curr_tp;
    return true;
  }
  return false;
}

}  // namespace

void RunMexcStream(models::CoinContext& coin_ctx,
                   quill::Logger* const& main_logger) {
  WebsocketBaseStream ws(coin_ctx, main_logger);
  ws.connect_domain();
  ws.ssl_handshake();
  ws.websocket_handshake();
  ws.websocket_control_callback();

  auto time_point = std::chrono::steady_clock::now();

  const auto init_msg =
      boost::replace_first_copy(kInitMsgTemplate, "{}", coin_ctx.coin);
  ws.write(init_msg);

  {
    const auto obj = ws.read();
    if (obj.at("channel") == "rs.sub.depth.full" &&
        obj.at("data") == "success") {
      LOG_DEBUG(main_logger, "{} Subscribe success", coin_ctx.to_str());
    } else {
      LOG_ERROR(main_logger, "{} Subscribe {}", coin_ctx.to_str(),
                obj.at("data").as_string().c_str());
      return;
    }
    ws.clear_buffer();
  }

  while (true) {
    if (check_deadline(time_point)) {
      ws.write(kPingMsg);
    }

    const auto obj = ws.read();
    if (obj.contains("channel") && obj.at("channel") == "push.depth.full") {
      fill_bid(obj, coin_ctx);
      fill_ask(obj, coin_ctx);

      if (coin_ctx.bid > 0 && coin_ctx.ask > 0) {
        LOG_DEBUG(coin_ctx.logger, "[bid={}; ask={}; exchange={}]",
                  coin_ctx.bid, coin_ctx.ask, coin_ctx.exchange);
      }

      const auto timestamp = obj.at("ts").as_int64();
      coin_ctx.ask_time = TimePoint(std::chrono::milliseconds(timestamp));
      coin_ctx.bid_time = coin_ctx.ask_time;
    } else if (obj.contains("channel") && obj.at("channel") == "pong") {
      LOG_DEBUG(main_logger, "Received pong msg! {}", coin_ctx.to_str());
    } else if (!obj.contains("channel") || obj.at("channel") != "clientId") {
      LOG_WARNING(main_logger, "{} unknown msg received: {}", coin_ctx.to_str(),
                  boost::json::serialize(obj));
    }
    ws.clear_buffer();
  }
}

}  // namespace stream
