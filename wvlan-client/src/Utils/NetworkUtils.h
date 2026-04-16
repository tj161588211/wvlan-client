/**
 * @file NetworkUtils.h
 * @brief 网络工具类 - 通用网络操作辅助函数
 * 
 * 本模块提供常用的网络操作工具函数。
 * 
 * @version 1.0.0
 * @date 2026-04-15
 * @author Hermes Agent (Technical Director)
 */

#ifndef WVLAN_NETWORK_UTILS_H
#define WVLAN_NETWORK_UTILS_H

#include <string>
#include <vector>

namespace WVLAN {
namespace Core {

class NetworkUtils {
public:
    /// <summary>
    /// 获取本机所有 IPv4 地址
    /// </summary>
    static std::vector<std::string> GetLocalIPv4Addresses();

    /// <summary>
    /// 获取本机所有 IPv6 地址
    /// </summary>
    static std::vector<std::string> GetLocalIPv6Addresses();

    /// <summary>
    /// 获取默认网关地址
    /// </summary>
    static std::string GetDefaultGateway();

    /// <summary>
    /// 检查主机是否可达
    /// </summary>
    static bool IsHostReachable(const std::string& host, int timeoutMs = 1000);

    /// <summary>
    /// DNS 解析
    /// </summary>
    static std::vector<std::string> ResolveHostname(const std::string& hostname);

    /// <summary>
    /// 检测网络类型
    /// </summary>
    enum class NetworkType {
        Unknown,
        LocalNetwork,
        PublicNetwork,
        PrivateNetwork
    };
    
    static NetworkType DetectNetworkType();

    /// <summary>
    /// 检查端口是否可访问
    /// </summary>
    static bool IsPortAccessible(const std::string& host, int port, int timeoutMs = 1000);

private:
    NetworkUtils() = delete;
};

} // namespace Core
} // namespace WVLAN

#endif // WVLAN_NETWORK_UTILS_H
