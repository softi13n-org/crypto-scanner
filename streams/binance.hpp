#pragma once

#include <quill/Logger.h>

#include "context.hpp"

namespace stream {

void RunBinanceStream(models::CoinContext& coin_ctx,
                      quill::Logger* const& main_logger);

}  // namespace stream
