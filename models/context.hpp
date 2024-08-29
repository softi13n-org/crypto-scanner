#pragma once

#include <chrono>
#include <string>
#include <unordered_map>

#include <quill/Logger.h>
#include <boost/noncopyable.hpp>

#include "common.hpp"

namespace models {

struct CoinContext {
  std::string domain;
  std::string port;
  std::string target;
  std::string coin;
  Exchange exchange;
  Percent comm_maker;
  Percent comm_taker;
  Money bid = -1;  // ask after commission
  Money ask = -1;  // bid after commission
  Money bid_pure = -1;
  Money ask_pure = -1;
  std::chrono::system_clock::time_point bid_time =
      std::chrono::system_clock::now();
  std::chrono::system_clock::time_point ask_time =
      std::chrono::system_clock::now();
  quill::Logger* logger = nullptr;

  CoinContext() = default;
  CoinContext(const CoinContext& other) = delete;
  CoinContext(CoinContext&& other) = default;

  std::string to_str() const {
    return fmt::format("[{:^5}: {:^10}]", coin, exchange);
  }
};

class Context : private boost::noncopyable {
 public:
  std::unordered_map<std::string, std::vector<CoinContext>> coin_to_ctx;
  Percent min_profit;
  quill::Logger* main_logger = nullptr;
  std::chrono::milliseconds scan_frequency_ms;

 public:
  explicit Context(const std::string& config_filename);

 private:
  void log_ctx_coin();
};

}  // namespace models