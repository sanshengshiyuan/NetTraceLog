#pragma once

#include "fmt/core.h"
#include "../log_common.h"

namespace logger {
class Formatter {
public:
    virtual ~Formatter() = default;
    virtual void Format(const LogMsg& msg, MemoryBuf* dest) = 0;
};
}