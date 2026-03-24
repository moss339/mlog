#pragma once

#include "sink.h"
#include <string>
#include <vector>
#include <mutex>
#include <fstream>

namespace mlog {

class FileSink : public Sink {
public:
    FileSink(const std::string& file_path, LogLevel level = kDefaultLevel);
    ~FileSink() override;

    void write(const LogMessage& msg) override;
    void flush() override;

    void set_max_file_size(size_t max_size) { max_file_size_ = max_size; }
    void set_max_file_count(int max_count) { max_file_count_ = max_count; }
    size_t get_max_file_size() const { return max_file_size_; }
    int get_max_file_count() const { return max_file_count_; }

private:
    void rotate_file();

    std::string file_path_;
    std::ofstream file_;
    std::mutex mutex_;
    size_t current_file_size_ = 0;
    size_t max_file_size_ = 10 * 1024 * 1024; // 10MB default
    int max_file_count_ = 5;
};

} // namespace mlog
