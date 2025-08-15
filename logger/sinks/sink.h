#pragma once

#include "../log_common.h"
#include "../formatter/formatter.h"
#include "../formatter/default_formatter.h"

namespace logger {
class LogSink {
public:
    LogSink() = default;
    virtual ~LogSink() = default;

    LogSink(const LogSink& other) = delete;
    LogSink& operator = (const LogSink& other) = delete;

    virtual void Log(const LogMsg& message) = 0;

    virtual void SetFormatter(std::unique_ptr<Formatter> formatter) = 0;

    virtual void Flush() {}
};
}