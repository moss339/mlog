#include "mlog/formatter.h"
#include "mlog/log_level.h"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace moss {
namespace mlog {

DefaultFormatter::DefaultFormatter(const std::string& pattern)
    : pattern_(pattern) {
}

std::string DefaultFormatter::format(const LogMessage& msg) {
    std::ostringstream oss;

    for (size_t i = 0; i < pattern_.size(); ++i) {
        if (pattern_[i] == '{' && i + 1 < pattern_.size() && pattern_[i + 1] == '}') {
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
