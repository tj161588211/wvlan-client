#pragma once

/**
 * @file WireGuardBridge.h
 * @brief WireGuard 驱动桥接层 - 托管代码封装
 * 
 * 本模块提供对 WireGuard 驱动功能的托管代码封装。
 * 通过 C++/CLI 技术，将原生 WireGuard API 转换为 .NET 可使用的类。
 * 
 * @version 1.0.0
 * @date 2026-04-15
 * @author Hermes Agent (Technical Director)
 */

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;

namespace WVLAN::NativeBridge {

    /// <summary>
    /// WireGuard 接口状态枚举
    /// </summary>
    public enum class WgInterfaceState {
        Down = 0,
        Up = 1,
        Connecting = 2,
        Error = 3
    };

    /// <summary>
    /// WireGuard 对等节点状态
    /// </summary>
    public ref class WgPeerStatus {
    public:
        property String^ PublicKey;
        property String^ Endpoint;
        property long ReceivedBytes;
        property long TransmittedBytes;
        property DateTime LastHandshake;
        property int PersistentKeepalive;

        WgPeerStatus() {
            ReceivedBytes = 0;
            TransmittedBytes = 0;
            LastHandshake = DateTime::MinValue;
            PersistentKeepalive = 0;
        }
    };

    /// <summary>
    /// WireGuard 接口统计信息
    /// </summary>
    public ref class WgInterfaceStats {
    public:
        property long TotalReceivedBytes;
        property long TotalTransmittedBytes;
        property int ActivePeers;
        property DateTime LastActivity;

        WgInterfaceStats() {
            TotalReceivedBytes = 0;
            TotalTransmittedBytes = 0;
            ActivePeers = 0;
            LastActivity = DateTime::MinValue;
        }
    };

    /// <summary>
    /// WireGuard 客户端桥接类
    /// 
    /// 封装 WireGuard 驱动的所有操作，包括接口创建、配置管理、
    /// 对等节点添加/删除、状态查询等。
    /// </summary>
    public ref class WireGuardBridge {
    public:
        /// <summary>
        /// 构造函数
        /// </summary>
        WireGuardBridge();

        /// <summary>
        /// 析构函数
        /// </summary>
        ~WireGuardBridge();

        /// <summary>
        /// 初始化 WireGuard 接口
        /// </summary>
        /// <param name="interfaceName">接口名称</param>
        /// <param name="privateKey">Base64 编码的私钥</param>
        /// <param name="listenPort">监听端口 (1-65535)</param>
        /// <returns>操作状态</returns>
        OperationStatus InitializeInterface(String^ interfaceName, String^ privateKey, int listenPort);

        /// <summary>
        /// 设置接口地址
        /// </summary>
        /// <param name="address">IP 地址和子网掩码，如 "10.0.0.2/24"</param>
        /// <returns>操作状态</returns>
        OperationStatus SetInterfaceAddress(String^ address);

        /// <summary>
        /// 添加对等节点
        /// </summary>
        /// <param name="publicKey">对端公钥 (Base64)</param>
        /// <param name="endpoint">对端地址 "IP:Port"</param>
        /// <param name="allowedIPs">允许的 IP 列表，逗号分隔</param>
        /// <returns>操作状态</returns>
        OperationStatus AddPeer(String^ publicKey, String^ endpoint, String^ allowedIPs);

        /// <summary>
        /// 移除对等节点
        /// </summary>
        /// <param name="publicKey">对端公钥 (Base64)</param>
        /// <returns>操作状态</returns>
        OperationStatus RemovePeer(String^ publicKey);

        /// <summary>
        /// 设置持久保活时间
        /// </summary>
        /// <param name="seconds">保活间隔秒数</param>
        /// <returns>操作状态</returns>
        OperationStatus SetPersistentKeepalive(int seconds);

        /// <summary>
        /// 启动接口
        /// </summary>
        /// <returns>操作状态</returns>
        OperationStatus StartInterface();

        /// <summary>
        /// 停止接口
        /// </summary>
        /// <returns>操作状态</returns>
        OperationStatus StopInterface();

        /// <summary>
        /// 关闭并删除接口
        /// </summary>
        /// <returns>操作状态</returns>
        OperationStatus ShutdownInterface();

        /// <summary>
        /// 获取接口状态
        /// </summary>
        /// <returns>接口状态枚举</returns>
        WgInterfaceState GetInterfaceState();

        /// <summary>
        /// 获取接口统计信息
        /// </summary>
        /// <returns>统计信息对象</returns>
        WgInterfaceStats^ GetInterfaceStats();

        /// <summary>
        /// 获取所有对等节点状态
        /// </summary>
        /// <returns>对等节点状态列表</returns>
        IList<WgPeerStatus^>^ GetPeersStatus();

        /// <summary>
        /// 检查 WireGuard 驱动是否已安装
        /// </summary>
        /// <returns>如果驱动已安装返回 true</returns>
        static bool IsDriverInstalled();

        /// <summary>
        /// 安装 WireGuard 驱动
        /// </summary>
        /// <returns>操作状态</returns>
        static OperationStatus InstallDriver();

        /// <summary>
        /// 卸载 WireGuard 驱动
        /// </summary>
        /// <returns>操作状态</returns>
        static OperationStatus UninstallDriver();

    internal:
        // 原生设备句柄
        void* _deviceHandle;
        String^ _interfaceName;
        bool _isInitialized;

        // 内部辅助方法
        static String^ ConvertByteArrayToBase64(array<Byte>^ data);
        static array<Byte>^ ConvertBase64ToByteArray(String^ base64);
    };

} // namespace WVLAN::NativeBridge
