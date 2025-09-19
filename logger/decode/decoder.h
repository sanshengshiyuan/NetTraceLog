#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <memory>

#include "compress/compress.h"
#include "encrypt/encrypt.h"
#include "../log_common.h"
#include "sinks/effective_sink.h"
#include "encrypt/aes_encrypt.h"

namespace logger {
namespace decoder {
class LogDecoder {
public:
  LogDecoder(std::filesystem::path log_path);

  void Decode(const std::string& log);

//   void DecodeChunk(const std::string& chunk);

//   void DecodeItem(const std::string& item);

//   void WriteToFile(const std::string& output_path);

private:
  void SetPrivateKey(const std::string& key) {
    pri_key_ = key;
  }
private:
  // 私有成员变量和方法
  std::unique_ptr<std::fstream> log_file_;
  std::unique_ptr<compress::Compression> decompress_; 
  std::unique_ptr<encrypt::Encryption> decrypt_;
  std::string pri_key_;
  std::string decompress_buf_;
  std::string decrypt_buf_;
};
}
}