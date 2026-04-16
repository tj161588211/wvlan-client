/**
 * @file WireGuardBridge.cpp
 * @brief WireGuard 驱动桥接层实现
 * 
 * 实现 WireGuardBridge 类的功能，封装 WireGuard-for-C API。
 */

#include "WireGuardBridge.h"
#include "../../Core/WireGuard/WireGuardManager.h"
#include <msclr/marshal_cppstd.h>

using namespace WVLAN::NativeBridge;
using namespace msclr::interop;

// ============================================================================
// WireGuardBridge 实现
// ============================================================================

WireGuardBridge::WireGuardBridge() 
    : _deviceHandle(nullptr), _interfaceName(nullptr), _isInitialized(false) {
}

WireGuardBridge::~WireGuardBridge() {
    if (_isInitialized) {
        ShutdownInterface();
    }
}

OperationStatus WireGuardBridge::InitializeInterface(String^ interfaceName, String^ privateKey, int listenPort) {
    if (interfaceName == nullptr || privateKey == nullptr) {
        return OperationStatus::InvalidParameter;
    }
    
    // 转换字符串
    std::string nativeInterfaceName = marshal_as<std::string>(interfaceName);
    std::string nativePrivateKey = marshal_as<std::string>(privateKey);
    
    // 调用原生 WireGuard 管理器
    WVLAN::Core::WireGuardManager wgManager;
    
    int result = wgManager.CreateInterface(
        nativeInterfaceName.c_str(),
        nativePrivateKey.c_str(),
        listenPort
    );
    
    if (result == 0) {
        _deviceHandle = wgManager.GetDeviceHandle();
        _interfaceName = interfaceName;
        _isInitialized = true;
        return OperationStatus::Success;
    }
    
    return static_cast<OperationStatus>(result);
}

OperationStatus WireGuardBridge::SetInterfaceAddress(String^ address) {
    if (!_isInitialized || _deviceHandle == nullptr) {
        return OperationStatus::InternalError;
    }
    
    if (address == nullptr) {
        return OperationStatus::InvalidParameter;
    }
    
    std::string nativeAddress = marshal_as<std::string>(address);
    
    WVLAN::Core::WireGuardManager wgManager;
    wgManager.SetDeviceHandle(_deviceHandle);
    
    int result = wgManager.SetInterfaceAddress(nativeAddress.c_str());
    return static_cast<OperationStatus>(result);
}

OperationStatus WireGuardBridge::AddPeer(String^ publicKey, String^ endpoint, String^ allowedIPs) {
    if (!_isInitialized || _deviceHandle == nullptr) {
        return OperationStatus::InternalError;
    }
    
    if (publicKey == nullptr || endpoint == nullptr || allowedIPs == nullptr) {
        return OperationStatus::InvalidParameter;
    }
    
    std::string nativePublicKey = marshal_as<std::string>(publicKey);
    std::string nativeEndpoint = marshal_as<std::string>(endpoint);
    std::string nativeAllowedIPs = marshal_as<std::string>(allowedIPs);
    
    WVLAN::Core::WireGuardManager wgManager;
    wgManager.SetDeviceHandle(_deviceHandle);
    
    int result = wgManager.AddPeer(
        nativePublicKey.c_str(),
        nativeEndpoint.c_str(),
        nativeAllowedIPs.c_str()
    );
    
    return static_cast<OperationStatus>(result);
}

OperationStatus WireGuardBridge::RemovePeer(String^ publicKey) {
    if (!_isInitialized || _deviceHandle == nullptr) {
        return OperationStatus::InternalError;
    }
    
    if (publicKey == nullptr) {
        return OperationStatus::InvalidParameter;
    }
    
    std::string nativePublicKey = marshal_as<std::string>(publicKey);
    
    WVLAN::Core::WireGuardManager wgManager;
    wgManager.SetDeviceHandle(_deviceHandle);
    
    int result = wgManager.RemovePeer(nativePublicKey.c_str());
    return static_cast<OperationStatus>(result);
}

OperationStatus WireGuardBridge::SetPersistentKeepalive(int seconds) {
    if (!_isInitialized || _deviceHandle == nullptr) {
        return OperationStatus::InternalError;
    }
    
    WVLAN::Core::WireGuardManager wgManager;
    wgManager.SetDeviceHandle(_deviceHandle);
    
    int result = wgManager.SetPersistentKeepalive(seconds);
    return static_cast<OperationStatus>(result);
}

OperationStatus WireGuardBridge::StartInterface() {
    if (!_isInitialized || _deviceHandle == nullptr) {
        return OperationStatus::InternalError;
    }
    
    WVLAN::Core::WireGuardManager wgManager;
    wgManager.SetDeviceHandle(_deviceHandle);
    
    int result = wgManager.StartInterface();
    return static_cast<OperationStatus>(result);
}

