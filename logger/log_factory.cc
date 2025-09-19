#include "log_factory.h"

namespace logger {
LogFactory::LogFactory() {}

LogFactory& LogFactory::Instance() {
    static LogFactory instance;
    return instance;
}
VariedLogHandle* LogFactory::GetLogHandle() {
    return log_handle_.get();
}

void LogFactory::SetLogHandle(std::shared_ptr<VariedLogHandle> log_handle) {
    this->log_handle_ = log_handle;
}

}