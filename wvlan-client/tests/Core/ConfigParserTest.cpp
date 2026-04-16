/**
 * @file ConfigParserTest.cpp
 * @brief 配置解析器单元测试
 */

#include <gtest/gtest.h>
#include "../../src/Core/Config/ConfigParser.h"

using namespace WVLAN::Core;

// ============================================================================
// ConfigParser 测试
// ============================================================================

class ConfigParserTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

// 测试有效的配置文件字符串
TEST_F(ConfigParserTest, ParseValidConfigString) {
    std::string configText = R"(
[Interface]
PrivateKey = sAxY0XsPftePA5pL5fqPqjUt4GCDq3l7K8gJvzCZdFM=
Address = 10.10.0.2/24
DNS = 8.8.8.8,8.8.4.4
ListenPort = 51820

[Peer]
PublicKey = yLchSJhS+bIqhLpD9fFh3y6QrWbEz8VnMxKjNtRzPxs=
AllowedIPs = 10.10.0.1/32
Endpoint = 192.168.1.100:51820
PersistentKeepalive = 25
)";

    ParseResult result = ConfigParser::ParseFromString(configText);
    
    EXPECT_TRUE(result.Success);
    EXPECT_FALSE(result.Config.Interface.PrivateKey.empty());
    EXPECT_EQ(result.Config.Interface.Address, "10.10.0.2/24");
    EXPECT_EQ(result.Config.Interface.DNS, "8.8.8.8,8.8.4.4");
    EXPECT_EQ(result.Config.Interface.ListenPort, 51820);
    EXPECT_EQ(result.Config.Peers.size(), 1);
    EXPECT_EQ(result.Config.Peers[0].PublicKey, "yLchSJhS+bIqhLpD9fFh3y6QrWbEz8VnMxKjNtRzPxs=");
}

// 测试多个对等节点
TEST_F(ConfigParserTest, ParseMultiplePeers) {
    std::string configText = R"(
[Interface]
PrivateKey = sAxY0XsPftePA5pL5fqPqjUt4GCDq3l7K8gJvzCZdFM=
Address = 10.10.0.2/24

[Peer]
PublicKey = peer1PublicKeyHereAAAAAAAAAAAAAAAAAAAA=
AllowedIPs = 10.10.0.1/32
Endpoint = 192.168.1.100:51820

[Peer]
PublicKey = peer2PublicKeyHereAAAAAAAAAAAAAAAAAAAA=
AllowedIPs = 10.10.0.3/32
Endpoint = 192.168.1.101:51820

[Peer]
PublicKey = peer3PublicKeyHereAAAAAAAAAAAAAAAAAAAA=
AllowedIPs = 10.10.0.4/32
Endpoint = 192.168.1.102:51820
)";

    ParseResult result = ConfigParser::ParseFromString(configText);
    
    EXPECT_TRUE(result.Success);
    EXPECT_EQ(result.Config.Peers.size(), 3);
}

// 测试缺少私钥的配置
TEST_F(ConfigParserTest, ParseMissingPrivateKey) {
    std::string configText = R"(
[Interface]
Address = 10.10.0.2/24

[Peer]
PublicKey = testPublicKeyHereAAAAAAAAAAAAAAAAAAAAAAA=
AllowedIPs = 10.10.0.1/32
)";

    ParseResult result = ConfigParser::ParseFromString(configText);
    
    // 验证应该失败，因为缺少私钥
    EXPECT_FALSE(result.Success);
}

// 测试缺少对等节点的配置
TEST_F(ConfigParserTest, ParseMissingPeers) {
    std::string configText = R"(
[Interface]
PrivateKey = sAxY0XsPftePA5pL5fqPqjUt4GCDq3l7K8gJvzCZdFM=
Address = 10.10.0.2/24
)";

    ParseResult result = ConfigParser::ParseFromString(configText);
    
    // 验证应该失败，因为缺少对等节点
    EXPECT_FALSE(result.Success);
}

