#include "logger.hpp"

#include <quill/Config.h>
#include <quill/Logger.h>
#include <quill/Quill.h>

namespace logger {

using namespace logger::impl;

namespace {

const std::string kLoggerTimestampFormat = "%Y-%m-%d %H:%M:%S.%Qus %Z";
const quill::Timezone kLoggerTimezone = quill::Timezone::GmtTime;

}  // namespace

quill::Logger* init_root_logger() {
  std::shared_ptr<quill::Handler> file_handler =
      quill::file_handler("logs/main.log", "w");
  file_handler->set_pattern(kLoggerFormatPattern, kLoggerTimestampFormat,
                            kLoggerTimezone);

  // set configuration
  quill::Config cfg;
  cfg.default_handlers.push_back(file_handler);

  // Apply configuration and start the backend worker thread
  quill::configure(cfg);
  quill::start();

  return quill::get_root_logger();
}

quill::Logger* make_logger(const std::string& filename,
                           const std::string& format_pattern) {
  std::shared_ptr<quill::Handler> file_handler =
      quill::file_handler(fmt::format("logs/{}", filename), "w");
  file_handler->set_pattern(format_pattern, kLoggerTimestampFormat,
                            kLoggerTimezone);
  return quill::create_logger(filename, std::move(file_handler));
}

}  // namespace logger