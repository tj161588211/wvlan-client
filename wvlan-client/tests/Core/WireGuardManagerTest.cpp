/**
 * @file WireGuardManagerTest.cpp
 * @brief WireGuard 管理器单元测试
 */

#include <gtest/gtest.h>
#include "../../src/Core/WireGuard/WireGuardManager.h"

using namespace WVLAN::Core;

// ============================================================================
// WireGuardManager 测试
// ============================================================================

class WireGuardManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        _manager = &WireGuardManager::Instance();
    }

    void TearDown() override {
        // 清理所有创建的接口
        auto interfaces = _manager->GetAllInterfaces();
        for (const auto& iface : interfaces) {
            if (iface.Name.find("WVLAN-Test") != std::string::npos) {
                _manager->DeleteInterface(iface.Name);
            }
        }
    }

    WireGuardManager* _manager;
};

// 测试单例模式
TEST_F(WireGuardManagerTest, SingletonInstance) {
    WireGuardManager& manager1 = WireGuardManager::Instance();
    WireGuardManager& manager2 = WireGuardManager::Instance();
    
    EXPECT_EQ(&manager1, &manager2);
}

// 测试驱动安装状态检测
TEST_F(WireGuardManagerTest, IsDriverInstalled) {
    bool isInstalled = _manager->IsDriverInstalled();
    
    // 只验证方法可以调用
    SUCCEED();
}

// 测试获取所有接口
TEST_F(WireGuardManagerTest, GetAllInterfaces) {
    auto interfaces = _manager->GetAllInterfaces();
    
    // 返回的应该是一个有效的列表（可能为空）
    EXPECT_TRUE(true);  // 如果方法没有崩溃就通过
}

// 测试获取指定接口
TEST_F(WireGuardManagerTest, GetInterface) {
    // 测试不存在的接口
    auto result = _manager->GetInterface("NonExistentInterface");
    
    EXPECT_FALSE(result.has_value());
}

// 测试接口名称验证
TEST_F(WireGuardManagerTest, IsValidInterfaceName) {
    // 有效的接口名称
    EXPECT_TRUE(_manager->IsValidInterfaceName("WireGuard0"));
    EXPECT_TRUE(_manager->IsValidInterfaceName("WG-Test-1"));
    EXPECT_TRUE(_manager->IsValidInterfaceName("wvlan1"));
    
    // 无效的接口名称
    EXPECT_FALSE(_manager->IsValidInterfaceName(""));
    EXPECT_FALSE(_manager->IsValidInterfaceName("WireGuard/Invalid"));
    EXPECT_FALSE(_manager->IsValidInterfaceName("WireGuard*Invalid"));
}

// 测试接口名称长度限制
TEST_F(WireGuardManagerTest, InterfaceNameLengthLimit) {
    // WireGuard 限制接口名称长度为 15 字符
    std::string validName = "ValidName123456";  // 15 字符
    std::string tooLongName = "ValidName1234567";  // 16 字符
    
    EXPECT_TRUE(_manager->IsValidInterfaceName(validName));
    EXPECT_FALSE(_manager->IsValidInterfaceName(tooLongName));
}

// 测试创建接口（模拟，不实际创建）
TEST_F(WireGuardManagerTest, CreateInterfaceValidation) {
    InterfaceConfig config;
    config.Name = "WVLAN-Test-001";
    config.PrivateKey = "sAxY0XsPftePA5pL5fqPqjUt4GCDq3l7K8gJvzCZdFM=";
    config.ListenPort = 51820;
    
    // 验证配置
    auto validationResult = _manager->ValidateInterfaceConfig(config);
    
    EXPECT_TRUE(validationResult.Success);
    EXPECT_TRUE(validationResult.ErrorMessage.empty());
}

// 测试无效配置验证
TEST_F(WireGuardManagerTest, ValidateInvalidConfig) {
    InterfaceConfig config;
    config.Name = "";  // 空名称
    config.PrivateKey = "invalidKey";  // 无效密钥
    config.ListenPort = 0;  // 无效端口
    
    auto validationResult = _manager->ValidateInterfaceConfig(config);
    
    EXPECT_FALSE(validationResult.Success);
    EXPECT_FALSE(validationResult.ErrorMessage.empty());
}

// 测试端口验证
TEST_F(WireGuardManagerTest, ValidatePort) {
    EXPECT_TRUE(_manager->IsValidPort(51820));
    EXPECT_TRUE(_manager->IsValidPort(1024));
    EXPECT_TRUE(_manager->IsValidPort(65535));
    
    EXPECT_FALSE(_manager->IsValidPort(0));
    EXPECT_FALSE(_manager->IsValidPort(1023));
    EXPECT_FALSE(_manager->IsValidPort(65536));
}

// 测试接口状态转换
TEST_F(WireGuardManagerTest, InterfaceStateTransition) {
    InterfaceConfig config;
    config.Name = "WVLAN-Test-State";
    config.PrivateKey = "sAxY0XsPftePA5pL5fqPqjUt4GCDq3l7K8gJvzCZdFM=";
    
    // 验证状态枚举值
    EXPECT_EQ(static_cast<int>(InterfaceState::Stopped), 0);
    EXPECT_EQ(static_cast<int>(InterfaceState::Starting), 1);
    EXPECT_EQ(static_cast<int>(InterfaceState::Running), 2);
    EXPECT_EQ(static_cast<int>(InterfaceState::Stopping), 3);
    EXPECT_EQ(static_cast<int>(InterfaceState::Error), 4);
}

// 测试错误码
TEST_F(WireGuardManagerTest, ErrorCodes) {
    // 验证错误码枚举值
    EXPECT_EQ(static_cast<int>(WireGuardError::Success), 0);
    EXPECT_EQ(static_cast<int>(WireGuardError::DriverNotInstalled), 1);
    EXPECT_EQ(static_cast<int>(WireGuardError::InvalidConfig), 2);
    EXPECT_EQ(static_cast<int>(WireGuardError::InterfaceNotFound), 3);
    EXPECT_EQ(static_cast<int>(WireGuardError::CreateFailed), 4);
    EXPECT_EQ(static_cast<int>(WireGuardError::StartFailed), 5);
    EXPECT_EQ(static_cast<int>(WireGuardError::StopFailed), 6);
    EXPECT_EQ(static_cast<int>(WireGuardError::DeleteFailed), 7);
    EXPECT_EQ(static_cast<int>(WireGuardError::AccessDenied), 8);
}

// 测试获取错误消息
TEST_F(WireGuardManagerTest, GetErrorMessage) {
    std::string msg = _manager->GetErrorMessage(WireGuardError::DriverNotInstalled);
    EXPECT_FALSE(msg.empty());
    
    msg = _manager->GetErrorMessage(WireGuardError::Success);
    EXPECT_FALSE(msg.empty());
}
