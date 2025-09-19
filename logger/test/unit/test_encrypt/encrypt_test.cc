// encrypt 模块的基础单元测试（不依赖 gtest）
#include <iostream>
#include <string>
#include <memory>

#include "encrypt/encrypt.h"
#include "encrypt/aes_encrypt.h"

int main() {
    auto [a_pub, a_pri] = logger::encrypt::GenECDHKey();
    auto b_pub = logger::encrypt::HexKeyToBinary("04827405069030E26A211C973C8710E6FBE79B5CAA364AC111FB171311902277537F8852EADD17EB339EB7CD0BA2490A58CDED2C702DFC1EFC7EDB544B869F039C");
    auto b_pri = logger::encrypt::HexKeyToBinary("FAA5BBE9017C96BF641D19D0144661885E831B5DDF52539EF1AB4790C05E665E");
    std::cout << "A Public Key (Hex): " << a_pub.size() << "\n";
    std::cout << "A Private Key (Hex): " << a_pri.size() << "\n";
    std::cout << "B Public Key (Hex): " << b_pub.size() << "\n";
    std::cout << "B Private Key (Hex): " << b_pri.size() << "\n";
    // 1) ECDH 密钥对生成应非空
    if (a_pub.empty() || a_pri.empty() || b_pub.empty() || b_pri.empty()) {
        std::cerr << "[FAIL] ECDH key generation failed.\n";
        return 1;
    }

    // 2) 双方计算的共享密钥应一致且非空
    std::string shared_ab = logger::encrypt::GenECDHSharedSecret(a_pri, b_pub);
    std::cout << "A-B Shared Secret (Hex): " << shared_ab.size() << "\n";
    std::string shared_ba = logger::encrypt::GenECDHSharedSecret(b_pri, a_pub);
    std::cout << "B-A Shared Secret (Hex): " << shared_ba.size() << "\n";
    if (shared_ab.empty() || shared_ba.empty() || shared_ab != shared_ba) {
        std::cerr << "[FAIL] ECDH shared secret generation failed.\n";
        return 1;
    }
    auto encrypt = std::make_unique<logger::encrypt::AesEncrypt>(shared_ab);
        // 生成并设置 16 字节 IV
    // std::string iv = logger::encrypt::AesEncrypt::GenerateIV();
    // encrypt->setIV(iv);
    std::string iv1 = encrypt->GetIV();
    std::string plaintext = "Hello, World! This is a test message.";
    std::string ciphertext;
    ciphertext.reserve(plaintext.size() + AES_BLOCK_SIZE);  // 预留足够空间
    encrypt->Encrypt(plaintext.data(), plaintext.size(), ciphertext);
    std::cout << "Ciphertext Size: " << ciphertext.size() << "\n";
    std::cout << "Ciphertext (Hex): " << logger::encrypt::BinaryKeyToHex(ciphertext) << "\n";
        // 解密前设置同样的 IV
    // encrypt->setIV(iv);
    std::string iv2 = encrypt->GetIV();
    if (iv1 != iv2) {
        std::cerr << "[FAIL] IV mismatch.\n";
    }
    std::string decrypted_text = encrypt->Decrypt(ciphertext.data(), ciphertext.size());
        //去除 PKCS#7 填充
        if (!decrypted_text.empty()) {
            unsigned char pad = decrypted_text.back();
            if (pad > 0 && pad <= AES_BLOCK_SIZE && decrypted_text.size() >= pad) {
                decrypted_text.resize(decrypted_text.size() - pad);
            }
        }
    std::cout << "Decrypted Text: " << decrypted_text << "\n";
   
    return 0;
}       










