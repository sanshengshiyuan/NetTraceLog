#include "aes_encrypt.h"

#include <stdexcept>

namespace logger {
namespace encrypt {

namespace detail {
void Encrypt(const void* intput, size_t input_size, std::string& output, const std::string& key, std::string& iv ) {
    AES_KEY aes_encrypt_key;
    if(AES_set_encrypt_key(reinterpret_cast<const unsigned char *>(key.data()), key.size(), &aes_encrypt_key) != 0) {
        throw std::runtime_error("Failed to initialize AES encryption key");
    }
    AES_cbc_encrypt(reinterpret_cast<const unsigned char*>(intput),reinterpret_cast<unsigned char*>(output.data()),input_size,&aes_encrypt_key,reinterpret_cast<unsigned char*>(iv.data()),AES_ENCRYPT);
}     
static std::string Decrypt(const void* data, size_t size, const std::string& key, std::string& iv) {
    std::string decrypted_text;
    AES_KEY aes_decrypt_key;
    if(AES_set_decrypt_key(reinterpret_cast<const unsigned char *>(key.data()), key.size(), &aes_decrypt_key) != 0) {
        throw std::runtime_error("Failed to initialize AES decryption key");
    }
    AES_cbc_encrypt(reinterpret_cast<const unsigned char*>(data),reinterpret_cast<unsigned char*>(decrypted_text.data()),size,&aes_decrypt_key,reinterpret_cast<unsigned char*>(iv.data()),AES_DECRYPT);
}            
}
AesEncrypt::AesEncrypt(std::string key) {
    key_ = std::move(key);
    iv_ = {0};
}
static std::string GenerateKey() {
    unsigned char* iv;
    if (RAND_bytes(iv, AES_BLOCK_SIZE) != 1) {
        throw std::runtime_error("Failed to generate random IV");
    }
    return std::string(reinterpret_cast<const char*>(iv), sizeof(iv));
}
static std::string GenerateIV() {
    unsigned char* key;
    if (RAND_bytes(key, AES_BLOCK_SIZE) != 1) {
        throw std::runtime_error("Failed to generate random IV");
    }
    return std::string(reinterpret_cast<const char*>(key), sizeof(key));
}
void AesEncrypt::Encrypt(const void* intput, size_t input_size, std::string& output) {
    detail::Encrypt(intput, input_size, output, key_, iv_);
}

std::string AesEncrypt::Decrypt(const void* data, size_t size) {
    return detail::Decrypt(data, size, key_, iv_);
}

}
}

