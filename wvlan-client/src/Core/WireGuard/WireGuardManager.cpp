/**
 * @file WireGuardManager.cpp
 * @brief WireGuard 管理器实现
 */

#include "WireGuardManager.h"
#include <stdexcept>
#include <windows.h>
#include <string>

// WireGuard-for-C API (假设已包含)
// #include <libwg.h>

namespace WVLAN {
namespace Core {

// 错误码定义
enum {
    ERROR_SUCCESS = 0,
    ERROR_INVALID_PARAM = -1,
    ERROR_ALREADY_EXISTS = -2,
    ERROR_NOT_FOUND = -3,
    ERROR_ACCESS_DENIED = -4,
    ERROR_NETWORK_FAILED = -5,
    ERROR_INTERNAL = -99
};

// 静态单例实例
static WireGuardManager* g_instance = nullptr;

WireGuardManager& WireGuardManager::Instance() {
    if (g_instance == nullptr) {
        g_instance = new WireGuardManager();
    }
    return *g_instance;
}

WireGuardManager::WireGuardManager() 
    : _currentDevice(nullptr), _libraryInitialized(false), _referenceCount(0) {
    InitializeWgLibrary();
}

WireGuardManager::~WireGuardManager() {
    if (_currentDevice != nullptr) {
        CloseDevice();
    }
    CleanupWgLibrary();
    
    if (g_instance == this) {
        g_instance = nullptr;
    }
}

int WireGuardManager::InitializeWgLibrary() {
    // 初始化 WireGuard-for-C 库
    // 这里调用 wg_init() 或类似函数
    _libraryInitialized = true;
    return ERROR_SUCCESS;
}

void WireGuardManager::CleanupWgLibrary() {
    if (_libraryInitialized) {
        // 调用 wg_exit() 或类似函数
        _libraryInitialized = false;
    }
}

bool WireGuardManager::IsDriverInstalled() const {
    // 检查 Windows 服务中是否存在 WireGuard 服务
    SC_HANDLE scManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (scManager == nullptr) {
        return false;
    }

    SC_HANDLE scService = OpenServiceA(scManager, "WireGuard", SERVICE_QUERY_STATUS);
    bool installed = (scService != nullptr);

    if (scService != nullptr) {
        CloseServiceHandle(scService);
    }
    CloseServiceHandle(scManager);

    return installed;
}

int WireGuardManager::InstallDriver() {
    if (!IsRunAsAdmin()) {
        return ERROR_ACCESS_DENIED;
    }

    // 调用 WireGuard 安装程序或使用 sc 命令安装驱动
    // 简化实现：返回成功
    return ERROR_SUCCESS;
}

int WireGuardManager::UninstallDriver() {
    if (!IsRunAsAdmin()) {
        return ERROR_ACCESS_DENIED;
    }

    // 停止并卸载 WireGuard 服务
    // 简化实现：返回成功
    return ERROR_SUCCESS;
}

std::string WireGuardManager::GetDriverVersion() const {
    // 获取驱动版本信息
    return "0.5.3"; // 示例版本
}

int WireGuardManager::CreateDevice(const char* interfaceName, const char* privateKey, int listenPort) {
    if (interfaceName == nullptr || privateKey == nullptr) {
        return ERROR_INVALID_PARAM;
    }

    // 创建新的 WireGuardDevice
    auto device = std::make_unique<WireGuardDevice>();
    int result = device->Create(interfaceName, privateKey, listenPort);

    if (result == ERROR_SUCCESS) {
        _currentDevice = device->GetHandle();
        // 注意：实际实现中需要管理设备生命周期
    }

    return result;
}

int WireGuardManager::OpenDevice(const char* interfaceName) {
    if (interfaceName == nullptr) {
        return ERROR_INVALID_PARAM;
    }

    auto device = std::make_unique<WireGuardDevice>();
    int result = device->Open(interfaceName);

    if (result == ERROR_SUCCESS) {
        _currentDevice = device->GetHandle();
    }

    return result;
}

int WireGuardManager::CloseDevice() {
    if (_currentDevice == nullptr) {
        return ERROR_SUCCESS;
    }

    auto device = std::make_unique<WireGuardDevice>();
    device->SetHandle(_currentDevice);
    int result = device->Close();

    _currentDevice = nullptr;
    return result;
}

int WireGuardManager::RemoveDevice() {
    if (_currentDevice == nullptr) {
        return ERROR_SUCCESS;
    }

    auto device = std::make_unique<WireGuardDevice>();
    device->SetHandle(_currentDevice);
    int result = device->Remove();

    _currentDevice = nullptr;
    return result;
}

void WireGuardManager::SetDeviceHandle(WgDeviceHandle* handle) {
    _currentDevice = handle;
}

WgDeviceHandle* WireGuardManager::GetDeviceHandle() const {
    return _currentDevice;
}

int WireGuardManager::SetInterfaceAddress(const char* address) {
    if (_currentDevice == nullptr || address == nullptr) {
        return ERROR_INVALID_PARAM;
    }

    auto device = std::make_unique<WireGuardDevice>();
    device->SetHandle(_currentDevice);
    return device->SetAddress(address);
}

int WireGuardManager::SetListenPort(int port) {
    if (_currentDevice == nullptr) {
        return ERROR_INVALID_PARAM;
    }

    auto device = std::make_unique<WireGuardDevice>();
    device->SetHandle(_currentDevice);
    return device->SetListenPort(port);
}

int WireGuardManager::AddPeer(const char* publicKey, const char* endpoint, const char* allowedIPs) {
    if (_currentDevice == nullptr || publicKey == nullptr) {
        return ERROR_INVALID_PARAM;
    }

    auto device = std::make_unique<WireGuardDevice>();
    device->SetHandle(_currentDevice);
    
    std::string ep = endpoint ? endpoint : "";
    std::string ips = allowedIPs ? allowedIPs : "";
    
    return device->AddPeer(publicKey, ep, ips);
}

int WireGuardManager::RemovePeer(const char* publicKey) {
    if (_currentDevice == nullptr || publicKey == nullptr) {
        return ERROR_INVALID_PARAM;
    }

    auto device = std::make_unique<WireGuardDevice>();
    device->SetHandle(_currentDevice);
    return device->RemovePeer(publicKey);
}

int WireGuardManager::SetPersistentKeepalive(int seconds) {
    if (_currentDevice == nullptr) {
        return ERROR_INVALID_PARAM;
    }

    auto device = std::make_unique<WireGuardDevice>();
    device->SetHandle(_currentDevice);
    return device->SetPersistentKeepalive(seconds);
}

int WireGuardManager::StartDevice() {
    if (_currentDevice == nullptr) {
        return ERROR_INVALID_PARAM;
    }

    auto device = std::make_unique<WireGuardDevice>();
    device->SetHandle(_currentDevice);
    return device->Start();
}

int WireGuardManager::StopDevice() {
    if (_currentDevice == nullptr) {
        return ERROR_SUCCESS;
    }

    auto device = std::make_unique<WireGuardDevice>();
    device->SetHandle(_currentDevice);
    return device->Stop();
}

int WireGuardManager::GetDeviceState() const {
    if (_currentDevice == nullptr) {
        return 0; // Down
    }

    auto device = std::make_unique<WireGuardDevice>();
    device->SetHandle(const_cast<WgDeviceHandle*>(_currentDevice));
    return device->GetState();
}

InterfaceStatistics WireGuardManager::GetDeviceStatistics() const {
    InterfaceStatistics stats = { 0, 0, 0, 0 };

    if (_currentDevice == nullptr) {
        return stats;
    }

    auto device = std::make_unique<WireGuardDevice>();
    device->SetHandle(const_cast<WgDeviceHandle*>(_currentDevice));
    return device->GetStatistics();
}

std::vector<PeerStatistics> WireGuardManager::GetPeersStatistics() const {
    std::vector<PeerStatistics> peers;

    if (_currentDevice == nullptr) {
        return peers;
    }

    auto device = std::make_unique<WireGuardDevice>();
    device->SetHandle(const_cast<WgDeviceHandle*>(_currentDevice));
    return device->GetPeersStatistics();
}

std::vector<std::string> WireGuardManager::GetAllInterfaces() const {
    std::vector<std::string> interfaces;
    
    // 从 Windows 网络接口列表中筛选 WireGuard 接口
    // 简化实现：返回空列表
    return interfaces;
}

// 辅助函数：检查管理员权限
static bool IsRunAsAdmin() {
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

} // namespace Core
} // namespace WVLAN
