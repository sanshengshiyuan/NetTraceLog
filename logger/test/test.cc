#include "../log_common.h"
#include "../log_handle.h"
#include "../mmap/mmap_aux.h"
#include "../sinks/sink.h"
#include "../sinks/console_sink.h"
#include "../formatter/formatter.h"
#include "../encrypt/encrypt.h"
#include "../context/context.h"
#include "../context/executor.h"
#include "../context/thread_pool.h"
#include "../sinks/effective_sink.h"
#include "../utils/sys_util.h"
#include "../utils/timer_count.h"
#include <filesystem>

namespace fs = std::filesystem;


int main() {
    TIMER_COUNT("main");
    std::shared_ptr<logger::LogSink> sink = std::make_shared<logger::ConsoleSink>();
    logger::EffectiveSink::Conf conf;
    fs::path outDir = fs::temp_directory_path() / "logger_test";
    std::error_code ec; fs::create_directories(outDir, ec);
    conf.dir = outDir.string();
    conf.prefix = "loggerdemo";
    conf.pub_key = "04827405069030E26A211C973C8710E6FBE79B5CAA364AC111FB171311902277537F8852EADD17EB339EB7CD0BA2490A58CDED2C702DFC1E"
    "FC7EDB544B869F039C";
    std::shared_ptr<logger::LogSink> effective_sink = std::make_shared<logger::EffectiveSink>(conf);
    logger::LogHandle handle({effective_sink});
    std::string str = logger::GenerateString(2000);
    
for (int i = 0; i < 200000; ++i) {
  handle.Log(logger::LogLevel::kInfo, logger::SourceLocation(__FILE__, __LINE__, 
                                  static_cast<const char*>(__FUNCTION__)), str);
  handle.Log(logger::LogLevel::kWarn, logger::SourceLocation(__FILE__, __LINE__, 
                                  static_cast<const char*>(__FUNCTION__)), str);
  handle.Log(logger::LogLevel::kError, logger::SourceLocation(__FILE__, __LINE__, 
                                  static_cast<const char*>(__FUNCTION__)), str);
}
    return 0;
}