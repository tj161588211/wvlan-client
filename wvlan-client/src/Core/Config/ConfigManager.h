/**
 * @file ConfigManager.h
 * @brief 配置管理器 - 配置持久化和生命周期管理
 * 
 * 本模块提供配置文件的持久化存储、加载、更新和删除功能。
 * 支持 SQLite 数据库存储和文件系统存储。
 * 
 * @version 1.0.0
 * @date 2026-04-15
 * @author Hermes Agent (Technical Director)
 */

#ifndef WVLAN_CONFIG_MANAGER_H
#define WVLAN_CONFIG_MANAGER_H

#include "ConfigParser.h"
#include <string>
#include <vector>
#include <memory>

namespace WVLAN {
namespace Core {

/// <summary>
/// 配置文件元数据
/// </summary>
struct ProfileMetadata {
    std::string ProfileName;
    std::string FilePath;
    std::string PublicKey;
    std::string Endpoint;
    int64_t CreatedAt;
    int64_t UpdatedAt;
    bool IsActive;

    ProfileMetadata() 
        : CreatedAt(0), UpdatedAt(0), IsActive(false) {}
};

/// <summary>
/// 配置管理器类
/// 
/// 提供配置文件的完整生命周期管理，包括：
/// - 配置文件的保存和加载
/// - 配置元数据管理
/// - 配置验证和转换
/// - 配置备份和恢复
/// </summary>
class ConfigManager {
public:
    /// <summary>
    /// 获取单例实例
    /// </summary>
    static ConfigManager& Instance();

    /// <summary>
    /// 析构函数
    /// </summary>
    ~ConfigManager();

    // 禁止拷贝
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    /// <summary>
    /// 初始化配置管理器
    /// </summary>
    /// <param name="storagePath">存储路径</param>
    /// <returns>0 表示成功</returns>
    int Initialize(const std::string& storagePath = "");

    /// <summary>
    /// 保存配置到存储
    /// </summary>
    /// <param name="config">网络配置</param>
    /// <param name="profileName">配置文件名称</param>
    /// <returns>0 表示成功</returns>
    int SaveProfile(const NetworkConfig& config, const std::string& profileName);

    /// <summary>
    /// 从存储加载配置
    /// </summary>
    /// <param name="profileName">配置文件名称</param>
    /// <returns>网络配置</returns>
    NetworkConfig LoadProfile(const std::string& profileName);

    /// <summary>
    /// 删除配置文件
    /// </summary>
    /// <param name="profileName">配置文件名称</param>
    /// <returns>0 表示成功</returns>
    int DeleteProfile(const std::string& profileName);

    /// <summary>
    /// 获取所有配置文件列表
    /// </summary>
    /// <returns>配置文件元数据列表</returns>
    std::vector<ProfileMetadata> GetAllProfiles();

    /// <summary>
    /// 获取当前激活的配置
    /// </summary>
    /// <returns>网络配置</returns>
    NetworkConfig GetActiveProfile();

    /// <summary>
    /// 设置激活的配置
    /// </summary>
    /// <param name="profileName">配置文件名称</param>
    /// <returns>0 表示成功</returns>
    int SetActiveProfile(const std::string& profileName);

    /// <summary>
    /// 从文件导入配置
    /// </summary>
    /// <param name="filePath">配置文件路径</param>
    /// <param name="profileName">配置文件名称</param>
    /// <returns>0 表示成功</returns>
    int ImportFromFile(const std::string& filePath, const std::string& profileName);

    /// <summary>
    /// 导出配置到文件
    /// </summary>
    /// <param name="profileName">配置文件名称</param>
    /// <param name="outputPath">输出文件路径</param>
    /// <returns>0 表示成功</returns>
    int ExportToFile(const std::string& profileName, const std::string& outputPath);

    /// <summary>
    /// 获取存储路径
    /// </summary>
    /// <returns>存储路径字符串</returns>
    std::string GetStoragePath() const;

private:
    /// <summary>
    /// 私有构造函数
    /// </summary>
    ConfigManager();

    /// <summary>
    /// 创建存储目录
    /// </summary>
    int CreateStorageDirectory();

    /// <summary>
    /// 获取配置文件路径
    /// </summary>
    std::string GetProfilePath(const std::string& profileName);

    // 成员变量
    std::string _storagePath;
    std::string _activeProfile;
    bool _initialized;
};

} // namespace Core
} // namespace WVLAN

#endif // WVLAN_CONFIG_MANAGER_H
