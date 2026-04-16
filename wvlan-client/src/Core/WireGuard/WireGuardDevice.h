/**
 * @file WireGuardDevice.h
 * @brief WireGuard 设备封装类 - 原生 C++ 实现
 * 
 * 本模块封装 WireGuard-for-C 的底层 API，提供设备级别的接口管理。
 * 包括设备创建、配置、启动、停止等功能。
 * 
 * @version 1.0.0
 * @date 2026-04-15
 * @author Hermes Agent (Technical Director)
 */

#ifndef WVLAN_WIREGUARD_DEVICE_H
#define WVLAN_WIREGUARD_DEVICE_H

#include <string>
#include <vector>
#include <cstdint>

namespace WVLAN {
namespace Core {

/// <summary>
/// 设备句柄类型定义
/// </summary>
struct WgDeviceHandle;

/// <summary>
/// 对等节点统计信息
/// </summary>
struct PeerStatistics {
    std::string PublicKey;
    std::string Endpoint;
    int64_t ReceivedBytes;
    int64_t TransmittedBytes;
    uint32_t LastHandshakeTime;
    int PersistentKeepalive;
};

/// <summary>
/// 接口统计信息
/// </summary>
struct InterfaceStatistics {
    int64_t ReceivedBytes;
    int64_t TransmittedBytes;
    int ActivePeers;
    uint32_t LastHandshakeTime;
};

/// <summary>
/// WireGuard 设备类
/// 
/// 封装单个 WireGuard 网络接口的所有操作。
/// 注意：此类不是线程安全的，外部调用需要保证线程安全。
/// </summary>
class WireGuardDevice {
public:
    /// <summary>
    /// 构造函数
    /// </summary>
    WireGuardDevice();

    /// <summary>
    /// 析构函数
    /// </summary>
    ~WireGuardDevice();

    /// <summary>
    /// 禁止拷贝
    /// </summary>
    WireGuardDevice(const WireGuardDevice&) = delete;
    WireGuardDevice& operator=(const WireGuardDevice&) = delete;

    /// <summary>
    /// 创建新的 WireGuard 接口
    /// </summary>
    /// <param name="interfaceName">接口名称</param>
    /// <param name="privateKey">Base64 编码的私钥</param>
    /// <param name="listenPort">监听端口 (0 表示自动分配)</param>
    /// <returns>0 表示成功，负数表示错误码</returns>
    int Create(const std::string& interfaceName, const std::string& privateKey, int listenPort);

    /// <summary>
    /// 打开已存在的接口
    /// </summary>
    /// <param name="interfaceName">接口名称</param>
    /// <returns>0 表示成功，负数表示错误码</returns>
    int Open(const std::string& interfaceName);

    /// <summary>
    /// 关闭接口
    /// </summary>
    /// <returns>0 表示成功，负数表示错误码</returns>
    int Close();

    /// <summary>
    /// 删除接口
    /// </summary>
    /// <returns>0 表示成功，负数表示错误码</returns>
    int Remove();

    /// <summary>
    /// 设置接口地址
    /// </summary>
    /// <param name="address">IP 地址和子网掩码 (CIDR 格式)</param>
    /// <returns>0 表示成功，负数表示错误码</returns>
    int SetAddress(const std::string& address);

    /// <summary>
    /// 设置监听端口
    /// </summary>
    /// <param name="port">端口号</param>
    /// <returns>0 表示成功，负数表示错误码</returns>
    int SetListenPort(int port);

    /// <summary>
    /// 添加对等节点
    /// </summary>
    /// <param name="publicKey">对端公钥 (Base64)</param>
    /// <param name="endpoint">对端地址 "IP:Port"</param>
    /// <param name="allowedIPs">允许的 IP 列表 (逗号分隔的 CIDR)</param>
    /// <returns>0 表示成功，负数表示错误码</returns>
    int AddPeer(const std::string& publicKey, const std::string& endpoint, const std::string& allowedIPs);

    /// <summary>
    /// 移除对等节点
    /// </summary>
    /// <param name="publicKey">对端公钥 (Base64)</param>
    /// <returns>0 表示成功，负数表示错误码</returns>
    int RemovePeer(const std::string& publicKey);

    /// <summary>
    /// 设置持久保活时间
    /// </summary>
    /// <param name="seconds">保活间隔秒数 (0 表示禁用)</param>
    /// <returns>0 表示成功，负数表示错误码</returns>
    int SetPersistentKeepalive(int seconds);

    /// <summary>
    /// 启动接口
    /// </summary>
    /// <returns>0 表示成功，负数表示错误码</returns>
    int Start();

    /// <summary>
    /// 停止接口
    /// </summary>
    /// <returns>0 表示成功，负数表示错误码</returns>
    int Stop();

    /// <summary>
    /// 获取接口状态
    /// </summary>
    /// <returns>0: 未启动，1: 已启动，2: 连接中，3: 错误</returns>
    int GetState() const;

    /// <summary>
    /// 获取接口统计信息
    /// </summary>
    /// <returns>接口统计信息</returns>
    InterfaceStatistics GetStatistics() const;

    /// <summary>
    /// 获取所有对等节点统计信息
    /// </summary>
    /// <returns>对等节点统计信息列表</returns>
    std::vector<PeerStatistics> GetPeersStatistics() const;

    /// <summary>
    /// 获取接口名称
    /// </summary>
    /// <returns>接口名称</returns>
    std::string GetInterfaceName() const;

    /// <summary>
    /// 获取设备句柄（用于桥接层）
    /// </summary>
    /// <returns>设备句柄指针</returns>
    WgDeviceHandle* GetHandle() const;

private:
    WgDeviceHandle* _deviceHandle;
    std::string _interfaceName;
    bool _isCreated;
};

} // namespace Core
} // namespace WVLAN

#endif // WVLAN_WIREGUARD_DEVICE_H
