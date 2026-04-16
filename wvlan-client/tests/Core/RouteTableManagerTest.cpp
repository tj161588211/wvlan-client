/**
 * @file RouteTableManagerTest.cpp
 * @brief 路由表管理器单元测试
 */

#include <gtest/gtest.h>
#include "../../src/Core/Routing/RouteTableManager.h"

using namespace WVLAN::Core;

// ============================================================================
// RouteTableManager 测试
// ============================================================================

class RouteTableManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        _manager = &RouteTableManager::Instance();
    }

    void TearDown() override {
    }

    RouteTableManager* _manager;
};

// 测试单例模式
TEST_F(RouteTableManagerTest, SingletonInstance) {
    RouteTableManager& manager1 = RouteTableManager::Instance();
    RouteTableManager& manager2 = RouteTableManager::Instance();
    
    EXPECT_EQ(&manager1, &manager2);
}

// 测试获取 IPv4 路由表
TEST_F(RouteTableManagerTest, GetIPv4RouteTable) {
    auto routes = _manager->GetIPv4RouteTable();
    
    // 应该至少有一条路由（默认路由）
    // 注意：在某些环境中可能返回空，这取决于系统配置
    SUCCEED();  // 只是验证方法可以调用而不崩溃
}

// 测试获取 IPv6 路由表
TEST_F(RouteTableManagerTest, GetIPv6RouteTable) {
    auto routes = _manager->GetIPv6RouteTable();
    
    // IPv6 可能未启用，所以不检查数量
    SUCCEED();
}

// 测试 CIDR 到子网掩码转换
TEST_F(RouteTableManagerTest, CIDRToNetmask) {
    EXPECT_EQ(RouteTableManager::CIDRToNetmask("192.168.1.0/24"), "255.255.255.0");
    EXPECT_EQ(RouteTableManager::CIDRToNetmask("10.0.0.0/8"), "255.0.0.0");
    EXPECT_EQ(RouteTableManager::CIDRToNetmask("172.16.0.0/12"), "255.240.0.0");
    EXPECT_EQ(RouteTableManager::CIDRToNetmask("0.0.0.0/0"), "0.0.0.0");
    EXPECT_EQ(RouteTableManager::CIDRToNetmask("192.168.1.1/32"), "255.255.255.255");
    
    // 没有斜杠的情况
    EXPECT_EQ(RouteTableManager::CIDRToNetmask("192.168.1.1"), "255.255.255.255");
}

// 测试获取 CIDR 掩码位数
TEST_F(RouteTableManagerTest, GetCIDRMask) {
    EXPECT_EQ(RouteTableManager::GetCIDRMask("192.168.1.0/24"), 24);
    EXPECT_EQ(RouteTableManager::GetCIDRMask("10.0.0.0/8"), 8);
    EXPECT_EQ(RouteTableManager::GetCIDRMask("172.16.0.0/16"), 16);
    EXPECT_EQ(RouteTableManager::GetCIDRMask("192.168.1.1"), 32);  // 无斜杠时默认为 32
}

// 测试获取接口名称
TEST_F(RouteTableManagerTest, GetInterfaceName) {
    // 获取所有接口
    auto interfaces = _manager->GetAllInterfaces();
    
    for (const auto& iface : interfaces) {
        int index = _manager->GetInterfaceIndex(iface);
        if (index > 0) {
            std::string name = _manager->GetInterfaceName(index);
            EXPECT_FALSE(name.empty());
        }
    }
}

// 测试管理员权限检测
TEST_F(RouteTableManagerTest, IsRunAsAdmin) {
    bool isAdmin = RouteTableManager::IsRunAsAdmin();
    
    // 只验证方法可以调用
    SUCCEED();
}

// 测试路由存在性检查
TEST_F(RouteTableManagerTest, RouteExists) {
    auto routes = _manager->GetIPv4RouteTable();
    
    // 检查默认路由是否存在
    for (const auto& route : routes) {
        if (route.Destination == "0.0.0.0") {
            bool exists = _manager->RouteExists("0.0.0.0");
            EXPECT_TRUE(exists);
            break;
        }
    }
}

// 测试获取到达目标的路由
TEST_F(RouteTableManagerTest, GetRouteTo) {
    RouteEntry route = _manager->GetRouteTo("8.8.8.8");
    
    // 应该能找到默认路由或具体路由
    SUCCEED();
}

// ============================================================================
// RouteResult 相关测试
// ============================================================================

TEST(RouteResultTest, DefaultConstructor) {
    RouteResult result;
    
    EXPECT_FALSE(result.Success);
    EXPECT_TRUE(result.ErrorMessage.empty());
    EXPECT_TRUE(result.AddedRoutes.empty());
    EXPECT_TRUE(result.RemovedRoutes.empty());
}

TEST(RouteResultTest, SuccessConstruction) {
    RouteResult result;
    result.Success = true;
    result.AddedRoutes.push_back("10.0.0.0/24");
    
    EXPECT_TRUE(result.Success);
    EXPECT_EQ(result.AddedRoutes.size(), 1);
}
