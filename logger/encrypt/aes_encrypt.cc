#include "aes_encrypt.h"

#include <stdexcept>
#include <cstring>

namespace logger {
namespace encrypt {

namespace detail {
    void Encrypt(const void* input, size_t input_size, std::string& output,
        const std::string& key, const std::string& iv) {
    // 检查IV长度是否符合AES块大小
    if (iv.size() != AES_BLOCK_SIZE) {
        throw std::runtime_error("IV length must be " + std::to_string(AES_BLOCK_SIZE) + " bytes");
    }

    AES_KEY aes_encrypt_key;
    if (AES_set_encrypt_key(reinterpret_cast<const unsigned char*>(key.data()),
                        static_cast<int>(key.size() * 8), &aes_encrypt_key) != 0) {
        throw std::runtime_error("Failed to initialize AES encryption key");
    }

    // 计算PKCS#7填充长度
    size_t padding_size = AES_BLOCK_SIZE - (input_size % AES_BLOCK_SIZE);
    const size_t padded = input_size + padding_size;

    // 准备输入缓冲区并应用PKCS#7填充
    std::string in;
    in.resize(padded);
    std::memcpy(&in[0], input, input_size);
    
    // 填充N个值为N的字节
    for (size_t i = input_size; i < padded; ++i) {
        in[i] = static_cast<char>(padding_size);
    }

    // 准备输出缓冲区
    output.resize(padded);

    // 采用拷贝的IV，避免原始IV被修改
    std::string iv_copy = iv;
    AES_cbc_encrypt(reinterpret_cast<const unsigned char*>(in.data()),
                reinterpret_cast<unsigned char*>(&output[0]),
                static_cast<unsigned int>(padded),
                &aes_encrypt_key,
                reinterpret_cast<unsigned char*>(&iv_copy[0]),
                AES_ENCRYPT);
}

std::string Decrypt(const void* data, size_t size, const std::string& key, const std::string& iv) {
    // 检查输入大小是否为块大小的整数倍
    if (size % AES_BLOCK_SIZE != 0) {
        throw std::runtime_error("Encrypted data size must be multiple of AES block size");
    }
    
    // 检查IV长度是否符合要求
    if (iv.size() != AES_BLOCK_SIZE) {
        throw std::runtime_error("IV length must be " + std::to_string(AES_BLOCK_SIZE) + " bytes");
    }

    std::string decrypted_text;
    AES_KEY aes_decrypt_key;
    decrypted_text.resize(size);
    
    if(AES_set_decrypt_key(reinterpret_cast<const unsigned char *>(key.data()), 
                          static_cast<int>(key.size() * 8),  // 传入位数
                          &aes_decrypt_key) != 0) {
        throw std::runtime_error("Failed to initialize AES decryption key");
    }

    // 采用拷贝的IV，避免原始IV被修改
    std::string iv_copy = iv;
    AES_cbc_encrypt(reinterpret_cast<const unsigned char*>(data),
                   reinterpret_cast<unsigned char*>(decrypted_text.data()),
                   size,
                   &aes_decrypt_key,
                   reinterpret_cast<unsigned char*>(&iv_copy[0]),
                   AES_DECRYPT);

    // 移除PKCS#7填充
    if (decrypted_text.empty()) {
        return "";
    }

    // 最后一个字节就是填充长度
    unsigned char padding_size = static_cast<unsigned char>(decrypted_text.back());
    
    // 验证填充是否有效
    if (padding_size == 0 || padding_size > AES_BLOCK_SIZE) {
        throw std::runtime_error("Invalid PKCS#7 padding");
    }

    // 检查所有填充字节是否正确
    for (size_t i = size - padding_size; i < size; ++i) {
        if (static_cast<unsigned char>(decrypted_text[i]) != padding_size) {
            throw std::runtime_error("Invalid PKCS#7 padding bytes");
        }
    }

    // 移除填充
    decrypted_text.resize(size - padding_size);
    return decrypted_text;
}          
}

AesEncrypt::AesEncrypt(std::string key) {
    key_ = std::move(key);
    iv_ = "0000000000000000";  // 16-byte IV for AES
}

std::string AesEncrypt::GenerateKey() {
    std::string key;
    key.resize(32); // AES-256
    if (RAND_bytes(reinterpret_cast<unsigned char*>(&key[0]),
                   static_cast<int>(key.size())) != 1) {
        throw std::runtime_error("Failed to generate random key");
    }
    return key;
}

std::string AesEncrypt::GenerateIV() {
    std::string iv;
    iv.resize(AES_BLOCK_SIZE);
    if (RAND_bytes(reinterpret_cast<unsigned char*>(&iv[0]),
                   AES_BLOCK_SIZE) != 1) {
        throw std::runtime_error("Failed to generate random IV");
    }
    return iv;
}

void AesEncrypt::Encrypt(const void* input, size_t input_size, std::string& output) {
    detail::Encrypt(input, input_size, output, key_, iv_);
}

std::string AesEncrypt::Decrypt(const void* data, size_t size) {
    return detail::Decrypt(data, size, key_, iv_);
}
// 新增：设置 IV
    void AesEncrypt::SetIV(const std::string& iv) { 
        iv_ = iv; 
    }
    //新增：获得IV
    std::string AesEncrypt::GetIV() const { 
        return iv_; 
    }
    //设置密钥
    void AesEncrypt::SetKey(const std::string& key) { 
        key_ = key; 
    }

}
}