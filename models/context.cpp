#include "context.hpp"

#include <fmt/format.h>
#include <quill/LogLevel.h>
#include <quill/Logger.h>
#include <quill/detail/LogMacros.h>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "logger.hpp"

namespace models {

namespace {

namespace pt = boost::property_tree;

template <typename T>
std::vector<T> as_vector(const pt::ptree& ptree,
                         const pt::ptree::key_type& key) {
  std::vector<T> result;
  for (const auto& item : ptree.get_child(key))
    result.push_back(item.second.get_value<T>());
  return result;
}

void set_log_level(std::string&& log_level, quill::Logger* logger) {
  boost::algorithm::to_lower(log_level);
  if (log_level == "debug") {
    logger->set_log_level(quill::LogLevel::Debug);
  } else if (log_level == "info") {
    logger->set_log_level(quill::LogLevel::Info);
  } else if (log_level == "warning") {
    logger->set_log_level(quill::LogLevel::Warning);
  } else if (log_level == "error") {
    logger->set_log_level(quill::LogLevel::Error);
  } else if (log_level == "crit" || log_level == "critical") {
    logger->set_log_level(quill::LogLevel::Critical);
  }
}

void fill_binance_context(CoinContext& context, const std::string& coin) {
  static const std::string kDomain = "fstream.binance.com";
  static const std::string kPort = "443";
  static const fmt::format_string<std::string> kTargetTemplate =
      "/ws/{}usdt@depth20@100ms";
  static const Exchange exchange = Exchange::kBinance;
  static const Percent kCommMaker = 0.02;
  static const Percent kCommTaker = 0.04;

  const auto target =
      fmt::format(kTargetTemplate, boost::algorithm::to_lower_copy(coin));
  context.domain = kDomain;
  context.port = kPort;
  context.target = target;
  context.coin = coin;
  context.exchange = exchange;
  context.comm_maker = kCommMaker;
  context.comm_taker = kCommTaker;
}

void fill_mexc_context(CoinContext& context, const std::string& coin) {
  static const std::string kDomain = "contract.mexc.com";
  static const std::string kPort = "443";
  static const std::string kTarget = "/ws";
  static const Exchange exchange = Exchange::kMexc;
  static const Percent kCommMaker = 0.00;
  static const Percent kCommTaker = 0.01;

  context.domain = kDomain;
  context.port = kPort;
  context.target = kTarget;
  context.coin = coin;
  context.exchange = exchange;
  context.comm_maker = kCommMaker;
  context.comm_taker = kCommTaker;
}

void fill_gate_context(CoinContext& context, const std::string& coin) {
  static const std::string kDomain = "fx-ws.gateio.ws";
  static const std::string kPort = "443";
  static const std::string kTarget = "/v4/ws/usdt";
  static const Exchange exchange = Exchange::kGate;
  static const Percent kCommMaker = 0.015;
  static const Percent kCommTaker = 0.05;

  context.domain = kDomain;
  context.port = kPort;
  context.target = kTarget;
  context.coin = coin;
  context.exchange = exchange;
  context.comm_maker = kCommMaker;
  context.comm_taker = kCommTaker;
}

}  // namespace

Context::Context(const std::string& config_filename)
    : main_logger(logger::init_root_logger()) {
  main_logger->set_log_level(quill::LogLevel::Debug);

  LOG_INFO(main_logger, "Start create context");

  pt::ptree config;
  pt::read_json(config_filename, config);

  set_log_level(config.get<std::string>("log_level"), main_logger);

  scan_frequency_ms =
      std::chrono::milliseconds(config.get<size_t>("scan_frequency_ms"));

  min_profit = config.get<Percent>("min_profit");
  auto coins = as_vector<std::string>(config, "coins");
  for (auto& coin : coins) {
    boost::algorithm::to_upper(coin);
  }
  const auto& exchanges = as_vector<std::string>(config, "exchanges");

  for (const auto& coin : coins) {
    auto logger = logger::make_logger("pure/" + coin + ".log");
    for (const auto& exchange : exchanges) {
      LOG_DEBUG(main_logger,
                "Start create coin context. [coin={}; exchange={}]", coin,
                exchange);

      coin_to_ctx[coin].push_back({});
      if (exchange == "binance") {
        fill_binance_context(coin_to_ctx[coin].back(), coin);
      } else if (exchange == "mexc") {
        fill_mexc_context(coin_to_ctx[coin].back(), coin);
      } else if (exchange == "gate" || exchange == "gateio") {
        fill_gate_context(coin_to_ctx[coin].back(), coin);
      }
      coin_to_ctx[coin].back().logger = logger;
    }
  }

  log_ctx_coin();
}

void Context::log_ctx_coin() {
  using namespace fmt::literals;
  for (const auto& [_, ctx_by_coin] : coin_to_ctx) {
    for (const auto& coin : ctx_by_coin) {
      const auto log = fmt::format(("{exchange},{domain},{coin},{target},{comm_"
                                    "maker:.4f},{comm_taker:.4f},{logger}"),
                                   "exchange"_a = coin.exchange,      //
                                   "domain"_a = coin.domain,          //
                                   "coin"_a = coin.coin,              //
                                   "target"_a = coin.target,          //
                                   "comm_maker"_a = coin.comm_maker,  //
                                   "comm_taker"_a = coin.comm_taker,  //
                                   "logger"_a = (uint64_t)coin.logger);
      LOG_DEBUG(main_logger, "{}", log);
    }
  }
}

}  // namespace models