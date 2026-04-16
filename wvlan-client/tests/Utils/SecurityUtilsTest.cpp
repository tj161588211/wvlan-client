/**
 * @file SecurityUtilsTest.cpp
 * @brief 安全工具单元测试
 */

#include <gtest/gtest.h>
#include "../../src/Utils/SecurityUtils.h"

using namespace WVLAN::Utils;

// ============================================================================
// SecurityUtils 测试
// ============================================================================

class SecurityUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

// 测试生成随机字节
TEST_F(SecurityUtilsTest, GenerateRandomBytes) {
    std::vector<uint8_t> randomBytes = SecurityUtils::GenerateRandomBytes(32);
    
    EXPECT_EQ(randomBytes.size(), 32);
    
    // 检查是否真的随机（几乎不可能全部相同）
    bool allSame = true;
    uint8_t firstByte = randomBytes[0];
    for (size_t i = 1; i < randomBytes.size(); ++i) {
        if (randomBytes[i] != firstByte) {
            allSame = false;
            break;
        }
    }
    EXPECT_FALSE(allSame);
}

// 测试不同长度的随机字节生成
TEST_F(SecurityUtilsTest, GenerateRandomBytesDifferentSizes) {
    size_t sizes[] = {16, 32, 64, 128};
    
    for (size_t size : sizes) {
        std::vector<uint8_t> bytes = SecurityUtils::GenerateRandomBytes(size);
        EXPECT_EQ(bytes.size(), size);
    }
}

// 测试 Base64 编码和解码
TEST_F(SecurityUtilsTest, Base64EncodeDecode) {
    std::string original = "Hello, World!";
    std::string encoded = SecurityUtils::Base64Encode(original);
    std::string decoded = SecurityUtils::Base64Decode(encoded);
    
    EXPECT_EQ(decoded, original);
    EXPECT_FALSE(encoded.empty());
}

// 测试 Base64 编码二进制数据
TEST_F(SecurityUtilsTest, Base64EncodeDecodeBinary) {
    std::vector<uint8_t> binaryData = {0x00, 0x01, 0x02, 0x03, 0xFF, 0xFE, 0xFD, 0xFC};
    std::string encoded = SecurityUtils::Base64Encode(binaryData);
    std::vector<uint8_t> decoded = SecurityUtils::Base64DecodeToBinary(encoded);
    
    EXPECT_EQ(decoded.size(), binaryData.size());
    EXPECT_EQ(decoded, binaryData);
}

// 测试 Base64 URL 安全编码
TEST_F(SecurityUtilsTest, Base64URLEncodeDecode) {
    std::string original = "Hello+World/Test=String";
    std::string encoded = SecurityUtils::Base64URLEncode(original);
    std::string decoded = SecurityUtils::Base64URLDecode(encoded);
    
    EXPECT_EQ(decoded, original);
    
    // URL 安全编码不应该包含 + 或 /
    EXPECT_TRUE(encoded.find('+') == std::string::npos || 
                encoded.find('/') == std::string::npos);
}

// 测试 WireGuard 密钥生成
TEST_F(SecurityUtilsTest, GenerateWireGuardKeyPair) {
    KeyPair keyPair = SecurityUtils::GenerateWireGuardKeyPair();
    
    // 验证密钥长度（WireGuard 使用 32 字节密钥，Base64 编码后为 44 字符）
    EXPECT_EQ(keyPair.PrivateKey.length(), 44);
    EXPECT_EQ(keyPair.PublicKey.length(), 44);
    
    // 私钥和公钥应该不同
    EXPECT_NE(keyPair.PrivateKey, keyPair.PublicKey);
    
    // 验证 Base64 格式（只包含 A-Z, a-z, 0-9, +, /, =）
    std::regex base64Pattern("^[A-Za-z0-9+/]+=*$");
    EXPECT_TRUE(std::regex_match(keyPair.PrivateKey, base64Pattern));
    EXPECT_TRUE(std::regex_match(keyPair.PublicKey, base64Pattern));
}

// 测试生成多个密钥对确保唯一性
TEST_F(SecurityUtilsTest, GenerateUniqueKeyPairs) {
    std::set<std::pair<std::string, std::string>> keyPairsSet;
    
    for (int i = 0; i < 100; ++i) {
        KeyPair keyPair = SecurityUtils::GenerateWireGuardKeyPair();
        
        // 所有生成的密钥对应唯一
        EXPECT_EQ(keyPairsSet.count({keyPair.PrivateKey, keyPair.PublicKey}), 0);
        keyPairsSet.insert({keyPair.PrivateKey, keyPair.PublicKey});
    }
    
    EXPECT_EQ(keyPairsSet.size(), 100);
}

// 测试密钥从字节生成
TEST_F(SecurityUtilsTest, KeyFromBytes) {
    std::vector<uint8_t> privateKeyBytes = SecurityUtils::GenerateRandomBytes(32);
    std::string privateKeyB64 = SecurityUtils::KeyFromBytes(privateKeyBytes);
    
    EXPECT_EQ(privateKeyB64.length(), 44);  // Base64 编码的 32 字节
    
    // 解码回二进制应该得到原始数据
    std::vector<uint8_t> decoded = SecurityUtils::Base64DecodeToBinary(privateKeyB64);
    EXPECT_EQ(decoded, privateKeyBytes);
}

// 测试密钥比较
TEST_F(SecurityUtilsTest, CompareKeys) {
    std::string key1 = "dQw4w9WgXcQ+AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==";
    std::string key2 = "dQw4w9WgXcQ+AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==";
    std::string key3 = "differentKeyHereXXXXXXXXXXXXXXXXXXXXXXXXXXXX=";
    
    EXPECT_TRUE(SecurityUtils::CompareKeys(key1, key2));
    EXPECT_FALSE(SecurityUtils::CompareKeys(key1, key3));
    EXPECT_FALSE(SecurityUtils::CompareKeys("", key1));
}

// 测试 HMAC-SHA256（如果实现）
#ifdef SECURITYUTILS_HAS_HMAC
TEST_F(SecurityUtilsTest, HMAC_SHA256) {
    std::string message = "Hello, World!";
    std::string key = "secret_key";
    
    std::string hmac = SecurityUtils::HMAC_SHA256(message, key);
    
    EXPECT_EQ(hmac.length(), 64);  // SHA256 输出是 32 字节，十六进制表示为 64 字符
    EXPECT_TRUE(std::all_of(hmac.begin(), hmac.end(), [](char c) {
        return std::isxdigit(c);
    }));
}
#endif
