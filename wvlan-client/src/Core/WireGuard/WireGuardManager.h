/**
 * @file WireGuardManager.h
 * @brief WireGuard 管理器 - 高层接口管理
 * 
 * 本模块提供 WireGuard 驱动的高级管理功能，包括：
 * - 驱动安装/卸载检测
 * - 设备创建/删除管理
 * - 全局配置管理
 * 
 * @version 1.0.0
 * @date 2026-04-15
 * @author Hermes Agent (Technical Director)
 */

#ifndef WVLAN_WIREGUARD_MANAGER_H
#define WVLAN_WIREGUARD_MANAGER_H

#include "WireGuardDevice.h"
#include <string>
#include <vector>
#include <memory>

// Forward declaration for opaque handle
struct WgDeviceHandle;

namespace WVLAN {
namespace Core {

/// <summary>
/// WireGuard 管理器类
/// 
/// 提供 WireGuard 驱动和设备的全局管理功能。
/// 这是一个单例类，应通过 Instance() 方法获取实例。
/// </summary>
class WireGuardManager {
public:
    /// <summary>
    /// 获取单例实例
    /// </summary>
    static WireGuardManager& Instance();

    /// <summary>
    /// 析构函数
    /// </summary>
    ~WireGuardManager();

    // 禁止拷贝
    WireGuardManager(const WireGuardManager&) = delete;
    WireGuardManager& operator=(const WireGuardManager&) = delete;

    // ========================================================================
    // 驱动管理
    // ========================================================================

    /// <summary>
    /// 检查 WireGuard 驱动是否已安装
    /// </summary>
    /// <returns>true 表示驱动已安装</returns>
    bool IsDriverInstalled() const;

    /// <summary>
    /// 安装 WireGuard 驱动
    /// </summary>
    /// <returns>0 表示成功，负数表示错误码</returns>
    int InstallDriver();

    /// <summary>
    /// 卸载 WireGuard 驱动
    /// </summary>
    /// <returns>0 表示成功，负数表示错误码</returns>
    int UninstallDriver();

    /// <summary>
    /// 获取驱动版本
    /// </summary>
    /// <returns>驱动版本号字符串</returns>
    std::string GetDriverVersion() const;

    // ========================================================================
    // 设备管理
    // ========================================================================

    /// <summary>
    /// 创建设备（低级接口）
    /// </summary>
    /// <param name="interfaceName">接口名称</param>
    /// <param name="privateKey">私钥</param>
    /// <param name="listenPort">监听端口</param>
    /// <returns>0 表示成功</returns>
    int CreateDevice(const char* interfaceName, const char* privateKey, int listenPort);

    /// <summary>
    /// 打开现有设备
    /// </summary>
    /// <param name="interfaceName">接口名称</param>
    /// <returns>0 表示成功</returns>
    int OpenDevice(const char* interfaceName);

    /// <summary>
    /// 关闭设备
    /// </summary>
    /// <returns>0 表示成功</returns>
    int CloseDevice();

    /// <summary>
    /// 移除设备
    /// </summary>
    /// <returns>0 表示成功</returns>
    int RemoveDevice();

    /// <summary>
    /// 设置当前设备句柄
    /// </summary>
    void SetDeviceHandle(WgDeviceHandle* handle);

    /// <summary>
    /// 获取当前设备句柄
    /// </summary>
    WgDeviceHandle* GetDeviceHandle() const;

    // ========================================================================
    // 接口配置
    // ========================================================================

    /// <summary>
    /// 设置接口地址
    /// </summary>
    /// <param name="address">IP 地址和子网掩码</param>
    /// <returns>0 表示成功</returns>
    int SetInterfaceAddress(const char* address);

    /// <summary>
    /// 设置监听端口
    /// </summary>
    /// <param name="port">端口号</param>
    /// <returns>0 表示成功</returns>
    int SetListenPort(int port);

    /// <summary>
    /// 添加对等节点
    /// </summary>
    /// <param name="publicKey">公钥</param>
    /// <param name="endpoint">端点地址</param>
    /// <param name="allowedIPs">允许的 IP 列表</param>
    /// <returns>0 表示成功</returns>
    int AddPeer(const char* publicKey, const char* endpoint, const char* allowedIPs);

    /// <summary>
    /// 移除对等节点
    /// </summary>
    /// <param name="publicKey">公钥</param>
    /// <returns>0 表示成功</returns>
    int RemovePeer(const char* publicKey);

    /// <summary>
    /// 设置持久保活
    /// </summary>
    /// <param name="seconds">保活间隔</param>
    /// <returns>0 表示成功</returns>
    int SetPersistentKeepalive(int seconds);

    // ========================================================================
    // 状态查询
    // ========================================================================

    /// <summary>
    /// 启动设备
    /// </summary>
    /// <returns>0 表示成功</returns>
    int StartDevice();

    /// <summary>
    /// 停止设备
    /// </summary>
    /// <returns>0 表示成功</returns>
    int StopDevice();

    /// <summary>
    /// 获取设备状态
    /// </summary>
    /// <returns>状态码</returns>
    int GetDeviceState() const;

    /// <summary>
    /// 获取设备统计信息
    /// </summary>
    /// <returns>统计信息</returns>
    InterfaceStatistics GetDeviceStatistics() const;

    /// <summary>
    /// 获取对等节点统计信息
    /// </summary>
    /// <returns>对等节点列表</returns>
    std::vector<PeerStatistics> GetPeersStatistics() const;

    /// <summary>
    /// 获取所有可用接口名称
    /// </summary>
    /// <returns>接口名称列表</returns>
    std::vector<std::string> GetAllInterfaces() const;

private:
    /// <summary>
    /// 私有构造函数
    /// </summary>
    WireGuardManager();

    /// <summary>
    /// 初始化 WireGuard 库
    /// </summary>
    int InitializeWgLibrary();

    /// <summary>
    /// 释放 WireGuard 库资源
    /// </summary>
    void CleanupWgLibrary();

    // 成员变量
    WgDeviceHandle* _currentDevice;
    bool _libraryInitialized;
    int _referenceCount;
};

} // namespace Core
} // namespace WVLAN

#endif // WVLAN_WIREGUARD_MANAGER_H
