#pragma once

#include <unordered_map>

#include <quill/Logger.h>

#include "context.hpp"

namespace scanner {

class Scanner : private boost::noncopyable {
 private:
  std::unordered_map<std::string, quill::Logger*> loggers_;
  quill::Logger* common_logger_;
  models::Context& ctx_;

 public:
  Scanner(models::Context& ctx);
  void run();

 private:
  void check_profit(const models::CoinContext& f, const models::CoinContext& s);
  void log_spread(const models::CoinContext& maker,
                  const models::CoinContext& taker);
};

}  // namespace scanner