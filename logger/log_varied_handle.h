#pragma once

#include "log_handle.h"

#include "fmt/core.h"

namespace logger {
class VariedHandle : public LogHandle {
public:
  using LogHandle::LogHandle;

  template <typename... Args>
  void Log(SourceLocation loc, LogLevel lvl, fmt::format_string<Args...> fmt, Args&&... args) {
    Log_(loc, lvl, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void Log(LogLevel lvl, fmt::format_string<Args...> fmt, Args&&... args) {
    Log(SourceLocation{}, lvl, fmt, std::forward<Args>(args)...);
  }

 private:
  template <typename... Args>
  void Log_(SourceLocation loc, LogLevel lvl, fmt::basic_string_view<char> fmt, Args&&... args) {
    if (!ShouldLog_(lvl)) {
      return;
    }
    // 在栈上分配内存，超过256字节会分配到堆上，线程安全，函数调用完毕后自动释放内存
    fmt::basic_memory_buffer<char, 256> buf;
    fmt::detail::vformat_to(buf, fmt, fmt::make_format_args(std::forward<Args>(args)...));
    LogMsg msg(loc, lvl, StringView(buf.data(), buf.size()));
    LogHandle::Log_(msg);
  }

};
}