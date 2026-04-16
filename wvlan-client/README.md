# WVLAN WireGuard 框架

## 项目概述

WVLAN (Windows Virtual LAN) 是一个基于 WireGuard 驱动的虚拟局域网客户端应用。本项目提供了完整的 WireGuard 驱动集成框架，用于在 Windows 平台上创建和管理安全的点对点虚拟网络隧道。

### 核心功能

- ✅ **WireGuard 驱动集成** - 与 WireGuard-for-C 库无缝集成
- ✅ **接口管理** - 创建、启动、停止、删除 WireGuard 接口
- ✅ **配置解析** - 支持标准 WireGuard .conf 配置文件格式
- ✅ **路由配置** - 自动配置 Windows 路由表
- ✅ **密钥管理** - 安全的密钥生成和存储
- ✅ **C++/CLI 桥接** - 为 WPF/C# GUI 提供原生代码访问
- ✅ **单元测试** - 完整的测试覆盖率

## 架构设计

```
┌─────────────────────────────────────────────────────────────┐
│                      WPF GUI Layer                          │
├─────────────────────────────────────────────────────────────┤
│              C++/CLI Bridge Layer                           │
│  ┌──────────────┬──────────────┬──────────────┬──────────┐ │
│  │NativeBridge  │WireGuardBridge│ ConfigBridge│RouteBridge│ │
│  └──────────────┴──────────────┴──────────────┴──────────┘ │
├─────────────────────────────────────────────────────────────┤
│                   Core Logic Layer                          │
│  ┌──────────────┬──────────────┬──────────────┬──────────┐ │
│  │WireGuardMgr  │ ConfigManager│ RouteManager │SecurityUtils││
│  └──────────────┴──────────────┴──────────────┴──────────┘ │
├─────────────────────────────────────────────────────────────┤
│                   Utility Layer                             │
│  ┌──────────────┬──────────────┬──────────────────────────┐ │
│  │ NetworkUtils │   Logger     │    SecurityUtils         │ │
│  └──────────────┴──────────────┴──────────────────────────┘ │
├─────────────────────────────────────────────────────────────┤
│              External Dependencies                          │
│  ┌──────────────┬─────────────────────────────────────────┐ │
│  │ WireGuard-C  │       Windows SDK APIs                  │ │
│  └──────────────┴─────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

## 快速开始

### 前置要求

- Visual Studio 2022 (17.0+)
- CMake 3.20+
- Windows SDK 10.0.19041.0+
- .NET 6.0 或 7.0

### 克隆项目

```bash
git clone <repository-url>
cd wvlan-client
```

### 获取依赖

```bash
# 获取 WireGuard-for-C 库
mkdir -p lib
git clone https://git.zx2c4.com/WireGuard-for-C lib/wireguard-c-src
```

### 构建项目

```bash
# 创建构建目录
mkdir build && cd build

# 配置 CMake
cmake .. -G "Visual Studio 17 2022" -A x64

# 编译
cmake --build . --config Release
```

### 运行测试

```bash
# 运行所有测试
ctest -C Release --verbose

# 或单独运行测试套件
./bin/wvlan_wireguard_tests.exe
./bin/wvlan_config_tests.exe
./bin/wvlan_routing_tests.exe
./bin/wvlan_security_tests.exe
```

## 目录结构

```
wvlan-client/
├── src/
│   ├── Bridge/                 # C++/CLI 桥接层
│   │   └── CSharpCLI/
│   │       ├── NativeBridge.*      # Windows API 桥接
│   │       ├── WireGuardBridge.*   # WireGuard API 封装
│   │       ├── ConfigBridge.*      # 配置桥接
│   │       └── RouteBridge.*       # 路由桥接
│   ├── Core/                   # 核心业务逻辑
│   │   ├── WireGuard/
│   │   │   ├── WireGuardDevice.*   # 设备类
│   │   │   └── WireGuardManager.*  # 管理器
│   │   ├── Config/
│   │   │   ├── ConfigParser.*      # 配置解析器
│   │   │   └── ConfigManager.*     # 配置管理器
│   │   └── Routing/
│   │       ├── RouteTableManager.* # 路由表管理器
│   │       └── RouteConfigurator.* # 路由配置器
│   └── Utils/                  # 工具类
│       ├── NetworkUtils.*        # 网络工具
│       ├── Logger.*              # 日志系统
│       └── SecurityUtils.*       # 安全工具
├── tests/                    # 单元测试
│   ├── Core/
│   ├── Utils/
│   └── main.cpp
├── docs/                     # 文档
├── scripts/                  # 构建脚本
├── lib/                      # 第三方库
├── CMakeLists.txt            # CMake 配置
├── dependencies.md           # 依赖说明
└── README.md                 # 本文件
```

## 使用示例

### C++ 原生代码

```cpp
#include "Core/WireGuard/WireGuardManager.h"
#include "Core/Config/ConfigParser.h"

