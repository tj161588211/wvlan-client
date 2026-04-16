/**
 * @file RouteTableManager.cpp
 * @brief 路由表管理器实现
 */

#include "RouteTableManager.h"
#include <windows.h>
#include <iphlpapi.h>
#include <vector>
#include <sstream>
#include <regex>
#include <cstring>

#pragma comment(lib, "iphlpapi.lib")

namespace WVLAN {
namespace Core {

// 错误码定义
enum {
    ERROR_SUCCESS = 0,
    ERROR_INVALID_PARAM = -1,
    ERROR_ACCESS_DENIED = -2,
    ERROR_NOT_FOUND = -3,
    ERROR_ALREADY_EXISTS = -4,
    ERROR_NETWORK_FAILED = -5,
    ERROR_INTERNAL = -99
};

static RouteTableManager* g_instance = nullptr;

RouteTableManager& RouteTableManager::Instance() {
    if (g_instance == nullptr) {
        g_instance = new RouteTableManager();
    }
    return *g_instance;
}

RouteTableManager::RouteTableManager() {
}

RouteTableManager::~RouteTableManager() {
    if (g_instance == this) {
        g_instance = nullptr;
    }
}

bool RouteTableManager::IsRunAsAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = nullptr;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(
            &ntAuthority, 2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &adminGroup)) {
        
        if (!CheckTokenMembership(nullptr, adminGroup, &isAdmin)) {
            isAdmin = FALSE;
        }
        FreeSid(adminGroup);
    }

    return (isAdmin != FALSE);
}

std::string RouteTableManager::CIDRToNetmask(const std::string& cidr) {
    size_t slashPos = cidr.find('/');
    if (slashPos == std::string::npos) {
        return "255.255.255.255";
    }

    int maskBits = std::stoi(cidr.substr(slashPos + 1));
    
    uint32_t mask = 0;
    if (maskBits > 0) {
        mask = ~((1 << (32 - maskBits)) - 1);
    }

    std::ostringstream oss;
    oss << ((mask >> 24) & 0xFF) << "."
        << ((mask >> 16) & 0xFF) << "."
        << ((mask >> 8) & 0xFF) << "."
        << (mask & 0xFF);

    return oss.str();
}

int RouteTableManager::GetCIDRMask(const std::string& cidr) {
    size_t slashPos = cidr.find('/');
    if (slashPos == std::string::npos) {
        return 32;
    }

    try {
        return std::stoi(cidr.substr(slashPos + 1));
    } catch (...) {
        return 32;
    }
}

std::vector<RouteEntry> RouteTableManager::GetIPv4RouteTable() {
    std::vector<RouteEntry> routes;

    MIB_IPFORWARD_TABLE2* pTable = nullptr;
    ULONG ulSize = 0;

    DWORD result = GetIpForwardTable2(AF_INET, &pTable);
    if (result != NO_ERROR || pTable == nullptr) {
        return routes;
    }

    for (ULONG i = 0; i < pTable->NumEntries; i++) {
        MIB_IPFORWARD_ROW2* row = &pTable->Table[i];

        RouteEntry entry;
        entry.IsIPv6 = false;
        entry.InterfaceIndex = (int)row->InterfaceIndex;
        entry.Metric = (int)row->Metric;

        // 转换目标地址
        char destStr[INET_ADDRSTRLEN] = {0};
        InetNtopA(AF_INET, &row->DestinationPrefix.Prefix.Ipv4.sin_addr, 
                  destStr, INET_ADDRSTRLEN);
        entry.Destination = destStr;

        // 转换下一跳地址
        char nextHopStr[INET_ADDRSTRLEN] = {0};
        InetNtopA(AF_INET, &row->NextHop.Ipv4.sin_addr, 
                  nextHopStr, INET_ADDRSTRLEN);
        entry.NextHop = nextHopStr;

        routes.push_back(entry);
    }

    FreeMibTable(pTable);
    return routes;
}

