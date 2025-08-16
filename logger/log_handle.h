#pragma once

#include <initializer_list>
#include <atomic>
#include <vector>

#include "log_common.h"

namespace logger {
class LogSink;
using LogSinkPtr = std::shared_ptr<LogSink>;
using LogSinkPtrList = std::initializer_list<LogSinkPtr>;
using StringView = std::string_view;
struct SourceLocation;
class LogHandle {
public:
    explicit LogHandle(LogSinkPtr);
    explicit LogHandle(LogSinkPtrList);

    template <class It>
    LogHandle(It begin, It end) : LogHandle(LogSinkPtrList(begin, end)) {}

    ~LogHandle() = default;

    LogHandle(const LogHandle& other) = delete;
    LogHandle& operator = (const LogHandle& other) = delete;

    void SetLogLevel(LogLevel level);
    LogLevel GetLogLevel() const;

    void Log(LogLevel level, SourceLocation loc, StringView message);
protected:
    bool ShouldLog_(LogLevel level) const noexcept {
        return level >= this->log_level_ && !log_sinks_.empty();
    }

    void Log_(const LogMsg& message);
private:
    std::atomic<LogLevel> log_level_;
    std::vector<LogSinkPtr> log_sinks_;
};
}

