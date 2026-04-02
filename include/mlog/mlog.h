#pragma once

#include "logger.h"
#include "sink.h"
#include "console_sink.h"
#include "file_sink.h"
#include "log_level.h"
#include "formatter.h"
#include "config.h"
#include "async_queue.h"

#include <string>
#include <memory>

namespace moss {
namespace mlog {

static std::shared_ptr<Logger> default_logger;

inline std::shared_ptr<Logger> init_default(const LoggerConfig& config) {
    default_logger = LoggerFactory::instance().create_logger(config);
    return default_logger;
}

inline void set_default_logger(std::shared_ptr<Logger> logger) {
    default_logger = std::move(logger);
}

inline std::shared_ptr<Logger> get_default_logger() {
    if (!default_logger) {
        LoggerConfig config;
        config.name = "default";
        config.level = LogLevel::Info;
        config.async = true;
        default_logger = LoggerFactory::instance().create_logger(config);
    }
    return default_logger;
}

}  // namespace mlog
}  // namespace moss
