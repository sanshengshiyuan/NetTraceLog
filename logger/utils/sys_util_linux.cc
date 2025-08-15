#include "sys_util.h"

#include <unistd.h>
#include <time.h>

namespace logger {
    size_t GetPageSize() {
        return GetPageSize();
    }

    size_t GetProcessId() {
        return static_cast<size_t>(::getpid());
    }

    size_t GetThreadId() {
        return static_cast<size_t>(::gettid());
    }

    void LocalTime(std::tm* time, std::time_t* now) {
        localtime_r(now, time);
    }
}