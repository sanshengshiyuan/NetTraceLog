#pragma once

#include <time.h>
#include <stdio.h>

#include <cstdint>
#include <ctime>
#include <string>

namespace logger {
    size_t GetPageSize();

    size_t GetProcessId();

    size_t GetThreadId();

    void LocalTime(std::tm* time, std::time_t* now);

    std::string GenerateString(size_t size);

}