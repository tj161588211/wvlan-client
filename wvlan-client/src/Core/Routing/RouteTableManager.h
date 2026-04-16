/**
 * @file RouteTableManager.h
 * @brief 路由表管理器 - Windows 路由表操作封装
 * 
 * 本模块提供对 Windows 路由表的读取、添加、删除和修改操作。
 * 支持 IPv4 和 IPv6 路由表管理。
 * 
 * @version 1.0.0
 * @date 2026-04-15
 * @author Hermes Agent (Technical Director)
 */

#ifndef WVLAN_ROUTE_TABLE_MANAGER_H
#define WVLAN_ROUTE_TABLE_MANAGER_H

#include <string>
#include <vector>
#include <cstdint>

namespace WVLAN {
namespace Core {

/// <summary>
/// 路由表项结构
/// </summary>
struct RouteEntry {
    std::string Destination;
    std::string NextHop;
    int InterfaceIndex;
    int Metric;
    std::string InterfaceAddress;
    bool IsIPv6;

    RouteEntry() 
        : InterfaceIndex(0), Metric(1), IsIPv6(false) {}
};

/// <summary>
/// 路由操作结果
/// </summary>
struct RouteResult {
    bool Success;
    std::string ErrorMessage;
    std::vector<std::string> AddedRoutes;
    std::vector<std::string> RemovedRoutes;

    RouteResult() : Success(false) {}
};

/// <summary>
/// 路由表管理器类
/// 
/// 提供 Windows 路由表的完整管理功能：
/// - 获取路由表
/// - 添加路由
/// - 删除路由
/// - 检查路由存在性
/// </summary>
class RouteTableManager {
public:
    /// <summary>
    /// 获取单例实例
    /// </summary>
    static RouteTableManager& Instance();

    /// <summary>
    /// 析构函数
    /// </summary>
    ~RouteTableManager();

    // 禁止拷贝
    RouteTableManager(const RouteTableManager&) = delete;
    RouteTableManager& operator=(const RouteTableManager&) = delete;

    // ========================================================================
    // 路由表查询
    // ========================================================================

    /// <summary>
    /// 获取所有 IPv4 路由表项
    /// </summary>
    /// <returns>路由表项列表</returns>
    std::vector<RouteEntry> GetIPv4RouteTable();

    /// <summary>
    /// 获取所有 IPv6 路由表项
    /// </summary>
    /// <returns>路由表项列表</returns>
    std::vector<RouteEntry> GetIPv6RouteTable();

    /// <summary>
    /// 获取到达指定地址的路由
    /// </summary>
    /// <param name="destination">目标地址</param>
    /// <returns>路由表项，未找到返回空对象</returns>
    RouteEntry GetRouteTo(const std::string& destination);

    /// <summary>
    /// 检查路由是否存在
    /// </summary>
    /// <param name="destination">目标网络地址 (CIDR 格式)</param>
    /// <returns>如果路由存在返回 true</returns>
    bool RouteExists(const std::string& destination);

    // ========================================================================
    // 路由操作
    // ========================================================================

    /// <summary>
    /// 添加路由
    /// </summary>
    /// <param name="destination">目标网络地址 (CIDR 格式)</param>
    /// <param name="gateway">网关地址</param>
    /// <param name="metric">路由度量值</param>
    /// <param name="interfaceIndex">接口索引 (0 表示自动)</param>
    /// <returns>操作结果</returns>
    RouteResult AddRoute(const std::string& destination, 
                         const std::string& gateway,
                         int metric = 1,
                         int interfaceIndex = 0);

    /// <summary>
    /// 删除路由
    /// </summary>
    /// <param name="destination">目标网络地址 (CIDR 格式)</param>
    /// <returns>操作结果</returns>
    RouteResult DeleteRoute(const std::string& destination);

    /// <summary>
    /// 添加多个路由
    /// </summary>
    /// <param name="routes">路由配置列表</param>
    /// <returns>操作结果</returns>
    RouteResult AddRoutes(const std::vector<std::tuple<std::string, std::string, int>>& routes);

    /// <summary>
    /// 删除多个路由
    /// </summary>
    /// <param name="destinations">目标地址列表</param>
    /// <returns>操作结果</returns>
    RouteResult DeleteRoutes(const std::vector<std::string>& destinations);

    // ========================================================================
    // 接口相关
    // ========================================================================

    /// <summary>
    /// 获取接口索引（通过接口名称）
    /// </summary>
    /// <param name="interfaceName">接口名称</param>
    /// <returns>接口索引，未找到返回 0</returns>
    int GetInterfaceIndex(const std::string& interfaceName);

    /// <summary>
    /// 获取接口名称（通过接口索引）
    /// </summary>
    /// <param name="interfaceIndex">接口索引</param>
    /// <returns>接口名称，未找到返回空字符串</returns>
    std::string GetInterfaceName(int interfaceIndex);

    /// <summary>
    /// 获取所有网络接口信息
    /// </summary>
    /// <returns>接口名称列表</returns>
    std::vector<std::string> GetAllInterfaces();

    // ========================================================================
    // 辅助功能
    // ========================================================================

    /// <summary>
    /// 检查是否以管理员权限运行
    /// </summary>
    /// <returns>如果是管理员返回 true</returns>
    static bool IsRunAsAdmin();

    /// <summary>
    /// 将 CIDR 地址转换为子网掩码
    /// </summary>
    /// <param name="cidr">CIDR 格式地址</param>
    /// <returns>子网掩码字符串</returns>
    static std::string CIDRToNetmask(const std::string& cidr);

    /// <summary>
    /// 从 CIDR 地址提取掩码位数
    /// </summary>
    /// <param name="cidr">CIDR 格式地址</param>
    /// <returns>掩码位数</returns>
    static int GetCIDRMask(const std::string& cidr);

private:
    /// <summary>
    /// 私有构造函数
    /// </summary>
    RouteTableManager();

    /// <summary>
    /// 执行路由命令
    /// </summary>
    RouteResult ExecuteRouteCommand(const std::string& command);
};

} // namespace Core
} // namespace WVLAN

#endif // WVLAN_ROUTE_TABLE_MANAGER_H
