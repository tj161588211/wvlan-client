/**
 * @file RouteConfigurator.h
 * @brief 路由配置器 - WireGuard 专用路由配置
 * 
 * 本模块提供 WireGuard 接口的专用路由配置功能，包括：
 * - 自动配置 AllowedIPs 路由
 * - 管理隧道接口路由
 * - 路由冲突检测和处理
 * 
 * @version 1.0.0
 * @date 2026-04-15
 * @author Hermes Agent (Technical Director)
 */

#ifndef WVLAN_ROUTE_CONFIGURATOR_H
#define WVLAN_ROUTE_CONFIGURATOR_H

#include "RouteTableManager.h"
#include <string>
#include <vector>

namespace WVLAN {
namespace Core {

/// <summary>
/// 路由配置器类
/// 
/// 专为 WireGuard 接口设计的路由配置工具，提供：
/// - 自动解析 AllowedIPs 并添加路由
/// - 路由冲突检测
/// - 批量路由管理
/// </summary>
class RouteConfigurator {
public:
    /// <summary>
    /// 构造函数
    /// </summary>
    RouteConfigurator();

    /// <summary>
    /// 析构函数
    /// </summary>
    ~RouteConfigurator();

    /// <summary>
    /// 配置 WireGuard 接口路由
    /// 
    /// 根据 AllowedIPs 配置自动添加路由规则。
    /// </summary>
    /// <param name="interfaceIndex">接口索引</param>
    /// <param name="allowedIPs">AllowedIPs 字符串（逗号分隔的 CIDR 列表）</param>
    /// <param name="metric">路由度量值</param>
    /// <returns>路由操作结果</returns>
    RouteResult ConfigureWGInterface(int interfaceIndex, 
                                      const std::string& allowedIPs,
                                      int metric = 1);

    /// <summary>
    /// 清除 WireGuard 接口相关的所有路由
    /// </summary>
    /// <param name="interfaceIndex">接口索引</param>
    /// <returns>路由操作结果</returns>
    RouteResult ClearWGInterfaceRoutes(int interfaceIndex);

    /// <summary>
    /// 添加单个 AllowedIP 路由
    /// </summary>
    /// <param name="cidr">CIDR 地址</param>
    /// <param name="interfaceIndex">接口索引</param>
    /// <param name="metric">路由度量值</param>
    /// <returns>路由操作结果</returns>
    RouteResult AddAllowedIPRoute(const std::string& cidr, 
                                   int interfaceIndex, 
                                   int metric = 1);

    /// <summary>
    /// 解析 AllowedIPs 字符串
    /// </summary>
    /// <param name="allowedIPs">AllowedIPs 字符串</param>
    /// <returns>CIDR 地址列表</returns>
    static std::vector<std::string> ParseAllowedIPs(const std::string& allowedIPs);

    /// <summary>
    /// 检查路由是否与现有路由冲突
    /// </summary>
    /// <param name="newRoute">新路由 CIDR</param>
    /// <returns>如果存在冲突返回 true</returns>
    bool CheckRouteConflict(const std::string& newRoute);

    /// <summary>
    /// 获取与指定 CIDR 冲突的现有路由
    /// </summary>
    /// <param name="cidr">CIDR 地址</param>
    /// <returns>冲突路由列表</returns>
    std::vector<RouteEntry> GetConflictingRoutes(const std::string& cidr);

    /// <summary>
    /// 配置默认路由（0.0.0.0/0）
    /// </summary>
    /// <param name="interfaceIndex">接口索引</param>
    /// <param name="metric">路由度量值</param>
    /// <returns>路由操作结果</returns>
    RouteResult ConfigureDefaultRoute(int interfaceIndex, int metric = 1);

    /// <summary>
    /// 移除默认路由
    /// </summary>
    /// <returns>路由操作结果</returns>
    RouteResult RemoveDefaultRoute();

private:
    RouteTableManager& _routeManager;
    std::vector<std::string> _configuredRoutes;

    /// <summary>
    /// 添加路由到已配置列表
    /// </summary>
    void AddToConfiguredRoutes(const std::string& route);

    /// <summary>
    /// 从已配置列表移除路由
    /// </summary>
    void RemoveFromConfiguredRoutes(const std::string& route);
};

} // namespace Core
} // namespace WVLAN

#endif // WVLAN_ROUTE_CONFIGURATOR_H
