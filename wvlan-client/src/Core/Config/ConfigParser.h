/**
 * @file ConfigParser.h
 * @brief WireGuard 配置解析器
 * 
 * 本模块负责解析标准 WireGuard 配置文件格式 (.conf)。
 * 支持 [Interface] 和 [Peer] 段的解析。
 * 
 * @version 1.0.0
 * @date 2026-04-15
 * @author Hermes Agent (Technical Director)
 */

#ifndef WVLAN_CONFIG_PARSER_H
#define WVLAN_CONFIG_PARSER_H

#include <string>
#include <vector>
#include <map>

namespace WVLAN {
namespace Core {

/// <summary>
/// 对等节点配置
/// </summary>
struct PeerConfig {
    std::string PublicKey;
    std::string PresharedKey;
    std::string Endpoint;
    std::string AllowedIPs;
    int PersistentKeepalive;

    PeerConfig() : PersistentKeepalive(0) {}
};

/// <summary>
/// 接口配置
/// </summary>
struct InterfaceConfig {
    std::string PrivateKey;
    std::string Address;
    std::string DNS;
    int MTU;
    int ListenPort;
    std::string FirewallMark;

    InterfaceConfig() 
        : ListenPort(51820), MTU(1420), FirewallMark("") {}
};

/// <summary>
/// 完整网络配置
/// </summary>
struct NetworkConfig {
    InterfaceConfig Interface;
    std::vector<PeerConfig> Peers;
    std::string FileName;
    std::string Description;

    NetworkConfig() = default;
};

/// <summary>
/// 解析结果
/// </summary>
struct ParseResult {
    bool Success;
    std::string ErrorMessage;
    NetworkConfig Config;

    ParseResult() : Success(false) {}
};

/// <summary>
/// WireGuard 配置解析器类
/// 
/// 提供配置文件读取、解析、验证和导出功能。
/// 支持标准 WireGuard .conf 文件格式。
/// </summary>
class ConfigParser {
public:
    /// <summary>
    /// 从文件路径解析配置
    /// </summary>
    /// <param name="filePath">配置文件路径</param>
    /// <returns>解析结果</returns>
    static ParseResult ParseFromFile(const std::string& filePath);

    /// <summary>
    /// 从文本内容解析配置
    /// </summary>
    /// <param name="content">配置文本内容</param>
    /// <returns>解析结果</returns>
    static ParseResult ParseFromString(const std::string& content);

    /// <summary>
    /// 将配置导出为文本格式
    /// </summary>
    /// <param name="config">网络配置</param>
    /// <returns>配置文本</returns>
    static std::string ExportToString(const NetworkConfig& config);

    /// <summary>
    /// 将配置保存到文件
    /// </summary>
    /// <param name="config">网络配置</param>
    /// <param name="filePath">输出文件路径</param>
    /// <returns>是否成功</returns>
    static bool SaveToFile(const NetworkConfig& config, const std::string& filePath);

    /// <summary>
    /// 验证配置有效性
    /// </summary>
    /// <param name="config">网络配置</param>
    /// <returns>验证是否通过</returns>
    static bool Validate(const NetworkConfig& config);

    /// <summary>
    /// 生成新的密钥对
    /// </summary>
    /// <param name="privateKey">输出的私钥 (Base64)</param>
    /// <param name="publicKey">输出的公钥 (Base64)</param>
    /// <returns>是否成功</returns>
    static bool GenerateKeyPair(std::string& privateKey, std::string& publicKey);

    /// <summary>
    /// 从私钥计算公钥
    /// </summary>
    /// <param name="privateKey">Base64 私钥</param>
    /// <param name="publicKey">输出的 Base64 公钥</param>
    /// <returns>是否成功</returns>
    static bool GetPublicKey(const std::string& privateKey, std::string& publicKey);

    /// <summary>
    /// 验证 Base64 编码的密钥
    /// </summary>
    /// <param name="key">Base64 密钥字符串</param>
    /// <returns>是否有效</returns>
    static bool IsValidKey(const std::string& key);

    /// <summary>
    /// 验证 IP 地址格式
    /// </summary>
    /// <param name="ip">IP 地址字符串</param>
    /// <returns>是否有效</returns>
    static bool IsValidIPAddress(const std::string& ip);

    /// <summary>
    /// 验证端点格式 (IP:Port)
    /// </summary>
    /// <param name="endpoint">端点字符串</param>
    /// <returns>是否有效</returns>
    static bool IsValidEndpoint(const std::string& endpoint);

private:
    /// <summary>
    /// 解析单个配置段
    /// </summary>
    static ParseResult ParseSection(const std::string& sectionName, 
                                     const std::map<std::string, std::string>& values,
                                     NetworkConfig& config);

    /// <summary>
    /// 分割键值对
    /// </summary>
    static void SplitKeyValue(const std::string& line, std::string& key, std::string& value);

    /// <summary>
    /// 去除字符串空白
    /// </summary>
    static std::string Trim(const std::string& str);
};

} // namespace Core
} // namespace WVLAN

#endif // WVLAN_CONFIG_PARSER_H
