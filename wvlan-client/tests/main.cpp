/**
 * @file main.cpp
 * @brief Google Test 测试入口
 */

#include <gtest/gtest.h>
#include <iostream>

// 声明外部测试套件（避免链接错误）
// WireGuardDeviceTest 在 WireGuardDeviceTest.cpp 中定义
// ConfigParserTest 在 ConfigParserTest.cpp 中定义
// RouteTableManagerTest 在 RouteTableManagerTest.cpp 中定义
// SecurityUtilsTest 在 SecurityUtilsTest.cpp 中定义
// WireGuardManagerTest 在 WireGuardManagerTest.cpp 中定义

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    // 设置输出
    ::testing::TestEventListeners& listeners = 
        ::testing::UnitTest::GetInstance()->listeners();
    
    // 打印测试总结
    auto result = RUN_ALL_TESTS();
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "WVLAN WireGuard 框架单元测试完成" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "测试结果：" << (result == 0 ? "全部通过" : "存在失败") << std::endl;
    std::cout << "========================================" << std::endl;
    
    return result;
}
