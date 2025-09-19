#pragma once

#include "formatter.h"

namespace logger {
class EffectiveFomatter : public Formatter {
public:
    void Format(const LogMsg& msg, MemoryBuf* dest) override;
};
}