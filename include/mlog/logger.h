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

namespace detail {

inline bool is_printf_flag(char c) {
    return c == '-' || c == '+' || c == ' ' || c == '#' || c == '0';
}

inline bool is_printf_digit(char c) {
    return (c >= '0' && c <= '9') || c == '.';
}

inline bool is_printf_specifier(char c) {
    return c == 'd' || c == 'i' || c == 'u' || c == 'f' || c == 's' ||
           c == 'c' || c == 'x' || c == 'X' || c == 'p' || c == 'e' ||
           c == 'E' || c == 'g' || c == 'G' || c == 'o' || c == 'l';
}

inline std::string format_printf_value(const std::string& spec, const std::string& value) {
    if (spec == "d" || spec == "i" || spec == "u" || spec == "o" || spec == "x" || spec == "X") {
        try {
            long long int_val = std::stoll(value);
            char buf[64];
            if (spec == "d" || spec == "i") snprintf(buf, sizeof(buf), "%lld", int_val);
            else if (spec == "u") snprintf(buf, sizeof(buf), "%llu", (unsigned long long)int_val);
            else if (spec == "o") snprintf(buf, sizeof(buf), "%llo", (unsigned long long)int_val);
            else snprintf(buf, sizeof(buf), "%llx", (unsigned long long)int_val);
            return std::string(buf);
        } catch (...) { return value; }
    } else if (spec == "f" || spec == "F" || spec == "e" || spec == "E" || spec == "g" || spec == "G") {
        try {
            double f_val = std::stod(value);
            char buf[128];
            snprintf(buf, sizeof(buf), ("%" + spec).c_str(), f_val);
            return std::string(buf);
        } catch (...) { return value; }
    } else if (spec == "s") {
        return value;
    } else if (spec == "c") {
        try {
            char c = static_cast<char>(std::stoll(value));
            return std::string(1, c);
        } catch (...) { return value; }
    }
    return value;
}

}  // namespace detail

template<typename... Args>
static std::string vformat(const std::string& fmt, Args&&... args) {
    std::ostringstream oss;
    std::array<std::string, sizeof...(Args)> values = {format_arg(std::forward<Args>(args))...};
    size_t arg_idx = 0;
    size_t i = 0;

    while (i < fmt.size()) {
        // Handle {} placeholder (native mlog style)
        if (fmt[i] == '{' && i + 1 < fmt.size() && fmt[i + 1] == '}') {
            if (arg_idx < values.size()) {
                oss << values[arg_idx++];
            } else {
                oss << "{}";
            }
            i += 2;
            continue;
        }

        // Handle {N} indexed placeholder
        if (fmt[i] == '{') {
            size_t j = i + 1;
            while (j < fmt.size() && fmt[j] != '}') ++j;
            if (j < fmt.size()) {
                size_t idx = 0;
                try {
                    idx = std::stoul(fmt.substr(i + 1, j - i - 1));
                } catch (...) {}
                if (idx < values.size()) {
                    oss << values[idx];
                    arg_idx = idx + 1;
                }
                i = j + 1;
                continue;
            }
        }

        // Handle %% escape
        if (fmt[i] == '%' && i + 1 < fmt.size() && fmt[i + 1] == '%') {
            oss << '%';
            i += 2;
            continue;
        }

        // Handle printf-style % specifier
        if (fmt[i] == '%' && i + 1 < fmt.size()) {
            size_t j = i + 1;
            std::string spec;
            // Collect flags, width, precision, length, then specifier
            while (j < fmt.size() && detail::is_printf_flag(fmt[j])) { spec += fmt[j++]; }
            while (j < fmt.size() && detail::is_printf_digit(fmt[j])) { spec += fmt[j++]; }
            while (j < fmt.size() && (fmt[j] == 'h' || fmt[j] == 'l' || fmt[j] == 'L')) { spec += fmt[j++]; }
            if (j < fmt.size() && detail::is_printf_specifier(fmt[j])) {
                spec += fmt[j++];
                if (arg_idx < values.size()) {
                    oss << detail::format_printf_value(spec, values[arg_idx++]);
                }
                i = j;
                continue;
            }
        }

        oss << fmt[i++];
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
