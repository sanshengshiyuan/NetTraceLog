#include "console_sink.h"

namespace logger {
    ConsoleSink::ConsoleSink() : formatter_(std::make_unique<DefaultFormatter>()) {}
    void ConsoleSink::Log(const LogMsg& message)  {
        MemoryBuf buf;
        formatter_->Format(message, &buf);
        fwrite(buf.data(), 1, buf.size(), stdout);
        fwrite("\n", 1, 1, stdout);
    }

    void ConsoleSink::SetFormatter(std::unique_ptr<Formatter> formatter) {
        this->formatter_ = std::move(formatter);
    }
}