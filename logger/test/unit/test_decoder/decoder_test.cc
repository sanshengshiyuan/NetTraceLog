#include "../../../logger/encrypt/encrypt.h"
#include "../../../logger/compress/compress.h"
#include "../../..//decode/decoder.h"

int main() {
  // 测试解码器
  logger::decoder::LogDecoder log_decoder("loggerdemo_20250825154739.log");
  log_decoder.Decode("test log data");
  return 0;
}