// 测试空配置
TEST_F(ConfigParserTest, ParseEmptyConfig) {
    std::string configText = "";
    
    ParseResult result = ConfigParser::ParseFromString(configText);
    
    EXPECT_FALSE(result.Success);
}

// 测试导出配置
TEST_F(ConfigParserTest, ExportToString) {
    NetworkConfig config;
    config.Interface.PrivateKey = "sAxY0XsPftePA5pL5fqPqjUt4GCDq3l7K8gJvzCZdFM=";
    config.Interface.Address = "10.10.0.2/24";
    config.Interface.ListenPort = 51820;
    
    PeerConfig peer;
    peer.PublicKey = "yLchSJhS+bIqhLpD9fFh3y6QrWbEz8VnMxKjNtRzPxs=";
    peer.AllowedIPs = "10.10.0.1/32";
    peer.Endpoint = "192.168.1.100:51820";
    peer.PersistentKeepalive = 25;
    config.Peers.push_back(peer);
    
    std::string exported = ConfigParser::ExportToString(config);
    
    EXPECT_FALSE(exported.empty());
    EXPECT_TRUE(exported.find("[Interface]") != std::string::npos);
    EXPECT_TRUE(exported.find("[Peer]") != std::string::npos);
    EXPECT_TRUE(exported.find("PrivateKey") != std::string::npos);
}

// 测试密钥验证
TEST_F(ConfigParserTest, IsValidKey) {
    // 有效的 44 字符 Base64 密钥
    std::string validKey = "dQw4w9WgXcQ+AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==";
    EXPECT_TRUE(ConfigParser::IsValidKey(validKey));
    
    // 太短的密钥
    std::string shortKey = "shortkey";
    EXPECT_FALSE(ConfigParser::IsValidKey(shortKey));
    
    // 无效的 Base64 字符
    std::string invalidKey = "invalid@key!here#$$$%^^&&**((()))__++||{{}}";
    EXPECT_FALSE(ConfigParser::IsValidKey(invalidKey));
}

// 测试 IP 地址验证
TEST_F(ConfigParserTest, IsValidIPAddress) {
    EXPECT_TRUE(ConfigParser::IsValidIPAddress("192.168.1.1"));
    EXPECT_TRUE(ConfigParser::IsValidIPAddress("10.0.0.1/24"));
    EXPECT_FALSE(ConfigParser::IsValidIPAddress("invalid.ip.address"));
    EXPECT_FALSE(ConfigParser::IsValidIPAddress("999.999.999.999"));
}

// 测试端点验证
TEST_F(ConfigParserTest, IsValidEndpoint) {
    EXPECT_TRUE(ConfigParser::IsValidEndpoint("192.168.1.1:51820"));
    EXPECT_TRUE(ConfigParser::IsValidEndpoint("10.0.0.1:12345"));
    EXPECT_FALSE(ConfigParser::IsValidEndpoint("invalid-endpoint"));
    EXPECT_FALSE(ConfigParser::IsValidEndpoint("192.168.1.1"));  // 缺少端口
    EXPECT_FALSE(ConfigParser::IsValidEndpoint("192.168.1.1:99999"));  // 端口超出范围
}

// 测试带有注释和空行的配置
TEST_F(ConfigParserTest, ParseWithCommentsAndBlankLines) {
    std::string configText = R"(
# 这是一个注释
[Interface]
# 私钥设置
PrivateKey = sAxY0XsPftePA5pL5fqPqjUt4GCDq3l7K8gJvzCZdFM=

; 另一个注释
Address = 10.10.0.2/24

[Peer]
PublicKey = testPeerPublicKeyHereAAAAAAAAAAAAAAAAAAAAA=
# AllowedIPs 设置
AllowedIPs = 10.10.0.1/32
)";

    ParseResult result = ConfigParser::ParseFromString(configText);
    
    EXPECT_TRUE(result.Success);
    EXPECT_FALSE(result.Config.Interface.PrivateKey.empty());
}
