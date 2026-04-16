/**
 * @file RouteConfigurator.cpp
 * @brief 路由配置器实现
 */

#include "RouteConfigurator.h"
#include <sstream>
#include <algorithm>

namespace WVLAN {
namespace Core {

RouteConfigurator::RouteConfigurator() 
    : _routeManager(RouteTableManager::Instance()) {
}

RouteConfigurator::~RouteConfigurator() {
    // 清理时可以选择保留或移除配置的路由
}

std::vector<std::string> RouteConfigurator::ParseAllowedIPs(const std::string& allowedIPs) {
    std::vector<std::string> result;
    
    if (allowedIPs.empty()) {
        return result;
    }

    std::stringstream ss(allowedIPs);
    std::string item;
    
    while (std::getline(ss, item, ',')) {
        // 去除空白
        item.erase(0, item.find_first_not_of(" \t\r\n"));
        item.erase(item.find_last_not_of(" \t\r\n") + 1);
        
        if (!item.empty()) {
            result.push_back(item);
        }
    }

    return result;
}

void RouteConfigurator::AddToConfiguredRoutes(const std::string& route) {
    if (std::find(_configuredRoutes.begin(), _configuredRoutes.end(), route) == _configuredRoutes.end()) {
        _configuredRoutes.push_back(route);
    }
}

void RouteConfigurator::RemoveFromConfiguredRoutes(const std::string& route) {
    _configuredRoutes.erase(
        std::remove(_configuredRoutes.begin(), _configuredRoutes.end(), route),
        _configuredRoutes.end()
    );
}

RouteResult RouteConfigurator::ConfigureWGInterface(int interfaceIndex,
                                                      const std::string& allowedIPs,
                                                      int metric) {
    RouteResult result;
    result.Success = true;

    if (interfaceIndex <= 0) {
        result.ErrorMessage = "无效的接口索引";
        return result;
    }

    std::vector<std::string> routes = ParseAllowedIPs(allowedIPs);
    
    for (const auto& route : routes) {
        RouteResult r = AddAllowedIPRoute(route, interfaceIndex, metric);
        if (r.Success) {
            result.AddedRoutes.push_back(route);
        } else {
            result.Success = false;
            // 继续添加其他路由
        }
    }

    if (result.AddedRoutes.empty() && !result.Success) {
        result.ErrorMessage = "未能添加任何路由";
    }

    return result;
}

RouteResult RouteConfigurator::ClearWGInterfaceRoutes(int interfaceIndex) {
    RouteResult result;
    result.Success = true;

    // 移除所有已配置的路由
    for (const auto& route : _configuredRoutes) {
        RouteResult r = _routeManager.DeleteRoute(route);
        if (r.Success) {
            result.RemovedRoutes.push_back(route);
        } else {
            result.Success = false;
        }
    }

    _configuredRoutes.clear();
    return result;
}

RouteResult RouteConfigurator::AddAllowedIPRoute(const std::string& cidr,
                                                  int interfaceIndex,
                                                  int metric) {
    RouteResult result;
    result.Success = false;

    if (cidr.empty() || interfaceIndex <= 0) {
        result.ErrorMessage = "无效的参数";
        return result;
    }

    // 检查路由冲突
    if (CheckRouteConflict(cidr)) {
        result.ErrorMessage = "路由冲突";
        return result;
    }

    // 获取接口地址作为网关
    std::string interfaceName = _routeManager.GetInterfaceName(interfaceIndex);
    int wgInterfaceIndex = _routeManager.GetInterfaceIndex(interfaceName);
    
    if (wgInterfaceIndex <= 0) {
        result.ErrorMessage = "无法找到接口";
        return result;
    }

    // 对于直接连接的网络，使用接口索引而不需要网关
    std::string netmask = RouteTableManager::CIDRToNetmask(cidr);
    
    // 使用 route add 命令添加路由
    std::ostringstream cmd;
    cmd << "route add " << cidr << " mask " << netmask;
    
    // 对于 WireGuard 接口，通常不需要指定网关
    // 路由会直接通过接口发送
    
    result = _routeManager.ExecuteRouteCommand(cmd.str());
    
    if (result.Success) {
        AddToConfiguredRoutes(cidr);
    }

    return result;
}

bool RouteConfigurator::CheckRouteConflict(const std::string& newRoute) {
    auto conflicting = GetConflictingRoutes(newRoute);
    return !conflicting.empty();
}

std::vector<RouteEntry> RouteConfigurator::GetConflictingRoutes(const std::string& cidr) {
    std::vector<RouteEntry> conflicts;
    
    auto routes = _routeManager.GetIPv4RouteTable();
    
    int newMask = RouteTableManager::GetCIDRMask(cidr);
    std::string newNet = cidr.substr(0, cidr.find('/'));
    
    for (const auto& route : routes) {
        int routeMask = RouteTableManager::GetCIDRMask(route.Destination);
        
        // 检查路由重叠（简化实现）
        if (newMask != routeMask) {
            // 检查是否一个路由包含另一个
            int smallerMask = std::min(newMask, routeMask);
            int largerMask = std::max(newMask, routeMask);
            
            // 如果掩码差异不大，可能存在冲突
            if (largerMask - smallerMask < 8) {
                conflicts.push_back(route);
            }
        }
    }

    return conflicts;
}

RouteResult RouteConfigurator::ConfigureDefaultRoute(int interfaceIndex, int metric) {
    return AddAllowedIPRoute("0.0.0.0/0", interfaceIndex, metric);
}

RouteResult RouteConfigurator::RemoveDefaultRoute() {
    return _routeManager.DeleteRoute("0.0.0.0");
}

} // namespace Core
} // namespace WVLAN
