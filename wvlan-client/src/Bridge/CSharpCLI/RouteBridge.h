#pragma once

/**
 * @file RouteBridge.h
 * @brief 路由配置桥接层 - 路由表管理的托管封装
 * 
 * 本模块提供对 Windows 路由表操作的托管代码封装。
 * 支持添加、删除、查询路由表项，以及自动路由配置功能。
 * 
 * @version 1.0.0
 * @date 2026-04-15
 * @author Hermes Agent (Technical Director)
 */

using namespace System;
using namespace System::Collections::Generic;

namespace WVLAN::NativeBridge {

    /// <summary>
    /// 路由表项信息
    /// </summary>
    public ref class RouteEntry {
    public:
        property String^ Destination;
        property String^ NextHop;
        property int InterfaceIndex;
        property int Metric;
        property String^ InterfaceAddress;

        RouteEntry() {
            Metric = 1;
            InterfaceIndex = 0;
        }
    };

    /// <summary>
    /// 路由配置结果
    /// </summary>
    public ref class RouteConfigResult {
    public:
        property bool Success;
        property String^ ErrorMessage;
        property IList<String^>^ AddedRoutes;
        property IList<String^>^ RemovedRoutes;

        RouteConfigResult() {
            Success = false;
            AddedRoutes = gcnew List<String^>();
            RemovedRoutes = gcnew List<String^>();
        }
    };

    /// <summary>
    /// 路由配置桥接类
    /// 
    /// 提供 WireGuard 虚拟网络的路由配置功能，包括：
    /// - 添加隧道路由
    /// - 添加允许的 IP 段路由
    /// - 移除已配置的路由
    /// - 获取当前路由表信息
    /// </summary>
    public ref class RouteBridge {
    public:
        /// <summary>
        /// 构造函数
        /// </summary>
        RouteBridge();

        /// <summary>
        /// 析构函数
        /// </summary>
        ~RouteBridge();

        /// <summary>
        /// 添加单个路由
        /// </summary>
        /// <param name="destination">目标网络地址 (CIDR 格式)</param>
        /// <param name="gateway">网关地址</param>
        /// <param name="metric">路由度量值</param>
        /// <returns>操作状态</returns>
        OperationStatus AddRoute(String^ destination, String^ gateway, int metric);

        /// <summary>
        /// 添加单个路由（指定接口）
        /// </summary>
        /// <param name="destination">目标网络地址</param>
        /// <param name="interfaceIndex">接口索引</param>
        /// <param name="metric">路由度量值</param>
        /// <returns>操作状态</returns>
        OperationStatus AddRouteWithInterface(String^ destination, int interfaceIndex, int metric);

        /// <summary>
        /// 删除单个路由
        /// </summary>
        /// <param name="destination">目标网络地址</param>
        /// <returns>操作状态</returns>
        OperationStatus DeleteRoute(String^ destination);

        /// <summary>
        /// 配置 WireGuard 接口路由
        /// 
        /// 为指定的 WireGuard 接口配置完整的路由规则，包括：
        /// - 隧道接口自身路由
        /// - AllowedIPs 列表中的所有路由
        /// </summary>
        /// <param name="interfaceIndex">WireGuard 接口索引</param>
        /// <param name="allowedIPs">AllowedIPs 列表（逗号分隔）</param>
        /// <param name="metric">路由度量值</param>
        /// <returns>配置结果</returns>
        RouteConfigResult^ ConfigureWGInterfaceRoutes(int interfaceIndex, String^ allowedIPs, int metric);

        /// <summary>
        /// 清除 WireGuard 相关的所有路由
        /// </summary>
        /// <param name="interfaceIndex">WireGuard 接口索引</param>
        /// <returns>操作状态</returns>
        OperationStatus ClearWGInterfaceRoutes(int interfaceIndex);

        /// <summary>
        /// 获取所有 IPv4 路由表项
        /// </summary>
        /// <returns>路由表项列表</returns>
        IList<RouteEntry^>^ GetIPv4RouteTable();

        /// <summary>
        /// 获取所有 IPv6 路由表项
        /// </summary>
        /// <returns>路由表项列表</returns>
        IList<RouteEntry^>^ GetIPv6RouteTable();

        /// <summary>
        /// 检查路由是否存在
        /// </summary>
        /// <param name="destination">目标网络地址</param>
        /// <returns>如果路由存在返回 true</returns>
        bool RouteExists(String^ destination);

        /// <summary>
        /// 获取到达指定地址的路由
        /// </summary>
        /// <param name="destination">目标地址</param>
        /// <returns>路由表项或 null</returns>
        RouteEntry^ GetRouteTo(String^ destination);

        /// <summary>
        /// 获取接口索引（通过接口名称）
        /// </summary>
        /// <param name="interfaceName">接口名称</param>
        /// <returns>接口索引，未找到返回 -1</returns>
        int GetInterfaceIndex(String^ interfaceName);

        /// <summary>
        /// 获取接口名称（通过接口索引）
        /// </summary>
        /// <param name="interfaceIndex">接口索引</param>
        /// <returns>接口名称，未找到返回空字符串</returns>
        String^ GetInterfaceName(int interfaceIndex);

    internal:
        // 内部辅助方法
        static OperationStatus ExecuteRouteCommand(String^ command);
        static String^ ConvertCIDRToNetmask(String^ cidr);
        static int ParseCIDRMask(String^ cidr);
    };

} // namespace WVLAN::NativeBridge
