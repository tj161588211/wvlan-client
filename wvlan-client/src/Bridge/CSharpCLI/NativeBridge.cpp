/**
 * @file NativeBridge.cpp
 * @brief C++/CLI 桥接层实现 - 网络操作
 * 
 * 实现 NetworkOperations 类的功能，提供底层网络操作。
 */

#include "NativeBridge.h"
#include <winsock2.h>
#include <iphlpapi.h>
#include <windows.h>
#include <vector>
#include <string>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace WVLAN::NativeBridge;

// ============================================================================
// NetworkOperations 实现
// ============================================================================

cli::array<String^>^ NetworkOperations::GetLocalIPAddresses() {
    auto ipList = gcnew List<String^>();
    
    PIP_ADAPTER_ADDRESSES pAddresses = nullptr;
    ULONG outBufLen = 16384;
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    
    do {
        pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
        if (pAddresses == nullptr) {
            throw gcnew System::OutOfMemoryException("Failed to allocate memory for adapter addresses");
        }
        
        DWORD result = GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, pAddresses, &outBufLen);
        if (result == ERROR_BUFFER_OVERFLOW) {
            free(pAddresses);
            continue;
        }
        if (result != NO_ERROR) {
            free(pAddresses);
            throw gcnew System::Exception("Failed to get adapter addresses");
        }
        
        for (PIP_ADAPTER_ADDRESSES pCurr = pAddresses; pCurr != nullptr; pCurr = pCurr->Next) {
            if (pCurr->OperStatus != IfOperStatusUp) continue;
            
            for (IP_ADAPTER_UNICAST_ADDRESS* pAddr = pCurr->FirstUnicastAddress; 
                 pAddr != nullptr; pAddr = pAddr->Next) {
                
                sockaddr* sa = pAddr->Address.lpSockaddr;
                if (sa->sa_family == AF_INET) {
                    sockaddr_in* ipv4 = (sockaddr_in*)sa;
                    char ipStr[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &ipv4->sin_addr, ipStr, INET_ADDRSTRLEN);
                    ipList->Add(gcnew String(ipStr));
                } else if (sa->sa_family == AF_INET6) {
                    sockaddr_in6* ipv6 = (sockaddr_in6*)sa;
                    char ipStr[INET6_ADDRSTRLEN];
                    inet_ntop(AF_INET6, &ipv6->sin6_addr, ipStr, INET6_ADDRSTRLEN);
                    ipList->Add(gcnew String(ipStr));
                }
            }
        }
        break;
    } while (false);
    
    if (pAddresses) free(pAddresses);
    
    return ipList->ToArray();
}

NetworkType NetworkOperations::DetectNetworkType() {
    // 使用 Windows 网络位置感知功能
    GUID networkCategory;
    HRESULT hr = GetNetworkCategory(
        ::Guid::Empty, // 使用当前网络
        &networkCategory
    );
    
    if (FAILED(hr)) {
        return NetworkType::Unknown;
    }
    
    // 注意：GetNetworkCategory 需要特定接口 GUID
    // 这里简化处理，返回 PrivateNetwork 作为默认
    return NetworkType::PrivateNetwork;
}

