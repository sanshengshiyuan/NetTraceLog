#include "decoder.h"
#include <vector>

namespace logger {
namespace decoder {
LogDecoder::LogDecoder(std::filesystem::path log_path) {
  log_file_ = std::make_unique<std::fstream>(log_path, std::ios::in | std::ios::binary);
  if (!log_file_ || !log_file_->is_open()) {
    throw std::runtime_error("LogDecoder: failed to open log file");
  }
  decompress_ = std::make_unique<compress::ZlibCompress>();
  decompress_->ResetStream();
  decrypt_.reset();
  decompress_buf_.reserve(10 * 1024 * 1024); // 10MB缓冲区
  decrypt_buf_.reserve(10 * 1024 * 1024);   // 10MB缓冲区
  pri_key_ = logger::encrypt::HexKeyToBinary("FAA5BBE9017C96BF641D19D0144661885E831B5DDF52539EF1AB4790C05E665E");
}
void LogDecoder::Decode(const std::string& log) {
  // 解码日志的实现
  detail::ChunkHeader chunk_header;
  log_file_->read(reinterpret_cast<char*>(&chunk_header), sizeof(chunk_header));
  if (chunk_header.magic == detail::ChunkHeader::kMagic) {
    // 处理有效的 Chunk
    LOG_INFO("Decoding chunk of size: {}", chunk_header.size);
    
  } else {
    // 报错错误
    std::cerr << "Chunk magic number mismatch\n";
    return;
  }
  //查看共享密钥字节数
  LOG_INFO("Chunk pub key size: {}", sizeof(chunk_header.pub_key));
  LOG_INFO("Chunk pub key size: {}", std::string(chunk_header.pub_key, 65).size());
  std::string shared_key = encrypt::GenECDHSharedSecret(pri_key_, std::string(chunk_header.pub_key, 65));
  if (shared_key.size() != 32) {
    throw std::runtime_error("ECDH shared key length error: must be 32 bytes for AES-256");
}
  decrypt_ = std::make_unique<encrypt::AesEncrypt>(shared_key);
  detail::ItemHeader item_header;
  log_file_->read(reinterpret_cast<char*>(&item_header), sizeof(item_header));
  if (item_header.magic == detail::ItemHeader::kMagic) {
    LOG_INFO("Decoding item of size: {}", item_header.size);
  } else {
    std::cerr << "Item magic number mismatch\n";
    return;
  }
  log_file_->read(decrypt_buf_.data(), item_header.size);
    std::string decrypted_data = decrypt_->Decrypt(decrypt_buf_.data(), item_header.size);
    LOG_INFO("Decrypted Data: {}", decrypted_data.size());
    std::string decompressed_data = decompress_->DeCompress(decrypted_data.data(), decrypted_data.size());
    std::cout << "Decompressed Data: " << decompressed_data.size() << std::endl;
    std::cout << "Decompressed Data: " << decompressed_data << std::endl;
}


}
}