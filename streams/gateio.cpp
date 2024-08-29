#include "gateio.hpp"

#include <algorithm>

#include <quill/detail/LogMacros.h>
#include <boost/algorithm/string/replace.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>

#include "base_stream.hpp"

namespace stream {

namespace {

const std::string kInitMsgTemplate = R"({
  "channel" : "futures.book_ticker",
  "event": "subscribe",
  "payload" : [
    "{}_USDT"
  ]
})";

// enum class PureType {
//   kAsk,
//   kBid,
// };

// Money get_pure(const boost::json::array& arr, const PureType& pure_type) {
//   std::vector<Money> v;
//   v.reserve(arr.size());
//   for (const auto& elem : arr) {
//     v.push_back(std::stold(elem.at("p").as_string().c_str()));
//   }
//   if (pure_type == PureType::kAsk) {
//     return *std::min_element(v.begin(), v.end());
//   }
//   return *std::max_element(v.begin(), v.end());
// }

void fill_bid(const boost::json::object& obj, models::CoinContext& coin_ctx) {
  const auto bid = obj.at("result").at("b").as_string().c_str();
  const auto size = obj.at("result").at("B").as_int64();
  if (size > 0) {
    coin_ctx.bid_pure = -1;
    coin_ctx.bid = -1;
  } else {
    coin_ctx.bid_pure = std::stold(bid);
    coin_ctx.bid = coin_ctx.bid_pure * (1. + coin_ctx.comm_taker * 0.01);
  }
}

void fill_ask(const boost::json::object& obj, models::CoinContext& coin_ctx) {
  const auto ask = obj.at("result").at("a").as_string().c_str();
  const auto size = obj.at("result").at("A").as_int64();
  if (size > 0) {
    coin_ctx.ask_pure = -1;
    coin_ctx.ask = -1;
  } else {
    coin_ctx.ask_pure = std::stold(ask);
    coin_ctx.ask = coin_ctx.ask_pure * (1. - coin_ctx.comm_maker * 0.01);
  }
}

}  // namespace

void RunGateStream(models::CoinContext& coin_ctx,
                   quill::Logger* const& main_logger) {
  WebsocketBaseStream ws(coin_ctx, main_logger);
  ws.connect_domain();
  ws.ssl_handshake();
  ws.websocket_handshake();
  ws.websocket_control_callback();

  const auto init_msg =
      boost::replace_first_copy(kInitMsgTemplate, "{}", coin_ctx.coin);
  ws.write(init_msg);

  while (true) {
    const auto obj = ws.read();
    if (obj.contains("channel") && obj.at("channel") == "futures.book_ticker") {
      if (obj.at("event") == "update") {
        fill_bid(obj, coin_ctx);
        fill_ask(obj, coin_ctx);

        if (coin_ctx.bid > 0 && coin_ctx.ask > 0) {
          LOG_DEBUG(coin_ctx.logger, "[bid={}; ask={}; exchange={}]",
                    coin_ctx.bid, coin_ctx.ask, coin_ctx.exchange);
        }

        const auto timestamp = obj.at("result").at("t").as_int64();
        coin_ctx.ask_time = TimePoint(std::chrono::milliseconds(timestamp));
        coin_ctx.bid_time = coin_ctx.ask_time;

      } else if (obj.at("event") == "subscribe") {
        if (obj.at("result").if_object() &&
            obj.at("result").at("status") == "success") {
          LOG_DEBUG(main_logger, "{} Subscribe success", coin_ctx.to_str());
        } else {
          LOG_ERROR(main_logger, "{} {}", coin_ctx.to_str(),
                    obj.at("error").at("message").as_string().c_str());
          break;
        }
      }
    } else {
      LOG_WARNING(main_logger, "{} unknown msg received: {}", coin_ctx.to_str(),
                  boost::json::serialize(obj));
    }
    ws.clear_buffer();
  }
}

}  // namespace stream