OperationStatus WireGuardBridge::StopInterface() {
    if (!_isInitialized || _deviceHandle == nullptr) {
        return OperationStatus::InternalError;
    }
    
    WVLAN::Core::WireGuardManager wgManager;
    wgManager.SetDeviceHandle(_deviceHandle);
    
    int result = wgManager.StopInterface();
    return static_cast<OperationStatus>(result);
}

OperationStatus WireGuardBridge::ShutdownInterface() {
    if (_deviceHandle != nullptr) {
        WVLAN::Core::WireGuardManager wgManager;
        wgManager.SetDeviceHandle(_deviceHandle);
        
        int result = wgManager.ShutdownInterface();
        _deviceHandle = nullptr;
        _isInitialized = false;
        
        return static_cast<OperationStatus>(result);
    }
    
    _isInitialized = false;
    return OperationStatus::Success;
}

WgInterfaceState WireGuardBridge::GetInterfaceState() {
    if (!_isInitialized || _deviceHandle == nullptr) {
        return WgInterfaceState::Down;
    }
    
    WVLAN::Core::WireGuardManager wgManager;
    wgManager.SetDeviceHandle(_deviceHandle);
    
    int state = wgManager.GetInterfaceState();
    
    switch (state) {
        case 1: return WgInterfaceState::Up;
        case 2: return WgInterfaceState::Connecting;
        case 3: return WgInterfaceState::Error;
        default: return WgInterfaceState::Down;
    }
}

WgInterfaceStats^ WireGuardBridge::GetInterfaceStats() {
    auto stats = gcnew WgInterfaceStats();
    
    if (!_isInitialized || _deviceHandle == nullptr) {
        return stats;
    }
    
    WVLAN::Core::WireGuardManager wgManager;
    wgManager.SetDeviceHandle(_deviceHandle);
    
    WVLAN::Core::InterfaceStatistics wgStats = wgManager.GetInterfaceStats();
    
    stats->TotalReceivedBytes = wgStats.ReceivedBytes;
    stats->TotalTransmittedBytes = wgStats.TransmittedBytes;
    stats->ActivePeers = wgStats.ActivePeers;
    stats->LastActivity = DateTime::Now; // 简化处理
    
    return stats;
}

IList<WgPeerStatus^>^ WireGuardBridge::GetPeersStatus() {
    auto peerList = gcnew List<WgPeerStatus^>();
    
    if (!_isInitialized || _deviceHandle == nullptr) {
        return peerList;
    }
    
    WVLAN::Core::WireGuardManager wgManager;
    wgManager.SetDeviceHandle(_deviceHandle);
    
    std::vector<WVLAN::Core::PeerStatistics> peers = wgManager.GetPeersStats();
    
    for (const auto& peer : peers) {
        auto peerStatus = gcnew WgPeerStatus();
        peerStatus->PublicKey = gcnew String(peer.PublicKey.c_str());
        peerStatus->Endpoint = gcnew String(peer.Endpoint.c_str());
        peerStatus->ReceivedBytes = peer.ReceivedBytes;
        peerStatus->TransmittedBytes = peer.TransmittedBytes;
        peerStatus->LastHandshake = DateTime::Now; // 简化处理
        peerStatus->PersistentKeepalive = peer.PersistentKeepalive;
        
        peerList->Add(peerStatus);
    }
    
    return peerList;
}

bool WireGuardBridge::IsDriverInstalled() {
    WVLAN::Core::WireGuardManager wgManager;
    return wgManager.IsDriverInstalled();
}

OperationStatus WireGuardBridge::InstallDriver() {
    WVLAN::Core::WireGuardManager wgManager;
    int result = wgManager.InstallDriver();
    return static_cast<OperationStatus>(result);
}

OperationStatus WireGuardBridge::UninstallDriver() {
    WVLAN::Core::WireGuardManager wgManager;
    int result = wgManager.UninstallDriver();
    return static_cast<OperationStatus>(result);
}

String^ WireGuardBridge::ConvertByteArrayToBase64(array<Byte>^ data) {
    if (data == nullptr) {
        return String::Empty;
    }
    
    cli::array<Byte>^ arr = data;
    return System::Convert::ToBase64String(arr);
}

array<Byte>^ WireGuardBridge::ConvertBase64ToByteArray(String^ base64) {
    if (base64 == nullptr || base64->Length == 0) {
        return nullptr;
    }
    
    return System::Convert::FromBase64String(base64);
}
