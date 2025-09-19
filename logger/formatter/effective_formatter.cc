#include "effective_formatter.h"
#include "effective_msg.pb.h"
#include "../utils/sys_util.h"

namespace logger {
void EffectiveFomatter::Format(const LogMsg& msg, MemoryBuf* dest) {
   EffectiveMsg effective_msg;
   effective_msg.set_level(static_cast<int>(msg.level_));
   effective_msg.set_timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
   effective_msg.set_pid(GetProcessId());
   effective_msg.set_tip(GetThreadId());
   effective_msg.set_line(msg.location_.line_);
   effective_msg.set_full_name(msg.location_.file_name_.data(), msg.location_.file_name_.size());
   effective_msg.set_func_name(msg.location_.func_name_.data(), msg.location_.func_name_.size());
   effective_msg.set_log_info(msg.message_.data(), msg.message_.size());
   size_t len = effective_msg.ByteSizeLong();
   dest->resize(len);
   effective_msg.SerializeToArray(dest->data(), len);
}
}