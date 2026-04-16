/**
 * @file NetworkUtils.cpp
 * @brief 网络工具类实现
 */

#include "NetworkUtils.h"
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <vector>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

namespace WVLAN {
namespace Core {

std::vector<std::string> NetworkUtils::GetLocalIPv4Addresses() {
    std::vector<std::string> addresses;
    
    PIP_ADAPTER_ADDRESSES pAddresses = nullptr;
    ULONG outBufLen = 16384;
    
    do {
        pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
        if (pAddresses == nullptr) break;
        
        DWORD result = GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, nullptr, pAddresses, &outBufLen);
        if (result == ERROR_BUFFER_OVERFLOW) {
            free(pAddresses);
            continue;
        }
        if (result != NO_ERROR) break;
        
        for (PIP_ADAPTER_ADDRESSES pCurr = pAddresses; pCurr != nullptr; pCurr = pCurr->Next) {
            if (pCurr->OperStatus != IfOperStatusUp) continue;
            
            for (IP_ADAPTER_UNICAST_ADDRESS* pAddr = pCurr->FirstUnicastAddress; 
                 pAddr != nullptr; pAddr = pAddr->Next) {
                
                sockaddr* sa = pAddr->Address.lpSockaddr;
                if (sa->sa_family == AF_INET) {
                    sockaddr_in* ipv4 = (sockaddr_in*)sa;
                    char ipStr[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &ipv4->sin_addr, ipStr, INET_ADDRSTRLEN);
                    addresses.push_back(ipStr);
                }
            }
        }
        break;
    } while (false);
    
    if (pAddresses) free(pAddresses);
    return addresses;
}

std::vector<std::string> NetworkUtils::GetLocalIPv6Addresses() {
    std::vector<std::string> addresses;
    
    PIP_ADAPTER_ADDRESSES pAddresses = nullptr;
    ULONG outBufLen = 16384;
    
    do {
        pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
        if (pAddresses == nullptr) break;
        
        DWORD result = GetAdaptersAddresses(AF_INET6, GAA_FLAG_INCLUDE_PREFIX, nullptr, pAddresses, &outBufLen);
        if (result == ERROR_BUFFER_OVERFLOW) {
            free(pAddresses);
            continue;
        }
        if (result != NO_ERROR) break;
        
        for (PIP_ADAPTER_ADDRESSES pCurr = pAddresses; pCurr != nullptr; pCurr = pCurr->Next) {
            if (pCurr->OperStatus != IfOperStatusUp) continue;
            
            for (IP_ADAPTER_UNICAST_ADDRESS* pAddr = pCurr->FirstUnicastAddress; 
                 pAddr != nullptr; pAddr = pAddr->Next) {
                
                sockaddr* sa = pAddr->Address.lpSockaddr;
                if (sa->sa_family == AF_INET6) {
                    sockaddr_in6* ipv6 = (sockaddr_in6*)sa;
                    char ipStr[INET6_ADDRSTRLEN];
                    inet_ntop(AF_INET6, &ipv6->sin6_addr, ipStr, INET6_ADDRSTRLEN);
                    addresses.push_back(ipStr);
                }
            }
        }
        break;
    } while (false);
    
    if (pAddresses) free(pAddresses);
    return addresses;
}

std::string NetworkUtils::GetDefaultGateway() {
    PIP_ADAPTER_ADDRESSES pAddresses = nullptr;
    ULONG outBufLen = 16384;
    std::string gateway;
    
    do {
        pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
        if (pAddresses == nullptr) break;
        
        DWORD result = GetAdaptersAddresses(AF_INET, 0, nullptr, pAddresses, &outBufLen);
        if (result == ERROR_BUFFER_OVERFLOW) {
            free(pAddresses);
            continue;
        }
        if (result != NO_ERROR) break;
        
        for (PIP_ADAPTER_ADDRESSES pCurr = pAddresses; pCurr != nullptr; pCurr = pCurr->Next) {
            if (pCurr->OperStatus != IfOperStatusUp) continue;
            
            IP_ADAPTER_GATEWAY_ADDRESS_L* pGateway = pCurr->FirstGatewayAddress;
            if (pGateway != nullptr && pGateway->Address.lpSockaddr != nullptr) {
                sockaddr* sa = pGateway->Address.lpSockaddr;
                if (sa->sa_family == AF_INET) {
                    sockaddr_in* ipv4 = (sockaddr_in*)sa;
                    char gwStr[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &ipv4->sin_addr, gwStr, INET_ADDRSTRLEN);
                    gateway = gwStr;
                    break;
                }
            }
        }
        break;
    } while (false);
    
    if (pAddresses) free(pAddresses);
    return gateway;
}

bool NetworkUtils::IsHostReachable(const std::string& host, int timeoutMs) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return false;
    }
    
    addrinfo hints = {}, *result = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(host.c_str(), nullptr, &hints, &result) != 0) {
        WSACleanup();
        return false;
    }
    
    SOCKET sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock == INVALID_SOCKET) {
        freeaddrinfo(result);
        WSACleanup();
        return false;
    }
    
    // 设置超时
    struct timeval tv;
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));
    
    bool reachable = (connect(sock, result->ai_addr, result->ai_addrlen) == 0);
    
    closesocket(sock);
    freeaddrinfo(result);
    WSACleanup();
    
    return reachable;
}

std::vector<std::string> NetworkUtils::ResolveHostname(const std::string& hostname) {
    std::vector<std::string> addresses;
    
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return addresses;
    }
    
    addrinfo hints = {}, *result = nullptr;
    hints.ai_family = AF_UNSPEC;
    
    if (getaddrinfo(hostname.c_str(), nullptr, &hints, &result) == 0) {
        for (addrinfo* res = result; res != nullptr; res = res->ai_next) {
            char ipStr[NI_MAXHOST];
            if (getnameinfo(res->ai_addr, res->ai_addrlen, ipStr, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST) == 0) {
                addresses.push_back(ipStr);
            }
        }
        freeaddrinfo(result);
    }
    
    WSACleanup();
    return addresses;
}

NetworkUtils::NetworkType NetworkUtils::DetectNetworkType() {
    // 简化实现：返回 PrivateNetwork
    return NetworkType::PrivateNetwork;
}

bool NetworkUtils::IsPortAccessible(const std::string& host, int port, int timeoutMs) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return false;
    }
    
    addrinfo hints = {}, *result = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    std::string portStr = std::to_string(port);
    
    if (getaddrinfo(host.c_str(), portStr.c_str(), &hints, &result) != 0) {
        WSACleanup();
        return false;
    }
    
    SOCKET sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock == INVALID_SOCKET) {
        freeaddrinfo(result);
        WSACleanup();
        return false;
    }
    
    struct timeval tv;
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));
    
    bool accessible = (connect(sock, result->ai_addr, result->ai_addrlen) == 0);
    
    closesocket(sock);
    freeaddrinfo(result);
    WSACleanup();
    
    return accessible;
}

} // namespace Core
} // namespace WVLAN
