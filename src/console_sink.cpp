#include "mlog/console_sink.h"
#include "mlog/formatter.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>

namespace moss {
namespace mlog {

ConsoleSink::ConsoleSink(LogLevel level)
    : Sink(level) {
}

void ConsoleSink::write(const LogMessage& msg) {
    if (static_cast<LogLevel>(msg.level) < level_) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    auto now = std::chrono::system_clock::from_time_t(msg.timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::cout << "[" << std::setfill('0') << std::setw(6) << msg.sequence << "]"
              << "[" << log_level_to_string(static_cast<LogLevel>(msg.level)) << "]"
              << "[" << std::put_time(std::localtime(&msg.timestamp), "%Y-%m-%d %H:%M:%S")
              << "." << std::setfill('0') << std::setw(3) << ms.count() << "]"
              << "[" << msg.logger_name << "]";

    if (!msg.file.empty()) {
        std::cout << "[" << msg.file << ":" << msg.line << "]";
    }

    std::cout << " " << msg.message << std::endl;
}

void ConsoleSink::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::cout.flush();
}

}  // namespace mlog
}  // namespace moss