std::vector<RouteEntry> RouteTableManager::GetIPv6RouteTable() {
    std::vector<RouteEntry> routes;

    MIB_IPFORWARD_TABLE2* pTable = nullptr;
    ULONG ulSize = 0;

    DWORD result = GetIpForwardTable2(AF_INET6, &pTable);
    if (result != NO_ERROR || pTable == nullptr) {
        return routes;
    }

    for (ULONG i = 0; i < pTable->NumEntries; i++) {
        MIB_IPFORWARD_ROW2* row = &pTable->Table[i];

        RouteEntry entry;
        entry.IsIPv6 = true;
        entry.InterfaceIndex = (int)row->InterfaceIndex;
        entry.Metric = (int)row->Metric;

        // 转换目标地址
        char destStr[INET6_ADDRSTRLEN] = {0};
        InetNtopA(AF_INET6, &row->DestinationPrefix.Prefix.Ipv6.sin6_addr, 
                  destStr, INET6_ADDRSTRLEN);
        entry.Destination = destStr;

        // 转换下一跳地址
        char nextHopStr[INET6_ADDRSTRLEN] = {0};
        InetNtopA(AF_INET6, &row->NextHop.Ipv6.sin6_addr, 
                  nextHopStr, INET6_ADDRSTRLEN);
        entry.NextHop = nextHopStr;

        routes.push_back(entry);
    }

    FreeMibTable(pTable);
    return routes;
}

RouteEntry RouteTableManager::GetRouteTo(const std::string& destination) {
    RouteEntry bestRoute;

    auto ipv4Routes = GetIPv4RouteTable();
    for (const auto& route : ipv4Routes) {
        // 简化实现：检查是否匹配
        if (route.NextHop == "0.0.0.0" || route.Destination == "0.0.0.0") {
            if (bestRoute.Metric == 0 || route.Metric < bestRoute.Metric) {
                bestRoute = route;
            }
        }
    }

    return bestRoute;
}

bool RouteTableManager::RouteExists(const std::string& destination) {
    auto routes = GetIPv4RouteTable();
    std::string netmask = CIDRToNetmask(destination);

    for (const auto& route : routes) {
        if (route.Destination == netmask) {
            return true;
        }
    }

    return false;
}

RouteResult RouteTableManager::AddRoute(const std::string& destination,
                                         const std::string& gateway,
                                         int metric,
                                         int interfaceIndex) {
    RouteResult result;
    result.Success = false;

    if (!IsRunAsAdmin()) {
        result.ErrorMessage = "需要管理员权限";
        return result;
    }

    if (destination.empty() || gateway.empty()) {
        result.ErrorMessage = "无效的参数";
        return result;
    }

    // 构建 route add 命令
    std::ostringstream cmd;
    cmd << "route add " << destination << " mask " << CIDRToNetmask(destination) 
        << " " << gateway;
    
    if (metric > 0) {
        cmd << " metric " << metric;
    }
    
    if (interfaceIndex > 0) {
        cmd << " if " << interfaceIndex;
    }

    return ExecuteRouteCommand(cmd.str());
}

RouteResult RouteTableManager::DeleteRoute(const std::string& destination) {
    RouteResult result;
    result.Success = false;

    if (!IsRunAsAdmin()) {
        result.ErrorMessage = "需要管理员权限";
        return result;
    }

    if (destination.empty()) {
        result.ErrorMessage = "无效的参数";
        return result;
    }

    std::ostringstream cmd;
    cmd << "route delete " << destination;

    return ExecuteRouteCommand(cmd.str());
}

RouteResult RouteTableManager::AddRoutes(const std::vector<std::tuple<std::string, std::string, int>>& routes) {
    RouteResult result;
    result.Success = true;

    for (const auto& route : routes) {
        RouteResult r = AddRoute(std::get<0>(route), std::get<1>(route), std::get<2>(route));
        if (r.Success) {
            result.AddedRoutes.push_back(std::get<0>(route));
        } else {
            result.Success = false;
        }
    }

    return result;
}

