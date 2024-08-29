#include "scanner.hpp"

#include <quill/detail/LogMacros.h>
#include <string>

#include "logger.hpp"

namespace scanner {

Scanner::Scanner(models::Context& ctx) : ctx_(ctx) {
  static const fmt::format_string<std::string> kProfitFileTemplate =
      "spread/{}.csv";
  static const std::string kFormatPatternLog = "%(ascii_time),%(message)";

  LOG_INFO(ctx.main_logger, "Start constructor scanner!");

  common_logger_ = logger::make_logger("spread/all.csv", kFormatPatternLog);
  common_logger_->set_log_level(ctx.main_logger->log_level());

  for (const auto& [coin, _] : ctx_.coin_to_ctx) {
    auto filename = fmt::format(fmt::runtime(kProfitFileTemplate), coin);
    loggers_[coin] =
        logger::make_logger(std::move(filename), kFormatPatternLog);
    loggers_[coin]->set_log_level(ctx.main_logger->log_level());
  }
}

void Scanner::run() {
  LOG_DEBUG(ctx_.main_logger, "Start run scanner.");

  while (true) {
    LOG_DEBUG(common_logger_, "Start iteration.");
    for (const auto& [_, ctx_by_coin] : ctx_.coin_to_ctx) {
      for (int i = 0; i < ctx_by_coin.size(); ++i) {
        if (ctx_by_coin[i].ask != -1 && ctx_by_coin[i].bid != -1) {
          for (int j = i + 1; j < ctx_by_coin.size(); ++j) {
            check_profit(ctx_by_coin[i], ctx_by_coin[j]);
          }
        }
      }
    }
    LOG_DEBUG(common_logger_, "Finish iteration.");
    std::this_thread::sleep_for(ctx_.scan_frequency_ms);
  }
}

void Scanner::check_profit(const models::CoinContext& f,
                           const models::CoinContext& s) {
  if (s.ask == -1 || s.bid == -1) {
    return;
  }
  LOG_DEBUG(loggers_.at(s.coin), "Start check profit! [{:^5}: {} and {}]",
            s.coin, f.exchange, s.exchange);
  if (f.ask - s.bid > s.ask - f.bid &&
      f.ask - s.bid >= f.ask * ctx_.min_profit * 0.01) {
    log_spread(f, s);
  } else if (s.ask - f.bid >= f.ask - s.bid &&
             s.ask - f.bid >= s.ask * ctx_.min_profit * 0.01) {
    log_spread(s, f);
  }
}

void Scanner::log_spread(const models::CoinContext& maker,
                         const models::CoinContext& taker) {
  using namespace fmt::literals;

  if (maker.coin != taker.coin) {
    throw std::logic_error("maker.coin != taker.coin");
  }

  const auto& calc_stread = [](const Money& ask, const Money& bid) {
    return 100 * (ask - bid) / ask;
  };

  const auto spread = calc_stread(maker.ask, taker.bid);
  const auto diff_time =
      std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(
                   maker.ask_time - taker.bid_time)
                   .count());

  const auto log = fmt::format(
      (" {coin:^5}, {spread:^10.6f}\n"
       "{space:^30}, {space:^5}, {space:^10}, "
       "{exchange_maker:^10}, {ask_pure:^12.6f}, {ask_after_comm:^12.6f}, "
       "-{comm_maker:^6.4f}%, {ask_time:%Y-%m-%d %H:%M:%S}\n"
       "{space:^30}, {space:^5}, {space:^10}, "
       "{exchange_taker:^10}, {bid_pure:^12.6f}, {bid_after_comm:^12.6f}, "
       "+{comm_taker:^6.4f}%, {bid_time:%Y-%m-%d %H:%M:%S}, "
       "{diff_time}ms"),
      "exchange"_a = maker.exchange,        //
      "coin"_a = maker.coin,                //
      "spread"_a = spread,                  //
      "exchange_maker"_a = maker.exchange,  //
      "ask_pure"_a = maker.ask_pure,        //
      "ask_after_comm"_a = maker.ask,       //
      "comm_maker"_a = maker.comm_maker,    //
      "ask_time"_a = maker.ask_time,        //
      "exchange_taker"_a = taker.exchange,  //
      "bid_pure"_a = taker.bid_pure,        //
      "bid_after_comm"_a = taker.bid,       //
      "comm_taker"_a = taker.comm_taker,    //
      "bid_time"_a = taker.bid_time,        //
      "diff_time"_a = diff_time,            //
      "space"_a = "");
  LOG_INFO(loggers_.at(maker.coin), "{}", log);
  LOG_INFO(common_logger_, "{}", log);
}

}  // namespace scanner