bool NetworkOperations::IsRunAsAdmin() {
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

OperationStatus NetworkOperations::AddRoute(String^ destination, String^ gateway, int metric) {
    // 转换为原生字符串
    const char* dest = (const char*)(Marshal::StringToHGlobalAnsi(destination)).ToPointer();
    const char* gw = (const char*)(Marshal::StringToHGlobalAnsi(gateway)).ToPointer();
    
    OperationStatus result = AddRouteNative(dest, gw, metric);
    
    Marshal::FreeHGlobal(IntPtr((void*)dest));
    Marshal::FreeHGlobal(IntPtr((void*)gw));
    
    return result;
}

OperationStatus NetworkOperations::DeleteRoute(String^ destination) {
    const char* dest = (const char*)(Marshal::StringToHGlobalAnsi(destination)).ToPointer();
    OperationStatus result = DeleteRouteNative(dest);
    Marshal::FreeHGlobal(IntPtr((void*)dest));
    return result;
}

IList<NetworkInterfaceInfo^>^ NetworkOperations::GetNetworkInterfaces() {
    auto interfaceList = gcnew List<NetworkInterfaceInfo^>();
    
    PIP_ADAPTER_ADDRESSES pAddresses = nullptr;
    ULONG outBufLen = 16384;
    
    do {
        pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
        if (pAddresses == nullptr) {
            throw gcnew System::OutOfMemoryException();
        }
        
        DWORD result = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, pAddresses, &outBufLen);
        if (result == ERROR_BUFFER_OVERFLOW) {
            free(pAddresses);
            continue;
        }
        if (result != NO_ERROR) {
            free(pAddresses);
            throw gcnew System::Exception("Failed to get adapter addresses");
        }
        
        for (PIP_ADAPTER_ADDRESSES pCurr = pAddresses; pCurr != nullptr; pCurr = pCurr->Next) {
            String^ name = gcnew String(pCurr->FriendlyName);
            String^ desc = gcnew String(pCurr->Description);
            String^ ip = "N/A";
            String^ mac = "N/A";
            bool isWg = (pCurr->IfType == 245); // WireGuard interface type
            
            // 获取 IP 地址
            for (IP_ADAPTER_UNICAST_ADDRESS* pAddr = pCurr->FirstUnicastAddress; 
                 pAddr != nullptr; pAddr = pAddr->Next) {
                sockaddr* sa = pAddr->Address.lpSockaddr;
                if (sa->sa_family == AF_INET) {
                    sockaddr_in* ipv4 = (sockaddr_in*)sa;
                    char ipStr[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &ipv4->sin_addr, ipStr, INET_ADDRSTRLEN);
                    ip = gcnew String(ipStr);
                    break;
                }
            }
            
            // 获取 MAC 地址
            if (pCurr->PhysicalAddressLength > 0) {
                System::Text::StringBuilder^ sb = gcnew System::Text::StringBuilder();
                for (ULONG i = 0; i < pCurr->PhysicalAddressLength; i++) {
                    if (i > 0) sb->Append("-");
                    sb->Append(pCurr->PhysicalAddress[i].ToString("X2"));
                }
                mac = sb->ToString();
            }
            
            interfaceList->Add(gcnew NetworkInterfaceInfo(name, desc, ip, mac, isWg));
        }
        break;
    } while (false);
    
    if (pAddresses) free(pAddresses);
    
    return interfaceList;
}

// ============================================================================
// 原生实现
// ============================================================================

OperationStatus NetworkOperations::AddRouteNative(const char* destination, const char* gateway, int metric) {
    if (!destination || !gateway) {
        return OperationStatus::InvalidParameter;
    }
    
    if (!IsRunAsAdmin()) {
        return OperationStatus::AccessDenied;
    }
    
    // 构建 route add 命令
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "route add %s mask 255.255.255.255 %s metric %d", 
             destination, gateway, metric);
    
    // 使用 shell 执行命令（简化实现）
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    
    char* cmdCopy = _strdup(cmd);
    BOOL result = CreateProcessA(
        nullptr, cmdCopy,
        nullptr, nullptr, FALSE,
        CREATE_NO_WINDOW,
        nullptr, nullptr,
        &si, &pi
    );
    free(cmdCopy);
    
    if (!result) {
        return OperationStatus::InternalError;
    }
    
    WaitForSingleObject(pi.hProcess, 5000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    return OperationStatus::Success;
}

OperationStatus NetworkOperations::DeleteRouteNative(const char* destination) {
    if (!destination) {
        return OperationStatus::InvalidParameter;
    }
    
    if (!IsRunAsAdmin()) {
        return OperationStatus::AccessDenied;
    }
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "route delete %s", destination);
    
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    
    char* cmdCopy = _strdup(cmd);
    BOOL result = CreateProcessA(
        nullptr, cmdCopy,
        nullptr, nullptr, FALSE,
        CREATE_NO_WINDOW,
        nullptr, nullptr,
        &si, &pi
    );
    free(cmdCopy);
    
    if (!result) {
        return OperationStatus::InternalError;
    }
    
    WaitForSingleObject(pi.hProcess, 5000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    return OperationStatus::Success;
}
