#include "mlog/file_sink.h"
#include "mlog/formatter.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>

namespace moss {
namespace mlog {

FileSink::FileSink(const std::string& file_path, LogLevel level)
    : Sink(level)
    , file_path_(file_path) {
    file_.open(file_path, std::ios::app);
    if (!file_.is_open()) {
        throw std::runtime_error("Failed to open log file: " + file_path);
    }
}

FileSink::~FileSink() {
    if (file_.is_open()) {
        file_.flush();
        file_.close();
    }
}

void FileSink::write(const LogMessage& msg) {
    if (static_cast<LogLevel>(msg.level) < level_) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    if (current_file_size_ >= max_file_size_) {
        rotate_file();
    }

    auto now = std::chrono::system_clock::from_time_t(msg.timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << "[" << std::setfill('0') << std::setw(6) << msg.sequence << "]"
        << "[" << log_level_to_string(static_cast<LogLevel>(msg.level)) << "]"
        << "[" << std::put_time(std::localtime(&msg.timestamp), "%Y-%m-%d %H:%M:%S")
        << "." << std::setfill('0') << std::setw(3) << ms.count() << "]"
        << "[" << msg.logger_name << "]";

    if (!msg.file.empty()) {
        oss << "[" << msg.file << ":" << msg.line << "]";
    }

    oss << " " << msg.message << "\n";

    std::string line = oss.str();
    file_ << line;
    current_file_size_ += line.size();
}

void FileSink::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_.is_open()) {
        file_.flush();
    }
}

void FileSink::rotate_file() {
    if (file_.is_open()) {
        file_.close();
    }

    // Remove oldest file if max_file_count exceeded
    std::string oldest = file_path_ + "." + std::to_string(max_file_count_);
    ::remove(oldest.c_str());

    // Shift files
    for (int i = max_file_count_ - 1; i > 0; --i) {
        std::string src = file_path_ + "." + std::to_string(i);
        std::string dst = file_path_ + "." + std::to_string(i + 1);
        ::rename(src.c_str(), dst.c_str());
    }

    // Rename current to .1
    std::string first = file_path_ + ".1";
    ::rename(file_path_.c_str(), first.c_str());

    // Reopen main file
    file_.open(file_path_, std::ios::app);
    current_file_size_ = 0;
}

}  // namespace mlog
}  // namespace moss
