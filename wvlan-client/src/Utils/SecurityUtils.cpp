/**
 * @file SecurityUtils.cpp
 * @brief 安全工具类实现
 */

#include "SecurityUtils.h"
#include <windows.h>
#include <wincrypt.h>
#include <algorithm>

#pragma comment(lib, "advapi32.lib")

namespace WVLAN {
namespace Core {

std::vector<uint8_t> SecurityUtils::GenerateRandomBytes(size_t length) {
    std::vector<uint8_t> data(length);
    
    HCRYPTPROV hProv = 0;
    if (CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        CryptGenRandom(hProv, static_cast<DWORD>(length), data.data());
        CryptReleaseContext(hProv, 0);
    }
    
    return data;
}

bool SecurityUtils::GenerateWireGuardKeyPair(std::string& privateKey, std::string& publicKey) {
    // WireGuard 私钥是 32 字节，公钥也是 32 字节
    auto privateKeyBytes = GenerateRandomBytes(32);
    
    // 确保私钥有效（设置适当的位）
    privateKeyBytes[0] &= 248;
    privateKeyBytes[31] &= 127;
    privateKeyBytes[31] |= 64;
    
    // 简化实现：这里应该使用 curve25519 算法计算公钥
    // 实际实现需要链接 libsodium 或使用 WireGuard 库
    
    privateKey = Base64Encode(privateKeyBytes);
    publicKey = Base64Encode(GenerateRandomBytes(32)); // 占位符
    
    return true;
}

bool SecurityUtils::GetPublicKeyFromPrivateKey(const std::string& privateKey, std::string& publicKey) {
    auto privBytes = Base64Decode(privateKey);
    
    if (privBytes.size() != 32) {
        return false;
    }
    
    // 简化实现：这里应该使用 curve25519 算法
    // 实际实现需要调用 WireGuard 或 libsodium API
    
    auto pubBytes = GenerateRandomBytes(32); // 占位符
    publicKey = Base64Encode(pubBytes);
    
    return true;
}

std::string SecurityUtils::Base64Encode(const std::vector<uint8_t>& data) {
    return Base64Encode(data.data(), data.size());
}

std::string SecurityUtils::Base64Encode(const uint8_t* data, size_t length) {
    if (length == 0) return "";
    
    static const char base64Chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    std::string result;
    result.reserve(((length + 2) / 3) * 4);
    
    size_t i = 0;
    while (i < length) {
        uint32_t octetA = i < length ? data[i++] : 0;
        uint32_t octetB = i < length ? data[i++] : 0;
        uint32_t octetC = i < length ? data[i++] : 0;
        
        uint32_t triple = (octetA << 16) + (octetB << 8) + octetC;
        
        result.push_back(base64Chars[(triple >> 18) & 0x3F]);
        result.push_back(base64Chars[(triple >> 12) & 0x3F]);
        result.push_back(base64Chars[(triple >> 6) & 0x3F]);
        result.push_back(base64Chars[triple & 0x3F]);
    }
    
    // 添加填充
    int mod = length % 3;
    if (mod == 1) {
        result[result.length() - 2] = '=';
        result[result.length() - 1] = '=';
    } else if (mod == 2) {
        result[result.length() - 1] = '=';
    }
    
    return result;
}

std::vector<uint8_t> SecurityUtils::Base64Decode(const std::string& encoded) {
    if (encoded.empty()) return {};
    
    static const int base64Table[] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
    };
    
    std::vector<uint8_t> result;
    result.reserve((encoded.length() / 4) * 3);
    
    uint32_t buffer = 0;
    int bitsCollected = 0;
    
    for (char c : encoded) {
        if (c == '=') break;
        
        int value = base64Table[(unsigned char)c];
        if (value < 0) continue;
        
        buffer = (buffer << 6) | value;
        bitsCollected += 6;
        
        if (bitsCollected >= 8) {
            bitsCollected -= 8;
            result.push_back((buffer >> bitsCollected) & 0xFF);
        }
    }
    
    return result;
}

bool SecurityUtils::IsValidBase64(const std::string& str) {
    if (str.empty()) return false;
    
    for (char c : str) {
        if (c == '=') continue;
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '+' || c == '/')) {
            return false;
        }
    }
    
    // 检查填充
    size_t len = str.length();
    if (len % 4 != 0) return false;
    
    return true;
}

bool SecurityUtils::ConstantTimeCompare(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    if (a.size() != b.size()) {
        return false;
    }
    
    volatile uint8_t result = 0;
    for (size_t i = 0; i < a.size(); i++) {
        result |= a[i] ^ b[i];
    }
    
    return result == 0;
}

void SecurityUtils::SecureZeroMemory(void* ptr, size_t size) {
    RtlSecureZeroMemory(ptr, size);
}

} // namespace Core
} // namespace WVLAN
