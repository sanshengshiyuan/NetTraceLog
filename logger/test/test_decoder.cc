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
#include "../../../decode/decoder.h"
#include <filesystem>

namespace fs = std::filesystem;


int main() {
  namespace fs = std::filesystem;
  fs::path dir = "/tmp/logger_test";
  fs::path latest;
  std::filesystem::file_time_type latest_time{};
  for (auto& p : fs::directory_iterator(dir)) {
    if (p.path().extension() == ".log") {
      auto t = fs::last_write_time(p.path());
      if (latest.empty() || t > latest_time) {
        latest = p.path();
        latest_time = t;
      }
    }
  }
  if (latest.empty()) {
    throw std::runtime_error("No .log files found in /tmp/logger_test");
  }
  logger::decoder::LogDecoder log_decoder(latest.string());
  log_decoder.Decode("test log data");
  return 0;
}