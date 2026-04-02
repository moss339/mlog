#pragma once

#include <string>
#include <cstdint>

namespace moss {
namespace mlog {

enum class LogLevel : uint8_t {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warn = 3,
    Error = 4,
    Fatal = 5,
    Off = 6
};

constexpr LogLevel kDefaultLevel = LogLevel::Info;

constexpr const char* log_level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info:  return "INFO";
        case LogLevel::Warn:  return "WARN";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Fatal: return "FATAL";
        case LogLevel::Off:   return "OFF";
        default:              return "UNKNOWN";
    }
}

inline LogLevel log_level_from_string(const std::string& str) {
    if (str == "trace" || str == "TRACE") return LogLevel::Trace;
    if (str == "debug" || str == "DEBUG") return LogLevel::Debug;
    if (str == "info"  || str == "INFO")  return LogLevel::Info;
    if (str == "warn"  || str == "WARN")  return LogLevel::Warn;
    if (str == "error" || str == "ERROR") return LogLevel::Error;
    if (str == "fatal" || str == "FATAL") return LogLevel::Fatal;
    if (str == "off"   || str == "OFF")   return LogLevel::Off;
    return kDefaultLevel;
}

constexpr bool is_valid_level(LogLevel level) {
    return level >= LogLevel::Trace && level <= LogLevel::Off;
}

}  // namespace mlog
}  // namespace moss
