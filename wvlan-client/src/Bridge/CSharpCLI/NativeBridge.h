#pragma once

/**
 * @file NativeBridge.h
 * @brief C++/CLI 桥接层 - 托管与非托管代码互操作
 * 
 * 本模块提供 .NET 托管代码与原生 C++ 代码之间的桥接功能。
 * 通过 C++/CLI 技术，实现类型转换、内存管理和异常处理的无缝集成。
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
    /// 网络类型枚举
    /// </summary>
    public enum class NetworkType {
        Unknown = 0,
        LocalNetwork = 1,
        PublicNetwork = 2,
        PrivateNetwork = 3
    };

    /// <summary>
    /// 操作结果状态码
    /// </summary>
    public enum class OperationStatus {
        Success = 0,
        InvalidParameter = -1,
        AccessDenied = -2,
        NetworkError = -3,
        Timeout = -4,
        NotFound = -5,
        AlreadyExists = -6,
        InternalError = -99
    };

    /// <summary>
    /// 网络操作工具类
    /// 
    /// 提供底层网络操作功能，包括 IP 地址获取、网络类型检测、
    /// 路由表操作等。所有操作都需要管理员权限。
    /// </summary>
    public ref class NetworkOperations sealed {
    public:
        /// <summary>
        /// 获取本地 IP 地址列表
        /// </summary>
        /// <returns>本地 IP 地址字符串数组</returns>
        cli::array<String^>^ GetLocalIPAddresses();

        /// <summary>
        /// 检测当前网络类型
        /// </summary>
        /// <returns>网络类型枚举值</returns>
        NetworkType DetectNetworkType();

        /// <summary>
        /// 检查是否以管理员权限运行
        /// </summary>
        /// <returns>如果是管理员权限返回 true</returns>
        static bool IsRunAsAdmin();

        /// <summary>
        /// 添加路由表项
        /// </summary>
        /// <param name="destination">目标网络地址</param>
        /// <param name="gateway">网关地址</param>
        /// <param name="metric">路由度量值</param>
        /// <returns>操作状态</returns>
        OperationStatus AddRoute(String^ destination, String^ gateway, int metric);

        /// <summary>
        /// 删除路由表项
        /// </summary>
        /// <param name="destination">目标网络地址</param>
        /// <returns>操作状态</returns>
        OperationStatus DeleteRoute(String^ destination);

        /// <summary>
        /// 获取网络接口信息
        /// </summary>
        /// <returns>接口信息列表</returns>
        IList<NetworkInterfaceInfo^>^ GetNetworkInterfaces();

    internal:
        /// <summary>
        /// 原生实现调用
        /// </summary>
        static OperationStatus AddRouteNative(const char* destination, const char* gateway, int metric);
        static OperationStatus DeleteRouteNative(const char* destination);
    };

    /// <summary>
    /// 网络接口信息类
    /// </summary>
    public ref class NetworkInterfaceInfo {
    public:
        property String^ Name;
        property String^ Description;
        property String^ IPAddress;
        property String^ MACAddress;
        property bool IsWireGuard;

        NetworkInterfaceInfo(String^ name, String^ desc, String^ ip, String^ mac, bool isWg) {
            Name = name;
            Description = desc;
            IPAddress = ip;
            MACAddress = mac;
            IsWireGuard = isWg;
        }
    };

} // namespace WVLAN::NativeBridge
