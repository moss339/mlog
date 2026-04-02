#pragma once

#include "config.h"
#include "sink.h"
#include "formatter.h"
#include "async_queue.h"
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <mutex>
#include <sstream>

namespace moss {
namespace mlog {

template<typename T>
static std::string format_arg(T&& arg) {
    std::ostringstream oss;
    oss << arg;
    return oss.str();
}

template<typename T>
static void format_to_string(std::ostringstream& oss, const std::string& fmt, size_t& i, T&& arg) {
    if (i >= fmt.size()) return;

    if (fmt[i] == '{' && i + 1 < fmt.size() && fmt[i + 1] == '}') {
        oss << format_arg(std::forward<T>(arg));
        i += 2;
    } else {
        oss << fmt[i++];
    }
}

template<typename T, typename... Args>
static void format_to_string(std::ostringstream& oss, const std::string& fmt, size_t& i, T&& arg, Args&&... args) {
    while (i < fmt.size()) {
        if (fmt[i] == '{' && i + 1 < fmt.size() && fmt[i + 1] == '}') {
            oss << format_arg(std::forward<T>(arg));
            i += 2;
            if constexpr (sizeof...(args) > 0) {
                format_to_string(oss, fmt, i, std::forward<Args>(args)...);
            }
            break;
        } else {
            oss << fmt[i++];
        }
    }
}

template<typename... Args>
static std::string vformat(const std::string& fmt, Args&&... args) {
    std::ostringstream oss;
    size_t placeholder_count = 0;
    std::string current;
    std::array<std::string, sizeof...(Args)> values = {format_arg(std::forward<Args>(args))...};

    for (size_t i = 0; i < fmt.size(); ++i) {
        if (fmt[i] == '{' && i + 1 < fmt.size() && fmt[i + 1] == '}') {
            if (placeholder_count < values.size()) {
                oss << values[placeholder_count++];
            } else {
                oss << "{}";
            }
            ++i;
        } else {
            oss << fmt[i];
        }
    }

    return oss.str();
}

class Logger {
public:
    explicit Logger(const LoggerConfig& config);
    ~Logger();

    void trace(const std::string& msg);
    void debug(const std::string& msg);
    void info(const std::string& msg);
    void warn(const std::string& msg);
    void error(const std::string& msg);
    void fatal(const std::string& msg);

    template<typename... Args>
    void trace(const std::string& fmt, Args&&... args) {
        trace(vformat(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void debug(const std::string& fmt, Args&&... args) {
        debug(vformat(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void info(const std::string& fmt, Args&&... args) {
        info(vformat(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void warn(const std::string& fmt, Args&&... args) {
        warn(vformat(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void error(const std::string& fmt, Args&&... args) {
        error(vformat(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void fatal(const std::string& fmt, Args&&... args) {
        fatal(vformat(fmt, std::forward<Args>(args)...));
    }

    void log(LogLevel level, const std::string& msg);

    const std::string& name() const { return name_; }
    LogLevel level() const { return level_; }
    void set_level(LogLevel level) { level_ = level; }

    void flush();
    void stop();

private:
    void async_worker();
    void enqueue_message(LogMessage&& msg);

    std::string name_;
    LogLevel level_;
    bool async_;
    std::unique_ptr<AsyncQueue> queue_;
    std::vector<SinkPtr> sinks_;
    std::unique_ptr<Formatter> formatter_;
    std::thread worker_thread_;
    std::atomic<bool> running_;
    std::atomic<uint64_t> sequence_;
    std::mutex sinks_mutex_;
};

class LoggerFactory {
public:
    static LoggerFactory& instance();

    std::shared_ptr<Logger> create_logger(const LoggerConfig& config);
    std::shared_ptr<Logger> get_logger(const std::string& name);

    bool load_config(const std::string& path);
    void clear();

private:
    LoggerFactory() = default;

    std::unordered_map<std::string, std::shared_ptr<Logger>> loggers_;
    std::mutex mutex_;
};

}  // namespace mlog
}  // namespace moss
