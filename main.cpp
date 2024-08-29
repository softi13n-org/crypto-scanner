#include <thread>

#include <quill/detail/LogMacros.h>

#include "context.hpp"
#include "streams/binance.hpp"
#include "streams/gateio.hpp"
#include "streams/mexc.hpp"
#include "utils/scanner.hpp"

namespace {

void run_stream(models::CoinContext& coin_ctx,
                quill::Logger* const& main_logger) {
  if (coin_ctx.exchange == Exchange::kBinance) {
    stream::RunBinanceStream(coin_ctx, main_logger);
  } else if (coin_ctx.exchange == Exchange::kMexc) {
    stream::RunMexcStream(coin_ctx, main_logger);
  } else if (coin_ctx.exchange == Exchange::kGate) {
    stream::RunGateStream(coin_ctx, main_logger);
  }
}

void run_scanner(models::Context& ctx) {
  scanner::Scanner scanner(ctx);
  scanner.run();
}

}  // namespace

int main() {
  models::Context ctx("config.json");

  LOG_INFO(ctx.main_logger, "Start main!");

  std::vector<std::thread> threads;
  threads.reserve(ctx.coin_to_ctx.size() + 1);

  for (auto& [_, ctx_by_coin] : ctx.coin_to_ctx) {
    for (models::CoinContext& coin_ctx : ctx_by_coin) {
      LOG_INFO(ctx.main_logger, "Starting stream. {}!", coin_ctx.to_str());

      threads.emplace_back(run_stream, std::ref(coin_ctx), ctx.main_logger);
      threads.back().detach();
    }
  }

  threads.emplace_back(run_scanner, std::ref(ctx));
  threads.back().join();

  return EXIT_SUCCESS;
}
