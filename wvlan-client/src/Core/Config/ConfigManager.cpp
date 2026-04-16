/**
 * @file ConfigManager.cpp
 * @brief 配置管理器实现
 */

#include "ConfigManager.h"
#include <fstream>
#include <filesystem>
#include <chrono>
#include <windows.h>

namespace fs = std::filesystem;

namespace WVLAN {
namespace Core {

static ConfigManager* g_instance = nullptr;

// 错误码定义
enum {
    ERROR_SUCCESS = 0,
    ERROR_INVALID_PARAM = -1,
    ERROR_NOT_FOUND = -2,
    ERROR_ALREADY_EXISTS = -3,
    ERROR_IO_FAILED = -4,
    ERROR_INTERNAL = -99
};

ConfigManager& ConfigManager::Instance() {
    if (g_instance == nullptr) {
        g_instance = new ConfigManager();
    }
    return *g_instance;
}

ConfigManager::ConfigManager() 
    : _storagePath(""), _activeProfile(""), _initialized(false) {
}

ConfigManager::~ConfigManager() {
    if (g_instance == this) {
        g_instance = nullptr;
    }
}

int ConfigManager::Initialize(const std::string& storagePath) {
    if (!storagePath.empty()) {
        _storagePath = storagePath;
    } else {
        // 默认存储路径：%APPDATA%\WVLAN\Profiles
        char appData[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_APPDATA, nullptr, 0, appData))) {
            _storagePath = std::string(appData) + "\\WVLAN\\Profiles";
        } else {
            _storagePath = "./wvlan_profiles";
        }
    }

    return CreateStorageDirectory();
}

int ConfigManager::CreateStorageDirectory() {
    try {
        if (!fs::exists(_storagePath)) {
            fs::create_directories(_storagePath);
        }
        _initialized = true;
        return ERROR_SUCCESS;
    } catch (const std::exception& e) {
        return ERROR_IO_FAILED;
    }
}

std::string ConfigManager::GetProfilePath(const std::string& profileName) {
    // 将配置文件名转换为安全的路径组件
    std::string safeName = profileName;
    for (char& c : safeName) {
        if (c == '/' || c == '\\' || c == ':' || c == '*' || 
            c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
            c = '_';
        }
    }
    
    return _storagePath + "\\" + safeName + ".conf";
}

int ConfigManager::SaveProfile(const NetworkConfig& config, const std::string& profileName) {
    if (!_initialized || profileName.empty()) {
        return ERROR_INVALID_PARAM;
    }

    std::string filePath = GetProfilePath(profileName);
    
    if (!ConfigParser::SaveToFile(config, filePath)) {
        return ERROR_IO_FAILED;
    }

    // 更新元数据（简化实现，实际应使用数据库）
    ProfileMetadata meta;
    meta.ProfileName = profileName;
    meta.FilePath = filePath;
    meta.CreatedAt = std::time(nullptr);
    meta.UpdatedAt = meta.CreatedAt;
    meta.IsActive = (_activeProfile == profileName);

    return ERROR_SUCCESS;
}

NetworkConfig ConfigManager::LoadProfile(const std::string& profileName) {
    NetworkConfig config;

    if (!_initialized || profileName.empty()) {
        return config;
    }

    std::string filePath = GetProfilePath(profileName);
    
    if (!fs::exists(filePath)) {
        return config;
    }

    ParseResult result = ConfigParser::ParseFromFile(filePath);
    if (result.Success) {
        config = result.Config;
    }

    return config;
}

int ConfigManager::DeleteProfile(const std::string& profileName) {
    if (!_initialized || profileName.empty()) {
        return ERROR_INVALID_PARAM;
    }

    std::string filePath = GetProfilePath(profileName);
    
    try {
        if (fs::exists(filePath)) {
            fs::remove(filePath);
        }
        
        if (_activeProfile == profileName) {
            _activeProfile = "";
        }

        return ERROR_SUCCESS;
    } catch (const std::exception&) {
        return ERROR_IO_FAILED;
    }
}

std::vector<ProfileMetadata> ConfigManager::GetAllProfiles() {
    std::vector<ProfileMetadata> profiles;

    if (!_initialized) {
        return profiles;
    }

    try {
        for (const auto& entry : fs::directory_iterator(_storagePath)) {
            if (entry.path().extension() == ".conf") {
                ProfileMetadata meta;
                meta.ProfileName = entry.path().stem().string();
                meta.FilePath = entry.path().string();
                meta.CreatedAt = std::filesystem::last_write_time(entry.path()).count();
                meta.IsActive = (_activeProfile == meta.ProfileName);
                
                profiles.push_back(meta);
            }
        }
    } catch (const std::exception&) {
        // 忽略目录读取错误
    }

    return profiles;
}

NetworkConfig ConfigManager::GetActiveProfile() {
    if (_activeProfile.empty()) {
        return NetworkConfig();
    }
    
    return LoadProfile(_activeProfile);
}

int ConfigManager::SetActiveProfile(const std::string& profileName) {
    if (!_initialized || profileName.empty()) {
        return ERROR_INVALID_PARAM;
    }

    std::string filePath = GetProfilePath(profileName);
    if (!fs::exists(filePath)) {
        return ERROR_NOT_FOUND;
    }

    _activeProfile = profileName;
    return ERROR_SUCCESS;
}

int ConfigManager::ImportFromFile(const std::string& filePath, const std::string& profileName) {
    if (filePath.empty() || profileName.empty()) {
        return ERROR_INVALID_PARAM;
    }

    ParseResult result = ConfigParser::ParseFromFile(filePath);
    if (!result.Success) {
        return ERROR_INVALID_PARAM;
    }

    return SaveProfile(result.Config, profileName);
}

int ConfigManager::ExportToFile(const std::string& profileName, const std::string& outputPath) {
    NetworkConfig config = LoadProfile(profileName);
    
    if (config.Peers.empty() && config.Interface.PrivateKey.empty()) {
        return ERROR_NOT_FOUND;
    }

    if (!ConfigParser::SaveToFile(config, outputPath)) {
        return ERROR_IO_FAILED;
    }

    return ERROR_SUCCESS;
}

std::string ConfigManager::GetStoragePath() const {
    return _storagePath;
}

} // namespace Core
} // namespace WVLAN
