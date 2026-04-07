#include "mlog/formatter.h"
#include "mlog/log_level.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

namespace moss {
namespace mlog {

DefaultFormatter::DefaultFormatter(const std::string& pattern)
    : pattern_(pattern.empty() ? "%H:%M:%S.%e [%l] %n: %m" : pattern) {
}

std::string DefaultFormatter::format(const LogMessage& msg) {
    std::ostringstream oss;

    for (size_t i = 0; i < pattern_.size(); ++i) {
        if (pattern_[i] == '%') {
            if (i + 1 < pattern_.size()) {
                char c = pattern_[i + 1];
                switch (c) {
                case 'd': {
                    if (i + 2 < pattern_.size() && pattern_[i + 2] == '{') {
                        size_t end = pattern_.find('}', i + 3);
                        if (end != std::string::npos) {
                            std::string sub_pattern = pattern_.substr(i + 3, end - i - 3);
                            if (!sub_pattern.empty()) {
                                auto now = std::chrono::system_clock::from_time_t(msg.timestamp);
                                std::time_t tm = std::chrono::system_clock::to_time_t(now);
                                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                                    now.time_since_epoch()) % 1000;
                                oss << std::put_time(std::localtime(&tm), sub_pattern.c_str());
                                if (!sub_pattern.empty() && sub_pattern.back() == '%') {
                                    oss << "." << std::setfill('0') << std::setw(3) << ms.count();
                                }
                            }
                            i = end + 1;
                            continue;
                        }
                    }
                    auto now = std::chrono::system_clock::from_time_t(msg.timestamp);
                    std::time_t tm = std::chrono::system_clock::to_time_t(now);
                    oss << std::put_time(std::localtime(&tm), "%Y-%m-%d %H:%M:%S");
                    i += 2;
                    continue;
                }
                case 'H':
                    oss << msg.sequence;
                    i += 2;
                    continue;
                case 'l':
                    oss << log_level_to_string(static_cast<LogLevel>(msg.level));
                    i += 2;
                    continue;
                case 'n':
                    oss << msg.logger_name;
                    i += 2;
                    continue;
                case 'm':
                    oss << msg.message;
                    i += 2;
                    continue;
                case 'f':
                    oss << msg.file;
                    i += 2;
                    continue;
                case 'L':
                    oss << msg.line;
                    i += 2;
                    continue;
                case 'e': {
                    auto now = std::chrono::system_clock::from_time_t(msg.timestamp);
                    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        now.time_since_epoch()) % 1000;
                    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                        now.time_since_epoch()) % 1000000000;
                    oss << std::setfill('0') << std::setw(9) << ns.count();
                    i += 2;
                    continue;
                }
                case '%':
                    oss << '%';
                    i += 2;
                    continue;
                }
            }
            oss << pattern_[i++];
        } else if (pattern_[i] == '{' && i + 1 < pattern_.size() && pattern_[i + 1] == '}') {
            // Legacy {} placeholder: output default format
            auto now = std::chrono::system_clock::from_time_t(msg.timestamp);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) % 1000;

            oss << "[" << std::setfill('0') << std::setw(6) << msg.sequence << "]"
                << "[" << log_level_to_string(static_cast<LogLevel>(msg.level)) << "]"
                << "[" << std::put_time(std::localtime(&msg.timestamp), "%Y-%m-%d %H:%M:%S")
                << "." << std::setfill('0') << std::setw(3) << ms.count() << "]"
                << "[" << msg.logger_name << "]";

            if (!msg.file.empty()) {
                oss << "[" << msg.file << ":" << msg.line << "]";
            }

            oss << " " << msg.message;
            ++i;
        } else {
            oss << pattern_[i];
        }
    }

    return oss.str();
}

}  // namespace mlog
}  // namespace moss
