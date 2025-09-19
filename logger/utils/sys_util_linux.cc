#include "sys_util.h"

#include <unistd.h>
#include <time.h>

namespace logger {
    size_t GetPageSize() {
        return static_cast<size_t>(::getpagesize());
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

    std::string GenerateString(size_t size) {
        std::string str;
        str.resize(size);
        for (size_t i = 0; i < size; ++i) {
            str[i] = 32 + rand() % (126 - 32 + 1);
        }
        return str;
    }
}