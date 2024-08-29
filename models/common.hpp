#pragma once

#include <fmt/core.h>
#include <chrono>

using TimePoint = std::chrono::system_clock::time_point;

typedef long double Money;
typedef long double Percent;

enum class Exchange {
  kBinance,
  kMexc,
  kGate,
};

template <>
struct fmt::formatter<Exchange> : fmt::formatter<std::string_view> {
  template <typename FormatContext>
  auto format(const Exchange& exchange, FormatContext& ctx) {
    switch (exchange) {
      case Exchange::kBinance:
        return fmt::formatter<std::string_view>::format("Binance", ctx);
      case Exchange::kGate:
        return fmt::formatter<std::string_view>::format("Gate.io", ctx);
      case Exchange::kMexc:
        return fmt::formatter<std::string_view>::format("Mexc", ctx);
    }
  }
};
