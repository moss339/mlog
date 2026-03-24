#pragma once

#include "sink.h"
#include <string>
#include <memory>

namespace mlog {

class Formatter {
public:
    virtual ~Formatter() = default;
    virtual std::string format(const LogMessage& msg) = 0;
};

using FormatterPtr = std::unique_ptr<Formatter>;

class DefaultFormatter : public Formatter {
public:
    explicit DefaultFormatter(const std::string& pattern = "[{}] [{}] [{}] {}");
    std::string format(const LogMessage& msg) override;

    void set_pattern(const std::string& pattern) { pattern_ = pattern; }
    std::string get_pattern() const { return pattern_; }

private:
    std::string pattern_;
};

} // namespace mlog
