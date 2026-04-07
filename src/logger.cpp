#include "mlog/logger.h"
#include "mlog/console_sink.h"
#include "mlog/file_sink.h"
#include <algorithm>
#include <cstring>
#include <iostream>

namespace moss {
namespace mlog {

static std::atomic<uint64_t> g_sequence{0};

static uint64_t next_sequence() {
    return g_sequence.fetch_add(1, std::memory_order_relaxed);
}

Logger::Logger(const LoggerConfig& config)
    : name_(config.name)
    , level_(config.level)
    , async_(config.async)
    , running_(true)
    , sequence_(0) {
    queue_ = std::make_unique<AsyncQueue>(config.queue_capacity);
    formatter_ = std::make_unique<DefaultFormatter>();

    for (const auto& sink_config : config.sinks) {
        SinkPtr sink;
        if (sink_config.type == "console") {
            sink = std::make_unique<ConsoleSink>(sink_config.level);
        } else if (sink_config.type == "file") {
            auto file_sink = std::make_unique<FileSink>(sink_config.file_path, sink_config.level);
            file_sink->set_max_file_size(sink_config.max_file_size);
            file_sink->set_max_file_count(sink_config.max_file_count);
            sink = std::move(file_sink);
        }
        if (sink) {
            sinks_.push_back(std::move(sink));
        }
    }

    if (sinks_.empty()) {
        sinks_.push_back(std::make_unique<ConsoleSink>(level_));
    }

    if (async_) {
        worker_thread_ = std::thread(&Logger::async_worker, this);
    }
}

Logger::~Logger() {
    stop();
}

void Logger::log(LogLevel level, const std::string& msg) {
    if (level < level_) {
        return;
    }

    LogMessage log_msg;
    log_msg.sequence = next_sequence();
    log_msg.level = static_cast<int>(level);
    log_msg.logger_name = name_;
    log_msg.message = msg;
    log_msg.file = "";
    log_msg.line = 0;
    log_msg.function = "";
    log_msg.timestamp = time(nullptr);

    if (async_) {
        enqueue_message(std::move(log_msg));
    } else {
        std::lock_guard<std::mutex> lock(sinks_mutex_);
        for (auto& sink : sinks_) {
            sink->write(log_msg);
        }
    }
}

void Logger::trace(const std::string& msg) { log(LogLevel::Trace, msg); }
void Logger::debug(const std::string& msg) { log(LogLevel::Debug, msg); }
void Logger::info(const std::string& msg)  { log(LogLevel::Info, msg); }
void Logger::warn(const std::string& msg)  { log(LogLevel::Warn, msg); }
void Logger::error(const std::string& msg) { log(LogLevel::Error, msg); }
void Logger::fatal(const std::string& msg) { log(LogLevel::Fatal, msg); }

void Logger::flush() {
    std::lock_guard<std::mutex> lock(sinks_mutex_);
    for (auto& sink : sinks_) {
        sink->flush();
    }
}

void Logger::stop() {
    bool expected = true;
    if (!running_.compare_exchange_strong(expected, false)) {
        return;
    }

    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
}

void Logger::async_worker() {
    std::unique_lock<std::mutex> lock(cv_mutex_);
    while (running_.load(std::memory_order_acquire)) {
        if (!queue_->empty()) {
            lock.unlock();
            LogMessageNode* node = queue_->pop();
            if (node) {
                if (node->msg) {
                    std::lock_guard<std::mutex> slock(sinks_mutex_);
                    for (auto& sink : sinks_) {
                        sink->write(*node->msg);
                    }
                    delete node->msg;
                }
                delete node;
            }
            lock.lock();
            continue;
        }

        cv_.wait_for(lock, std::chrono::milliseconds(100),
            [this] { return !running_.load(std::memory_order_acquire) || !queue_->empty(); });
    }
    lock.unlock();

    // Drain remaining messages
    while (!queue_->empty()) {
        LogMessageNode* node = queue_->pop();
        if (node) {
            if (node->msg) {
                std::lock_guard<std::mutex> lock(sinks_mutex_);
                for (auto& sink : sinks_) {
                    sink->write(*node->msg);
                }
                delete node->msg;
            }
            delete node;
        }
    }

    flush();
}

void Logger::enqueue_message(LogMessage&& msg) {
    LogMessageNode* node = new LogMessageNode;
    node->next = nullptr;
    node->msg = new LogMessage(std::move(msg));

    queue_->push(node);
    cv_.notify_one();
}

LoggerFactory& LoggerFactory::instance() {
    static LoggerFactory factory;
    return factory;
}

std::shared_ptr<Logger> LoggerFactory::create_logger(const LoggerConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto logger = std::make_shared<Logger>(config);
    loggers_[config.name] = logger;
    return logger;
}

std::shared_ptr<Logger> LoggerFactory::get_logger(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = loggers_.find(name);
    if (it != loggers_.end()) {
        return it->second;
    }
    return nullptr;
}

bool LoggerFactory::load_config(const std::string& path) {
    LoggerConfig config;
    if (!mlog::load_config_from_file(path, config)) {
        return false;
    }
    create_logger(config);
    return true;
}

void LoggerFactory::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& pair : loggers_) {
        pair.second->stop();
    }
    loggers_.clear();
}

}  // namespace mlog
}  // namespace moss
