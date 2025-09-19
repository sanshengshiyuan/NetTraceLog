#pragma once

#include <chrono>
#include <string>

#include "internal_log.h"

namespace logger {
class TimerCount {
public:
    TimerCount(std::string_view info) : info_(info), start_(std::chrono::steady_clock::now()) {}
    ~TimerCount() {
        auto end = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
        LOG_INFO("TimerCount: {} took {} microseconds", info_, diff.count());
    }

private:    
    std::string_view info_;
    std::chrono::time_point<std::chrono::steady_clock> start_;
};
}
#define _TIMER_CONCAT(a, b) a##b
#define TIMER_COUNT(info) logger::TimerCount _TIMER_CONCAT(timer_count, __LINE__)(info)
//#define TIMER_COUNT(info) void(0)
