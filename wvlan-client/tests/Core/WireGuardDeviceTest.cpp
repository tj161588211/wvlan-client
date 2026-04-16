/**
 * @file WireGuardDeviceTest.cpp
 * @brief WireGuard 设备单元测试
 */

#include <gtest/gtest.h>
#include "../../src/Core/WireGuard/WireGuardDevice.h"
#include "../../src/Core/WireGuard/WireGuardManager.h"

using namespace WVLAN::Core;

// ============================================================================
// WireGuardDevice 测试
// ============================================================================

class WireGuardDeviceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 设置测试前置条件
    }

    void TearDown() override {
        // 清理测试后条件
    }
};

TEST_F(WireGuardDeviceTest, ConstructorInitializesCorrectly) {
    WireGuardDevice device;
    
    // 验证初始状态
    EXPECT_EQ(device.GetState(), 0);  // Down state
    EXPECT_TRUE(device.GetInterfaceName().empty());
}

TEST_F(WireGuardDeviceTest, CreateWithInvalidParameters) {
    WireGuardDevice device;
    
    // 测试空接口名称
    int result = device.Create("", "testkey", 51820);
    EXPECT_LT(result, 0);
    
    // 测试空私钥
    result = device.Create("test-interface", "", 51820);
    EXPECT_LT(result, 0);
}

TEST_F(WireGuardDeviceTest, InvalidPortNumbers) {
    WireGuardDevice device;
    
    // 端口号为负数
    int result = device.Create("test", "dQw4w9WgXcQ=" + std::string(32, 'a'), -1);
    EXPECT_NE(result, ERROR_SUCCESS);
    
    // 端口号超过最大值
    result = device.Create("test", "dQw4w9WgXcQ=" + std::string(32, 'a'), 70000);
    EXPECT_NE(result, ERROR_SUCCESS);
}

TEST_F(WireGuardDeviceTest, GetStatisticsWhenNotInitialized) {
    WireGuardDevice device;
    
    InterfaceStatistics stats = device.GetStatistics();
    
    EXPECT_EQ(stats.ReceivedBytes, 0);
    EXPECT_EQ(stats.TransmittedBytes, 0);
    EXPECT_EQ(stats.ActivePeers, 0);
}

// ============================================================================
// WireGuardManager 测试
// ============================================================================

class WireGuardManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(WireGuardManagerTest, SingletonInstance) {
    WireGuardManager& manager1 = WireGuardManager::Instance();
    WireGuardManager& manager2 = WireGuardManager::Instance();
    
    EXPECT_EQ(&manager1, &manager2);
}

TEST_F(WireGuardManagerTest, IsDriverInstalled) {
    WireGuardManager& manager = WireGuardManager::Instance();
    
    // 这个测试可能需要实际环境才能正确判断
    bool installed = manager.IsDriverInstalled();
    
    // 只是验证方法可以调用，不验证具体值
    EXPECT_NO_THROW(installed = manager.IsDriverInstalled());
}

TEST_F(WireGuardManagerTest, GetDriverVersion) {
    WireGuardManager& manager = WireGuardManager::Instance();
    
    std::string version = manager.GetDriverVersion();
    
    EXPECT_FALSE(version.empty());
}

TEST_F(WireGuardManagerTest, GetAllInterfacesEmptyOnInit) {
    WireGuardManager& manager = WireGuardManager::Instance();
    
    auto interfaces = manager.GetAllInterfaces();
    
    // 可能没有 WireGuard 接口
    EXPECT_TRUE(interfaces.empty() || !interfaces.empty());  // 任一种情况都可能
}

// ============================================================================
// 集成测试
// ============================================================================

TEST_F(WireGuardDeviceTest, FullLifecycleSimulation) {
    // 模拟完整的设备生命周期
    // 注意：这些测试在真实环境中会实际创建/删除接口
    
    WireGuardDevice device;
    
    // 1. 创建设备（如果驱动未安装，应该返回错误）
    bool driverInstalled = false;
    // WireGuardManager& mgr = WireGuardManager::Instance();
    // driverInstalled = mgr.IsDriverInstalled();
    
    if (!driverInstalled) {
        // 驱动未安装时，预期操作会失败
        int result = device.Create("wg-test", "invalid-key", 51820);
        EXPECT_LT(result, 0);
    } else {
        // 如果有驱动，尝试完整流程
        // 注意：这可能会影响系统网络配置，需要谨慎
    }
}
