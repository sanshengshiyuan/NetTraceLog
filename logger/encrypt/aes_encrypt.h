#pragma once

#include "encrypt.h"

#include "openssl/aes.h"
#include <openssl/rand.h>

namespace logger {
namespace encrypt {
class AesEncrypt final : public Encryption {
public:
    AesEncrypt(std::string key);
    virtual ~AesEncrypt() = default;

    static std::string GenerateKey();
    static std::string GenerateIV();

    virtual void Encrypt(const void* intput, size_t input_size, std::string& output) override;

    virtual std::string Decrypt(const void* data, size_t size) override;
private:
    std::string key_;
    std::string iv_;
};
}
}


