#pragma once

#include "log_level.h"
#include <string>
#include <vector>
#include <memory>

namespace moss {
namespace mlog {

class Sink;

struct SinkConfig {
    std::string type;
    std::string file_path;
    LogLevel level = kDefaultLevel;
    size_t max_file_size = 10 * 1024 * 1024;
    int max_file_count = 5;
};

struct LoggerConfig {
    std::string name;
    LogLevel level = kDefaultLevel;
    bool async = true;
    size_t queue_capacity = 1024;
    std::vector<SinkConfig> sinks;
};

bool load_config_from_file(const std::string& path, LoggerConfig& config);

}  // namespace mlog
}  // namespace moss
