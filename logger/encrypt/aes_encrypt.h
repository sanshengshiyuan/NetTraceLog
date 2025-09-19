#pragma once

#include "encrypt.h"

#include "openssl/aes.h"
#include <openssl/rand.h>

namespace logger {
namespace encrypt {
class AesEncrypt final : public Encryption {
public:
    AesEncrypt() = default;
    
    AesEncrypt(std::string key);
    virtual ~AesEncrypt() = default;

    static std::string GenerateKey();
    static std::string GenerateIV();

    virtual void Encrypt(const void* intput, size_t input_size, std::string& output) override;

    virtual std::string Decrypt(const void* data, size_t size) override;
    // 新增：设置 IV
    void SetIV(const std::string& iv);
    //新增：获得IV
    std::string GetIV() const;
    //设置密钥
    void SetKey(const std::string& key) override;   
private:
    std::string key_;
    std::string iv_;
};
}
}


