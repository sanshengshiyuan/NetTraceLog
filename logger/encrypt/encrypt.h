#pragma once

#include <string>
#include <tuple>

namespace logger {
namespace encrypt {
std::string BinaryKeyToHex(const std::string& binary_key);
std::string HexKeyToBinary(const std::string& hex_key);

std::tuple<std::string, std::string> GenECDHKey(); 
std::string GenECDHSharedSecret(const std::string& client_pri, const std::string& server_pub);

class Encryption {
public:
    Encryption() = default;
    virtual ~Encryption() = default;

    virtual void Encrypt(const void* intput, size_t input_size, std::string& output) = 0;

    virtual std::string Decrypt(const void* data, size_t size) = 0;

    virtual void SetKey(const std::string& key) = 0;
};
}
}