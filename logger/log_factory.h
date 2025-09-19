#pragma once
#include <memory>

namespace logger {
class VariedLogHandle;
class LogFactory {
public:
    LogFactory(const LogFactory& other) = delete;
    LogFactory& operator=(const LogFactory& other) = delete;

    static LogFactory& Instance();

    VariedLogHandle* GetLogHandle();

    void SetLogHandle(std::shared_ptr<VariedLogHandle> log_handle);
private:
    LogFactory();
private:
    std::shared_ptr<VariedLogHandle> log_handle_;
};
}