#pragma once

#include <quill/Logger.h>

#define QUILL_ROOT_LOGGER_ONLY

namespace logger {

namespace impl {

inline const std::string kLoggerFormatPattern =
    "%(ascii_time) \t[%(thread)] \tLOG_%(level_name) "
    "\t%(pathname):%(lineno) \t%(message)";

}  // namespace impl

quill::Logger* init_root_logger();

quill::Logger* make_logger(
    const std::string& filename,
    const std::string& format_pattern = impl::kLoggerFormatPattern);

}  // namespace logger