using namespace WVLAN::Core;

// 1. 检查驱动状态
auto& manager = WireGuardManager::Instance();
if (!manager.IsDriverInstalled()) {
    // 处理驱动未安装情况
    return false;
}

// 2. 解析配置文件
auto result = ConfigParser::ParseFromFile("config.conf");
if (!result.Success) {
    // 处理解析错误
    return false;
}

// 3. 创建并启动接口
InterfaceConfig config = result.Config.Interface;
auto createResult = manager.CreateInterface(config);
if (createResult.Success) {
    auto& device = manager.GetInterface(config.Name).value();
    device.Start();
}
```

### C# / WPF 代码

```csharp
using WVLAN.Bridge;

public class VpnController
{
    private readonly WireGuardBridge _bridge;
    
    public async Task<bool> ConnectAsync(string configPath)
    {
        // 导入配置
        var configBridge = new ConfigBridge();
        bool imported = await Task.Run(() => 
            configBridge.ImportConfiguration(configPath, "MyVPN"));
        
        if (!imported) return false;
        
        // 创建并启动接口
        var wireGuardBridge = new WireGuardBridge();
        return await Task.Run(() => 
            wireGuardBridge.CreateInstance("MyVPN"));
    }
}
```

## 主要组件

### WireGuardDevice

表示单个 WireGuard 接口，提供生命周期管理：
- `Create()` - 创建设备
- `Start()` - 启动设备
- `Stop()` - 停止设备
- `Delete()` - 删除设备
- `GetState()` - 获取设备状态

### WireGuardManager

单例模式的管理器，负责：
- 驱动安装状态检测
- 接口创建/删除
- 接口状态查询
- 错误处理

### ConfigParser

解析 WireGuard 配置文件：
- 支持 [Interface] 和 [Peer] 段
- 验证密钥格式
- 导出配置为标准格式

### RouteTableManager

Windows 路由表管理：
- IPv4/IPv6路由操作
- 路由存在性检查
- 默认网关检测

## 测试覆盖

| 模块 | 测试文件 | 覆盖率 |
|------|---------|--------|
| WireGuardDevice | WireGuardDeviceTest.cpp | ~80% |
| WireGuardManager | WireGuardManagerTest.cpp | ~75% |
| ConfigParser | ConfigParserTest.cpp | ~85% |
| RouteTableManager | RouteTableManagerTest.cpp | ~70% |
| SecurityUtils | SecurityUtilsTest.cpp | ~90% |

运行测试:
```bash
# CTest
ctest -C Release

# 或执行单个测试可执行文件
./tests/Release/wvlan_security_tests.exe
```

## 配置选项

### CMake 变量

| 变量 | 默认 | 说明 |
|------|------|------|
| `WVLAN_BUILD_WIREGUARD` | OFF | 从源码构建 WireGuard |
| `WVLAN_BUILD_TESTS` | ON | 构建单元测试 |
| `CMAKE_BUILD_TYPE` | Release | 构建类型 |

### 环境变量

| 变量 | 说明 |
|------|------|
| `WVLAN_LOG_LEVEL` | 日志级别 (Debug/Info/Warning/Error) |
| `WVLAN_CONFIG_DIR` | 配置文件存储目录 |

## 安全考虑

- 私钥应使用 Windows DPAPI 加密存储
- 所有敏感数据使用后应清零
- 需要管理员权限才能操作路由表和接口
- 建议使用代码签名确保驱动完整性

## 故障排查

### 常见问题

**Q: 驱动未安装**
- 运行 WireGuard 安装包
- 或以管理员身份运行程序

**Q: 无法创建接口**
- 检查是否以管理员身份运行
- 确认 WireGuard 驱动已正确安装

**Q: 路由配置失败**
- 检查管理员权限
- 验证接口名称有效性

## 贡献指南

1. Fork 项目
2. 创建特性分支 (`git checkout -b feature/amazing-feature`)
3. 提交更改 (`git commit -m 'Add amazing feature'`)
4. 推送到分支 (`git push origin feature/amazing-feature`)
5. 开启 Pull Request

### 代码风格

- 头文件使用 `.h` 扩展名
- 命名空间 `WVLAN::Core`, `WVLAN::Utils`, `WVLAN::Bridge`
- 类名使用 PascalCase
- 注释使用中文

## 许可证

本项目采用 MIT 许可证。详见 [LICENSE](LICENSE) 文件。

## 联系方式

- 项目主页: https://github.com/wvlan/wvlan-client
- 问题反馈: https://github.com/wvlan/wvlan-client/issues
- 开发团队：WVLAN Team

---

**版本**: 1.0.0  
**最后更新**: 2026-04-15  
**文档**: [使用说明](docs/USAGE.md) | [依赖说明](dependencies.md) | [架构文档](docs/architecture/)
