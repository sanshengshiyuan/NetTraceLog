#include "encrypt.h"

#include "openssl/ec.h"
#include "openssl/ecdh.h"
#include "openssl/bn.h"
#include "openssl/obj_mac.h"
#include "fmt/core.h"

namespace logger {
namespace encrypt {
std::string BinaryKeyToHex(const std::string& binary_key) {
    std::string hex_key;
    hex_key.reserve(binary_key.size() * 2);
    for (unsigned char c : hex_key) {
        hex_key += fmt::format("{:02x}", c);
    }
    return hex_key;
}
std::string HexKeyToBinary(const std::string& hex_key) {
    std::string binary_key;
    if (hex_key.size() % 2 != 0) {
        return binary_key; // 十六进制长度必须为偶数
    }
    
    binary_key.reserve(hex_key.size() / 2);
    for (size_t i = 0; i < hex_key.size(); i += 2) {
        // 截取2个字符作为一个十六进制字节
        std::string hex_byte = hex_key.substr(i, 2);
        // 转换为整数（base=16）
        unsigned int val;
        try {
            val = std::stoul(hex_byte, nullptr, 16);
        } catch (...) {
            return ""; // 无效的十六进制字符
        }
        binary_key.push_back(static_cast<unsigned char>(val));
    }
    
    return binary_key;
}
std::tuple<std::string, std::string> GenECDHKey() {
    EC_GROUP *curve = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    if (curve == nullptr) {
        return {};
    }
    EC_KEY *key = EC_KEY_new();
    if (key == nullptr || !EC_KEY_set_group(key, curve) || !EC_KEY_generate_key(key)) {
        EC_GROUP_free(curve);
        EC_KEY_free(key);
        return {};
    }
    //提取私钥
    const BIGNUM *priv_binary = EC_KEY_get0_private_key(key);
    if (priv_binary == nullptr) {
        EC_GROUP_free(curve);
        EC_KEY_free(key);
        return {};
    }
    //提取公钥
    const EC_POINT *pub_point = EC_KEY_get0_public_key(key);
    if (pub_point == nullptr) {
        EC_GROUP_free(curve);
        EC_KEY_free(key);
        return {};
    }
    //将公钥和私钥转换为二进制字符串
    int priv_len = BN_num_bytes(priv_binary);
    // 分配缓冲区
    unsigned char* priv_buf = (unsigned char*)OPENSSL_malloc(priv_len);
    if (!priv_buf) {
        EC_GROUP_free(curve);
        EC_KEY_free(key);
        return {};
    }
    // 将BIGNUM转换为二进制（大端序）
    BN_bn2bin(priv_binary, priv_buf);
    // 封装为std::string（二进制字符串）
    std::string priv_str(reinterpret_cast<char*>(priv_buf), priv_len);
    // 释放缓冲区
    OPENSSL_free(priv_buf);

    // 2. 转换公钥（EC_POINT -> 二进制字符串）
    size_t pub_len = EC_POINT_point2oct(
        curve,               // 椭圆曲线组
        pub_point,           // 公钥点
        POINT_CONVERSION_UNCOMPRESSED,  // 公钥格式
        nullptr, 0,          // 先传nullptr获取所需长度
        nullptr
    );
    // 分配缓冲区
    unsigned char* pub_buf = (unsigned char*)OPENSSL_malloc(pub_len);
    if (!pub_buf) {
        EC_GROUP_free(curve);
        EC_KEY_free(key);
        return {};
    }
    // 将EC_POINT转换为二进制
    if (!EC_POINT_point2oct(
        curve, 
        pub_point, 
        POINT_CONVERSION_UNCOMPRESSED, 
        pub_buf, pub_len, 
        nullptr
    )) {
        OPENSSL_free(pub_buf);
        EC_GROUP_free(curve);
        EC_KEY_free(key);
        return {};
    }
    // 封装为std::string（二进制字符串）
    std::string pub_str(reinterpret_cast<char*>(pub_buf), pub_len);
    // 释放缓冲区
    OPENSSL_free(pub_buf);

    // 3. 释放资源并返回
    EC_GROUP_free(curve);
    EC_KEY_free(key);

    return std::make_tuple(pub_str, priv_str);
}
std::string GenECDHSharedSecret(const std::string& client_pri, const std::string& server_pub) {
    // 1. 初始化曲线（固定使用secp256r1）
    EC_GROUP* curve = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    if (!curve) return "";

    // 2. 客户端私钥：std::string -> BIGNUM -> EC_KEY
    EC_KEY* client_key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    BIGNUM* priv_bn = BN_bin2bn(reinterpret_cast<const unsigned char*>(client_pri.data()), client_pri.size(), nullptr);
    if (!client_key || !priv_bn || !EC_KEY_set_private_key(client_key, priv_bn)) {
        EC_KEY_free(client_key);
        BN_free(priv_bn);
        EC_GROUP_free(curve);
        return "";
    }

    // 3. 服务器公钥：std::string -> EC_POINT
    EC_POINT* server_point = EC_POINT_new(curve);
    if (!server_point || EC_POINT_oct2point(curve, server_point, 
        reinterpret_cast<const unsigned char*>(server_pub.data()), server_pub.size(), nullptr) != 1) {
        EC_POINT_free(server_point);
        EC_KEY_free(client_key);
        BN_free(priv_bn);
        EC_GROUP_free(curve);
        return "";
    }

    // 4. 核心：调用ECDH_compute_key计算共享密钥
    size_t secret_len = ECDH_compute_key(nullptr, 0, server_point, client_key, nullptr);
    unsigned char* secret = (unsigned char*)OPENSSL_malloc(secret_len);
    bool success = (secret && ECDH_compute_key(secret, secret_len, server_point, client_key, nullptr) == secret_len);

    // 5. 封装结果并释放所有资源
    std::string result = success ? std::string(reinterpret_cast<char*>(secret), secret_len) : "";
    OPENSSL_free(secret);
    EC_POINT_free(server_point);
    EC_KEY_free(client_key);
    BN_free(priv_bn);
    EC_GROUP_free(curve);
    return result;
}
}
}