RouteResult RouteTableManager::DeleteRoutes(const std::vector<std::string>& destinations) {
    RouteResult result;
    result.Success = true;

    for (const auto& dest : destinations) {
        RouteResult r = DeleteRoute(dest);
        if (r.Success) {
            result.RemovedRoutes.push_back(dest);
        } else {
            result.Success = false;
        }
    }

    return result;
}

RouteResult RouteTableManager::ExecuteRouteCommand(const std::string& command) {
    RouteResult result;
    result.Success = false;

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    char* cmdCopy = _strdup(command.c_str());
    BOOL created = CreateProcessA(
        nullptr, cmdCopy,
        nullptr, nullptr, FALSE,
        CREATE_NO_WINDOW,
        nullptr, nullptr,
        &si, &pi
    );
    free(cmdCopy);

    if (!created) {
        result.ErrorMessage = "执行命令失败";
        return result;
    }

    WaitForSingleObject(pi.hProcess, 5000);
    
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    result.Success = (exitCode == 0);
    if (!result.Success) {
        result.ErrorMessage = "命令执行返回错误代码：" + std::to_string(exitCode);
    }

    return result;
}

int RouteTableManager::GetInterfaceIndex(const std::string& interfaceName) {
    PIP_ADAPTER_ADDRESSES pAddresses = nullptr;
    ULONG outBufLen = 16384;

    do {
        pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
        if (pAddresses == nullptr) return 0;

        DWORD result = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, pAddresses, &outBufLen);
        if (result == ERROR_BUFFER_OVERFLOW) {
            free(pAddresses);
            continue;
        }
        if (result != NO_ERROR) {
            free(pAddresses);
            return 0;
        }

        for (PIP_ADAPTER_ADDRESSES pCurr = pAddresses; pCurr != nullptr; pCurr = pCurr->Next) {
            std::string name(pCurr->FriendlyName);
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            
            std::string targetName = interfaceName;
            std::transform(targetName.begin(), targetName.end(), targetName.begin(), ::tolower);

            if (name.find(targetName) != std::string::npos) {
                free(pAddresses);
                return (int)pCurr->IfIndex;
            }
        }
        break;
    } while (false);

    if (pAddresses) free(pAddresses);
    return 0;
}

std::string RouteTableManager::GetInterfaceName(int interfaceIndex) {
    PIP_ADAPTER_ADDRESSES pAddresses = nullptr;
    ULONG outBufLen = 16384;

    do {
        pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
        if (pAddresses == nullptr) return "";

        DWORD result = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, pAddresses, &outBufLen);
        if (result == ERROR_BUFFER_OVERFLOW) {
            free(pAddresses);
            continue;
        }
        if (result != NO_ERROR) {
            free(pAddresses);
            return "";
        }

        for (PIP_ADAPTER_ADDRESSES pCurr = pAddresses; pCurr != nullptr; pCurr = pCurr->Next) {
            if ((int)pCurr->IfIndex == interfaceIndex) {
                std::string name(pCurr->FriendlyName);
                free(pAddresses);
                return name;
            }
        }
        break;
    } while (false);

    if (pAddresses) free(pAddresses);
    return "";
}

std::vector<std::string> RouteTableManager::GetAllInterfaces() {
    std::vector<std::string> interfaces;

    PIP_ADAPTER_ADDRESSES pAddresses = nullptr;
    ULONG outBufLen = 16384;

    do {
        pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
        if (pAddresses == nullptr) return interfaces;

        DWORD result = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, pAddresses, &outBufLen);
        if (result == ERROR_BUFFER_OVERFLOW) {
            free(pAddresses);
            continue;
        }
        if (result != NO_ERROR) {
            free(pAddresses);
            return interfaces;
        }

        for (PIP_ADAPTER_ADDRESSES pCurr = pAddresses; pCurr != nullptr; pCurr = pCurr->Next) {
            interfaces.push_back(pCurr->FriendlyName);
        }
        break;
    } while (false);

    if (pAddresses) free(pAddresses);
    return interfaces;
}

} // namespace Core
} // namespace WVLAN
