#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <type_traits>

#define LOGGER_LEVEL_TRACE 0
#define LOGGER_LEVEL_DEBUG 1
#define LOGGER_LEVEL_INFO 2
#define LOGGER_LEVEL_WARN 3
#define LOGGER_LEVEL_ERROR 4
#define LOGGER_LEVEL_CRITICAL 5
#define LOGGER_LEVEL_OFF 6


namespace logger {
using StringView = std::string_view;
using MemoryBuf = std::string;

enum class LogLevel {
  kTrace = LOGGER_LEVEL_TRACE,
  kDebug = LOGGER_LEVEL_DEBUG,
  kInfo = LOGGER_LEVEL_INFO,
  kWarn = LOGGER_LEVEL_WARN,
  kError = LOGGER_LEVEL_ERROR,
  kFatal = LOGGER_LEVEL_CRITICAL,
  kOff = LOGGER_LEVEL_OFF
};

struct SourceLocation {
    constexpr SourceLocation() = default;
    SourceLocation(StringView file_name_in, int32_t line_in, StringView func_name) : 
    file_name_{file_name_in}, line_{line_in}, func_name_(func_name) {
        if (!file_name_in.empty()) {
            size_t pos = file_name_in.rfind('/');
            if (pos != std::string::npos) {
                file_name_ = file_name_in.substr(pos + 1);
            } else {
                pos = file_name_in.rfind('\\');
                if (pos != std::string::npos) {
                    file_name_ = file_name_in.substr(pos + 1);
                }
            }
        }
    }

    StringView file_name_;
    int32_t line_{0};
    StringView func_name_;
};
struct LogMsg {
    LogMsg(SourceLocation loc, LogLevel level, StringView msg) : location_{loc}, level_{level}, message_{msg} {}
    LogMsg(LogLevel level, StringView msg) : LogMsg(SourceLocation{}, level, msg) {}
    SourceLocation location_;
    LogLevel level_;
    StringView message_;
};
}