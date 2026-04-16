cmake_minimum_required(VERSION 3.20)
project(WVLANClient VERSION 1.0.0 LANGUAGES CXX CSharp)

# C++ 标准设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 输出目录配置
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 配置类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# ============================================================================
# WireGuard-for-C 子项目
# ============================================================================
option(WVLAN_BUILD_WIREGUARD "Build WireGuard-for-C from source" OFF)

if(WVLAN_BUILD_WIREGUARD)
    add_subdirectory(wireguard-c-src EXCLUDE_FROM_ALL)
    set(WIREGUARD_LIB wg)
else()
    # 使用预编译的 WireGuard 库
    find_library(WIREGUARD_LIB 
        NAMES libwg libwg-static wg
        PATHS 
            ${CMAKE_SOURCE_DIR}/lib
            ${CMAKE_SOURCE_DIR}/wireguard-c-src/build
            ${CMAKE_SOURCE_DIR}/wireguard-c-src/build/src
        NO_DEFAULT_PATH
    )
    
    if(NOT WIREGUARD_LIB)
        message(WARNING "WireGuard library not found. Set WVLAN_BUILD_WIREGUARD=ON or provide library path.")
    endif()
endif()

# ============================================================================
# 原生 C++ 核心库
# ============================================================================
add_library(wvlan_core STATIC
    src/Core/WireGuard/WireGuardDevice.cpp
    src/Core/WireGuard/WireGuardManager.cpp
    src/Core/Config/ConfigParser.cpp
    src/Core/Config/ConfigManager.cpp
    src/Core/Routing/RouteTableManager.cpp
    src/Core/Routing/RouteConfigurator.cpp
    src/Utils/NetworkUtils.cpp
    src/Utils/Logger.cpp
    src/Utils/SecurityUtils.cpp
)

target_include_directories(wvlan_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Utils
)

if(WIREGUARD_LIB)
    target_link_libraries(wvlan_core PRIVATE ${WIREGUARD_LIB})
    target_compile_definitions(wvlan_core PRIVATE HAVE_WIREGUARD)
endif()

# Windows 特定链接库
target_link_libraries(wvlan_core PRIVATE
    ws2_32
    iphlpapi
    advapi32
    crypt32
)

# ============================================================================
# C++/CLI桥接层
# ============================================================================
add_library(wvlan_bridge_cli clr)
set_target_properties(wvlan_bridge_cli PROPERTIES
    CLR_SUPPORT ON
    VS_DOTNET_TARGET_FRAMEWORK_VERSION "v6.0"
)

target_sources(wvlan_bridge_cli PRIVATE
    src/Bridge/CSharpCLI/NativeBridge.cpp
    src/Bridge/CSharpCLI/WireGuardBridge.cpp
    src/Bridge/CSharpCLI/ConfigBridge.cpp
    src/Bridge/CSharpCLI/RouteBridge.cpp
)

target_include_directories(wvlan_bridge_cli PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Bridge
)

target_link_libraries(wvlan_bridge_cli PRIVATE wlan_core)

# ============================================================================
# 单元测试
# ============================================================================
option(WVLAN_BUILD_TESTS "Build unit tests" ON)

if(WVLAN_BUILD_TESTS)
    # Google Test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    # 核心层测试 - WireGuard 模块
    add_executable(wvlan_wireguard_tests
        tests/main.cpp
        tests/Core/WireGuardDeviceTest.cpp
        tests/Core/WireGuardManagerTest.cpp
    )

    target_include_directories(wvlan_wireguard_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )

    target_link_libraries(wvlan_wireguard_tests PRIVATE
        wlan_core
        gtest
        gtest_main
    )

    add_test(NAME WireGuardTests COMMAND wlan_wireguard_tests)

    # 核心层测试 - 配置模块
    add_executable(wvlan_config_tests
        tests/main.cpp
        tests/Core/ConfigParserTest.cpp
    )

    target_include_directories(wvlan_config_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )

    target_link_libraries(wvlan_config_tests PRIVATE
        wlan_core
        gtest
        gtest_main
    )

    add_test(NAME ConfigTests COMMAND wlan_config_tests)

    # 核心层测试 - 路由模块
    add_executable(wvlan_routing_tests
        tests/main.cpp
        tests/Core/RouteTableManagerTest.cpp
    )

    target_include_directories(wvlan_routing_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )

    target_link_libraries(wvlan_routing_tests PRIVATE
        wlan_core
        gtest
        gtest_main
    )

    add_test(NAME RoutingTests COMMAND wlan_routing_tests)

    # 工具类测试 - 安全模块
    add_executable(wvlan_security_tests
        tests/main.cpp
        tests/Utils/SecurityUtilsTest.cpp
    )

    target_include_directories(wvlan_security_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )

    target_link_libraries(wvlan_security_tests PRIVATE
        wlan_core
        gtest
        gtest_main
    )

    add_test(NAME SecurityTests COMMAND wlan_security_tests)

    # 综合测试目标
    add_custom_target(run_all_tests
        COMMAND $<TARGET_FILE:wvlan_wireguard_tests>
        COMMAND $<TARGET_FILE:wvlan_config_tests>
        COMMAND $<TARGET_FILE:wvlan_routing_tests>
        COMMAND $<TARGET_FILE:wvlan_security_tests>
        DEPENDS wlan_wireguard_tests wlan_config_tests wlan_routing_tests wlan_security_tests
        COMMENT "Running all unit tests"
    )
endif()

# ============================================================================
# 安装配置
# ============================================================================
install(TARGETS wlan_core wlan_bridge_cli
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY src/
    DESTINATION include/wvlan
    FILES_MATCHING PATTERN "*.h"
)

# ============================================================================
# 自定义目标
# ============================================================================
add_custom_target(copy_dlls ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/lib/wvlan_core.dll
        ${CMAKE_BINARY_DIR}/bin/
    DEPENDS wlan_core
    COMMENT "Copying DLLs to output directory"
)

# 打印配置摘要
message(STATUS "===========================================")
message(STATUS "WVLAN Client Configuration Summary")
message(STATUS "===========================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "WireGuard source: ${WVLAN_BUILD_WIREGUARD}")
message(STATUS "Build tests: ${WVLAN_BUILD_TESTS}")
message(STATUS "===========================================")
