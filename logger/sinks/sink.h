#pragma once

#include "../log_common.h"
#include "../formatter/formatter.h"
#include "../formatter/default_formatter.h"
#include "../formatter/effective_formatter.h"
#include "../compress/zlib_compress.h"
#include "../compress/zstd_compress.h"

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