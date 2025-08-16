#include "log_handle.h"
#include "sinks/sink.h"

namespace logger {
    LogHandle::LogHandle(LogSinkPtr sink) : log_level_(LogLevel::kInfo){
        log_sinks_.push_back(std::move(sink));
    }
    LogHandle::LogHandle(LogSinkPtrList sinks) : log_level_(LogLevel::kInfo){
        for (auto sink : sinks) {
            log_sinks_.push_back(std::move(sink));
        }
    }
    void LogHandle::SetLogLevel(LogLevel level) {
        this->log_level_ = level;
    }
    LogLevel LogHandle::GetLogLevel() const {
        return this->log_level_;
    }

    void LogHandle::Log(LogLevel level, SourceLocation loc, StringView message) {
        if (!ShouldLog_(level)) {
            return;
        }
        LogMsg msg(loc, level, message);
        Log_(msg);
    }

    void LogHandle::Log_(const LogMsg& log_msg) {
        for (auto& sink : log_sinks_) {
            sink->Log(log_msg);
        }
    }
}