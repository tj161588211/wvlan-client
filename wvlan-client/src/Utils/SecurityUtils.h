/**
 * @file SecurityUtils.h
 * @brief 安全工具类 - 密钥生成、加密等操作
 * 
 * @version 1.0.0
 * @date 2026-04-15
 */

#ifndef WVLAN_SECURITY_UTILS_H
#define WVLAN_SECURITY_UTILS_H

#include <string>
#include <vector>
#include <cstdint>

namespace WVLAN {
namespace Core {

class SecurityUtils {
public:
    /// <summary>
    /// 生成随机字节序列
    /// </summary>
    static std::vector<uint8_t> GenerateRandomBytes(size_t length);

    /// <summary>
    /// 生成 WireGuard 密钥对
    /// </summary>
    static bool GenerateWireGuardKeyPair(std::string& privateKey, std::string& publicKey);

    /// <summary>
    /// 从私钥计算公钥
    /// </summary>
    static bool GetPublicKeyFromPrivateKey(const std::string& privateKey, std::string& publicKey);

    /// <summary>
    /// Base64 编码
    /// </summary>
    static std::string Base64Encode(const std::vector<uint8_t>& data);
    static std::string Base64Encode(const uint8_t* data, size_t length);

    /// <summary>
    /// Base64 解码
    /// </summary>
    static std::vector<uint8_t> Base64Decode(const std::string& encoded);

    /// <summary>
    /// 验证 Base64 字符串
    /// </summary>
    static bool IsValidBase64(const std::string& str);

    /// <summary>
    /// 安全比较两个字节数组（防止时序攻击）
    /// </summary>
    static bool ConstantTimeCompare(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b);

    /// <summary>
    /// 安全清除内存
    /// </summary>
    static void SecureZeroMemory(void* ptr, size_t size);

private:
    SecurityUtils() = delete;
};

} // namespace Core
} // namespace WVLAN

#endif // WVLAN_SECURITY_UTILS_H
