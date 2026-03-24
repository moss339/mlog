#pragma once

#include "log_level.h"
#include "async_queue.h"
#include <string>
#include <memory>

namespace mlog {

class Sink {
public:
    virtual ~Sink() = default;

    virtual void write(const LogMessage& msg) = 0;
    virtual void flush() = 0;
    virtual void set_level(LogLevel level) { level_ = level; }
    virtual LogLevel get_level() const { return level_; }

protected:
    explicit Sink(LogLevel level) : level_(level) {}

    LogLevel level_;
};

using SinkPtr = std::unique_ptr<Sink>;

} // namespace mlog
