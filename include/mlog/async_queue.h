#pragma once

#include <atomic>
#include <memory>
#include <cstddef>
#include <cstdint>
#include <string>

namespace moss {
namespace mlog {

struct LogMessage {
    uint64_t sequence;
    int level;
    std::string logger_name;
    std::string message;
    std::string file;
    int line;
    std::string function;
    time_t timestamp;
};

struct LogMessageNode {
    LogMessageNode* next;
    LogMessage* msg;
};

class AsyncQueue {
public:
    explicit AsyncQueue(size_t capacity = 1024);
    ~AsyncQueue();

    bool push(LogMessageNode* node);
    LogMessageNode* pop();
    bool empty() const;
    size_t size() const;

    void set_capacity(size_t capacity);
    size_t get_capacity() const { return capacity_; }

private:
    LogMessageNode* head_;
    std::atomic<LogMessageNode*> tail_;
    std::atomic<size_t> size_;
    size_t capacity_;

    static constexpr size_t kCacheLineSize = 64;
};

}  // namespace mlog
}  // namespace moss
