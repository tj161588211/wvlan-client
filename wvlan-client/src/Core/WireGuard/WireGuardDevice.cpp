/**
 * @file WireGuardDevice.cpp
 * @brief WireGuard 设备封装类实现
 */

#include "WireGuardDevice.h"
#include "WireGuardManager.h"
#include <stdexcept>
#include <cstring>

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

WireGuardDevice::WireGuardDevice() 
    : _deviceHandle(nullptr), _interfaceName(""), _isCreated(false) {
}

WireGuardDevice::~WireGuardDevice() {
    if (_isCreated && _deviceHandle != nullptr) {
        Close();
    }
}

int WireGuardDevice::Create(const std::string& interfaceName, const std::string& privateKey, int listenPort) {
    if (interfaceName.empty() || privateKey.empty()) {
        return ERROR_INVALID_PARAM;
    }

    // 使用 WireGuardManager 创建设备
    WireGuardManager manager;
    
    int result = manager.CreateDevice(
        interfaceName.c_str(),
        privateKey.c_str(),
        listenPort
    );

    if (result == ERROR_SUCCESS) {
        _deviceHandle = manager.GetDeviceHandle();
        _interfaceName = interfaceName;
        _isCreated = true;
    }

    return result;
}

int WireGuardDevice::Open(const std::string& interfaceName) {
    if (interfaceName.empty()) {
        return ERROR_INVALID_PARAM;
    }

    WireGuardManager manager;
    
    int result = manager.OpenDevice(interfaceName.c_str());
    
    if (result == ERROR_SUCCESS) {
        _deviceHandle = manager.GetDeviceHandle();
        _interfaceName = interfaceName;
        _isCreated = true;
    }

    return result;
}

int WireGuardDevice::Close() {
    if (_deviceHandle == nullptr) {
        return ERROR_SUCCESS;
    }

    WireGuardManager manager;
    manager.SetDeviceHandle(_deviceHandle);
    
    int result = manager.CloseDevice();
    
    _deviceHandle = nullptr;
    _isCreated = false;

    return result;
}

int WireGuardDevice::Remove() {
    if (_deviceHandle == nullptr) {
        return ERROR_SUCCESS;
    }

    WireGuardManager manager;
    manager.SetDeviceHandle(_deviceHandle);
    
    int result = manager.RemoveDevice();
    
    _deviceHandle = nullptr;
    _isCreated = false;

    return result;
}

int WireGuardDevice::SetAddress(const std::string& address) {
    if (_deviceHandle == nullptr || address.empty()) {
        return ERROR_INVALID_PARAM;
    }

    WireGuardManager manager;
    manager.SetDeviceHandle(_deviceHandle);
    
    return manager.SetInterfaceAddress(address.c_str());
}

int WireGuardDevice::SetListenPort(int port) {
    if (_deviceHandle == nullptr || port < 0 || port > 65535) {
        return ERROR_INVALID_PARAM;
    }

    WireGuardManager manager;
    manager.SetDeviceHandle(_deviceHandle);
    
    return manager.SetListenPort(port);
}

int WireGuardDevice::AddPeer(const std::string& publicKey, const std::string& endpoint, const std::string& allowedIPs) {
    if (_deviceHandle == nullptr || publicKey.empty()) {
        return ERROR_INVALID_PARAM;
    }

    WireGuardManager manager;
    manager.SetDeviceHandle(_deviceHandle);
    
    return manager.AddPeer(publicKey.c_str(), endpoint.c_str(), allowedIPs.c_str());
}

int WireGuardDevice::RemovePeer(const std::string& publicKey) {
    if (_deviceHandle == nullptr || publicKey.empty()) {
        return ERROR_INVALID_PARAM;
    }

    WireGuardManager manager;
    manager.SetDeviceHandle(_deviceHandle);
    
    return manager.RemovePeer(publicKey.c_str());
}

int WireGuardDevice::SetPersistentKeepalive(int seconds) {
    if (_deviceHandle == nullptr || seconds < 0) {
        return ERROR_INVALID_PARAM;
    }

    WireGuardManager manager;
    manager.SetDeviceHandle(_deviceHandle);
    
    return manager.SetPersistentKeepalive(seconds);
}

int WireGuardDevice::Start() {
    if (_deviceHandle == nullptr) {
        return ERROR_INVALID_PARAM;
    }

    WireGuardManager manager;
    manager.SetDeviceHandle(_deviceHandle);
    
    return manager.StartDevice();
}

int WireGuardDevice::Stop() {
    if (_deviceHandle == nullptr) {
        return ERROR_SUCCESS;
    }

    WireGuardManager manager;
    manager.SetDeviceHandle(_deviceHandle);
    
    return manager.StopDevice();
}

int WireGuardDevice::GetState() const {
    if (_deviceHandle == nullptr) {
        return 0; // Down
    }

    WireGuardManager manager;
    manager.SetDeviceHandle(const_cast<WgDeviceHandle*>(_deviceHandle));
    
    return manager.GetDeviceState();
}

InterfaceStatistics WireGuardDevice::GetStatistics() const {
    InterfaceStatistics stats = { 0, 0, 0, 0 };

    if (_deviceHandle == nullptr) {
        return stats;
    }

    WireGuardManager manager;
    manager.SetDeviceHandle(const_cast<WgDeviceHandle*>(_deviceHandle));
    
    return manager.GetDeviceStatistics();
}

std::vector<PeerStatistics> WireGuardDevice::GetPeersStatistics() const {
    std::vector<PeerStatistics> peers;

    if (_deviceHandle == nullptr) {
        return peers;
    }

    WireGuardManager manager;
    manager.SetDeviceHandle(const_cast<WgDeviceHandle*>(_deviceHandle));
    
    return manager.GetPeersStatistics();
}

std::string WireGuardDevice::GetInterfaceName() const {
    return _interfaceName;
}

WgDeviceHandle* WireGuardDevice::GetHandle() const {
    return _deviceHandle;
}

} // namespace Core
} // namespace WVLAN
