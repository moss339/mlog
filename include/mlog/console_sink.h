#pragma once

#include "sink.h"
#include <iostream>
#include <mutex>

namespace mlog {

class ConsoleSink : public Sink {
public:
    explicit ConsoleSink(LogLevel level = kDefaultLevel);
    ~ConsoleSink() override = default;

    void write(const LogMessage& msg) override;
    void flush() override;

private:
    std::mutex mutex_;
};

} // namespace mlog
