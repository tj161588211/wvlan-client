#pragma once

/**
 * @file ConfigBridge.h
 * @brief 配置管理桥接层 - WireGuard 配置文件的托管封装
 * 
 * 本模块提供对 WireGuard 配置文件的解析、验证和管理的托管代码封装。
 * 支持标准 WireGuard .conf 文件格式的导入导出。
 * 
 * @version 1.0.0
 * @date 2026-04-15
 * @author Hermes Agent (Technical Director)
 */

using namespace System;
using namespace System::Collections::Generic;

namespace WVLAN::NativeBridge {

    /// <summary>
    /// 对等节点配置信息
    /// </summary>
    public ref class PeerConfig {
    public:
        property String^ PublicKey;
        property String^ Endpoint;
        property String^ AllowedIPs;
        property int PersistentKeepalive;

        PeerConfig() {
            PersistentKeepalive = 0;
            AllowedIPs = "0.0.0.0/0";
        }
    };

    /// <summary>
    /// 接口配置信息
    /// </summary>
    public ref class InterfaceConfig {
    public:
        property String^ PrivateKey;
        property String^ Address;
        property String^ DNS;
        property int ListenPort;

        InterfaceConfig() {
            ListenPort = 51820;
            DNS = "8.8.8.8,8.8.4.4";
        }
    };

    /// <summary>
    /// 完整网络配置文件
    /// </summary>
    public ref class NetworkProfile {
    public:
        property String^ ProfileName;
        property InterfaceConfig^ Interface;
        property IList<PeerConfig^>^ Peers;
        property DateTime CreatedAt;
        property DateTime UpdatedAt;

        NetworkProfile() {
            Interface = gcnew InterfaceConfig();
            Peers = gcnew List<PeerConfig^>();
            CreatedAt = DateTime::Now;
            UpdatedAt = DateTime::Now;
        }
    };

    /// <summary>
    /// 配置解析结果
    /// </summary>
    public ref class ConfigParseResult {
    public:
        property bool Success;
        property String^ ErrorMessage;
        property NetworkProfile^ Profile;

        ConfigParseResult() {
            Success = false;
            Profile = nullptr;
        }
    };

    /// <summary>
    /// 配置管理桥接类
    /// 
    /// 提供 WireGuard 配置文件的解析、验证、保存和加载功能。
    /// 支持标准 .conf 文件格式和内部 JSON 格式。
    /// </summary>
    public ref class ConfigBridge {
    public:
        /// <summary>
        /// 构造函数
        /// </summary>
        ConfigBridge();

        /// <summary>
        /// 从文件路径解析配置文件
        /// </summary>
        /// <param name="filePath">配置文件路径</param>
        /// <returns>解析结果</returns>
        ConfigParseResult^ ParseConfigFile(String^ filePath);

        /// <summary>
        /// 从配置文本解析
        /// </summary>
        /// <param name="configText">配置文本内容</param>
        /// <returns>解析结果</returns>
        ConfigParseResult^ ParseConfigText(String^ configText);

        /// <summary>
        /// 验证配置有效性
        /// </summary>
        /// <param name="profile">网络配置文件</param>
        /// <returns>验证是否通过</returns>
        bool ValidateConfig(NetworkProfile^ profile);

        /// <summary>
        /// 保存配置到文件
        /// </summary>
        /// <param name="profile">网络配置文件</param>
        /// <param name="filePath">输出文件路径</param>
        /// <returns>操作状态</returns>
        OperationStatus SaveConfigToFile(NetworkProfile^ profile, String^ filePath);

        /// <summary>
        /// 从文件加载配置
        /// </summary>
        /// <param name="filePath">配置文件路径</param>
        /// <returns>网络配置文件</returns>
        NetworkProfile^ LoadConfigFromFile(String^ filePath);

        /// <summary>
        /// 导出配置为标准 WireGuard 格式文本
        /// </summary>
        /// <param name="profile">网络配置文件</param>
        /// <returns>配置文本</returns>
        String^ ExportToWireGuardFormat(NetworkProfile^ profile);

        /// <summary>
        /// 生成新的密钥对
        /// </summary>
        /// <returns>包含公钥和私钥的字典</returns>
        IDictionary<String^, String^>^ GenerateKeyPair();

        /// <summary>
        /// 从私钥计算公钥
        /// </summary>
        /// <param name="privateKey">Base64 私钥</param>
        /// <returns>Base64 公钥</returns>
        String^ GetPublicKeyFromPrivateKey(String^ privateKey);

        /// <summary>
        /// 保存配置到本地存储
        /// </summary>
        /// <param name="profile">网络配置文件</param>
        /// <returns>操作状态</returns>
        OperationStatus SaveProfileToStorage(NetworkProfile^ profile);

        /// <summary>
        /// 从本地存储加载配置
        /// </summary>
        /// <param name="profileName">配置文件名称</param>
        /// <returns>网络配置文件</returns>
        NetworkProfile^ LoadProfileFromStorage(String^ profileName);

        /// <summary>
        /// 获取所有保存的配置列表
        /// </summary>
        /// <returns>配置名称列表</returns>
        IList<String^>^ GetAllProfileNames();

        /// <summary>
        /// 删除保存的配置
        /// </summary>
        /// <param name="profileName">配置文件名称</param>
        /// <returns>操作状态</returns>
        OperationStatus DeleteProfileFromStorage(String^ profileName);

    internal:
        // 存储路径
        String^ _storagePath;

        // 内部辅助方法
        static bool IsValidBase64Key(String^ key);
        static bool IsValidIPAddress(String^ ip);
        static bool IsValidEndpoint(String^ endpoint);
    };

} // namespace WVLAN::NativeBridge
