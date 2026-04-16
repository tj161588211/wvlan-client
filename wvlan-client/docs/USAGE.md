# WVLAN WireGuard 框架 - 使用说明

## 项目概述

WVLAN (Windows Virtual LAN) 是一个基于 WireGuard 驱动的虚拟局域网客户端应用。本项目提供了完整的 WireGuard 驱动集成框架，包括驱动管理、接口配置、路由设置等功能模块。

## 目录结构

```
wvlan-client/
├── src/
│   ├── Bridge/              # C++/CLI 桥接层
│   │   └── CSharpCLI/       # .NET 互操作接口
│   │       ├── NativeBridge.*        # Windows API 桥接
│   │       ├── WireGuardBridge.*     # WireGuard API 桥接
│   │       ├── ConfigBridge.*        # 配置桥接
│   │       └── RouteBridge.*         # 路由桥接
│   ├── Core/                # 核心功能模块
│   │   ├── WireGuard/       # WireGuard 驱动集成
│   │   │   ├── WireGuardDevice.*    # 设备管理
│   │   │   └── WireGuardManager.*   # 管理器
│   │   ├── Config/          # 配置管理
│   │   │   ├── ConfigParser.*       # 配置文件解析
│   │   │   └── ConfigManager.*      # 配置存储管理
│   │   └── Routing/         # 路由管理
│   │       ├── RouteTableManager.*  # 路由表管理
│   │       └── RouteConfigurator.*  # 路由配置器
│   └── Utils/               # 工具类
│       ├── NetworkUtils.*   # 网络工具
│       ├── Logger.*         # 日志系统
│       └── SecurityUtils.*  # 安全工具
├── tests/                   # 单元测试
│   ├── Core/
│   │   ├── WireGuardDeviceTest.cpp
│   │   ├── WireGuardManagerTest.cpp
│   │   └── ConfigParserTest.cpp
│   ├── Utils/
│   │   └── SecurityUtilsTest.cpp
│   └── main.cpp
├── lib/                     # 第三方库
│   └── wireguard-c-src/     # WireGuard-for-C 源码
├── scripts/                 # 构建脚本
│   └── build.ps1
├── docs/                    # 文档
├── CMakeLists.txt           # CMake 构建配置
├── dependencies.md          # 依赖说明
└── README.md                # 项目说明
```

## 快速开始

### 1. 环境准备

#### Windows 开发环境
```powershell
# 1. 安装 Visual Studio 2022
# 下载: https://visualstudio.microsoft.com/downloads/
# 选择工作负载:"使用 C++ 的桌面开发"和".NET 桌面开发"

# 2. 安装 CMake
winget install Kitware.CMake

# 3. 克隆项目
git clone <repository-url>
cd wvlan-client
```

### 2. 获取 WireGuard-for-C 库

```bash
# 创建 lib 目录
mkdir -p lib

# 克隆 WireGuard-for-C
git clone https://git.zx2c4.com/WireGuard-for-C lib/wireguard-c-src

# 或使用特定版本
cd lib/wireguard-c-src
git checkout 0.0.20230826
```

### 3. 编译项目

```powershell
# 使用 Visual Studio Developer Command Prompt
cd wvlan-client

# 创建构建目录
mkdir build
cd build

# 配置 CMake
cmake .. -G "Visual Studio 17 2022" -A x64

# 编译
cmake --build . --config Release
```

### 4. 运行测试

```powershell
# 运行单元测试
.\tests\Release\WVLAN.Tests.exe
```

## 核心 API 使用

### WireGuard 接口管理

#### C++ 原生代码示例
```cpp
#include "Core/WireGuard/WireGuardManager.h"
#include "Core/WireGuard/WireGuardDevice.h"

using namespace WVLAN::Core;

// 1. 获取管理器实例
auto& manager = WireGuardManager::Instance();

// 2. 检查驱动是否安装
if (!manager.IsDriverInstalled()) {
    // 提示用户安装驱动
    return false;
}

// 3. 创建接口配置
InterfaceConfig config;
config.Name = "WVLAN0";
config.PrivateKey = "sAxY0XsPftePA5pL5fqPqjUt4GCDq3l7K8gJvzCZdFM=";
config.ListenPort = 51820;
config.Addresses = { "10.10.0.2/24" };

// 4. 验证配置
auto validationResult = manager.ValidateInterfaceConfig(config);
if (!validationResult.Success) {
    std::cerr << "配置无效：" << validationResult.ErrorMessage << std::endl;
    return false;
}

// 5. 创建接口
auto result = manager.CreateInterface(config);
if (!result.Success) {
    std::cerr << "创建接口失败：" << result.ErrorMessage << std::endl;
    return false;
}

// 6. 启动接口
auto& device = manager.GetInterface("WVLAN0").value();
device.Start();
```

#### C# / WPF 代码示例
```csharp
using WVLAN.Bridge;

namespace WVLAN.Client
{
    public class WireGuardController
    {
        private readonly WireGuardBridge _wireGuardBridge;
        
        public WireGuardController()
        {
            _wireGuardBridge = new WireGuardBridge();
        }
        
        public async Task<bool> CreateInterfaceAsync(string name, string privateKey, int port)
        {
            try
            {
                // 检查驱动
                if (!_wireGuardBridge.IsDriverInstalled())
                {
                    await ShowDriverInstallPrompt();
                    return false;
                }
                
                // 创建接口
                var config = new InterfaceConfig
                {
                    Name = name,
                    PrivateKey = privateKey,
                    ListenPort = port,
                    Addresses = new[] { "10.10.0.2/24" }
                };
                
                bool success = await Task.Run(() => 
                    _wireGuardBridge.CreateInstance(config));
                    
                return success;
            }
            catch (Exception ex)
            {
                Logger.Error($"创建接口失败：{ex.Message}");
                return false;
            }
        }
        
        public void StartInterface(string name)
        {
            _wireGuardBridge.StartInterface(name);
        }
        
        public void StopInterface(string name)
        {
            _wireGuardBridge.StopInterface(name);
        }
        
        public List<WireGuardInterface> GetAllInterfaces()
        {
            return _wireGuardBridge.GetAllInterfaces();
        }
    }
}
```

### 配置管理

#### 解析 WireGuard 配置文件
```cpp
#include "Core/Config/ConfigParser.h"
#include "Core/Config/ConfigManager.h"

using namespace WVLAN::Core;

// 1. 从文件解析
ParseResult result = ConfigParser::ParseFromFile("C:\\Users\\User\\.wg.conf");

if (result.Success) {
    const auto& config = result.Config;
    
    // 访问接口配置
    std::string privateKey = config.Interface.PrivateKey;
    std::string address = config.Interface.Address;
    int port = config.Interface.ListenPort;
    
    // 遍历对等节点
    for (const auto& peer : config.Peers) {
        std::cout << "Peer: " << peer.PublicKey << std::endl;
        std::cout << "AllowedIPs: " << peer.AllowedIPs << std::endl;
        std::cout << "Endpoint: " << peer.Endpoint << std::endl;
    }
}

// 2. 保存到配置管理器
ConfigManager& configManager = ConfigManager::Instance();
configManager.SaveConfiguration("MyVPN", result.Config);

// 3. 加载配置
auto loadedConfig = configManager.LoadConfiguration("MyVPN");
```

#### C# 配置管理
```csharp
using WVLAN.Bridge;

public class ConfigurationService
{
    private readonly ConfigBridge _configBridge;
    
    public ConfigurationService()
    {
        _configBridge = new ConfigBridge();
    }
    
    public async Task<bool> ImportConfigurationAsync(string filePath, string name)
    {
        return await Task.Run(() => 
            _configBridge.ImportConfiguration(filePath, name));
    }
    
    public async Task<List<string>> GetSavedConfigurationsAsync()
    {
        return await Task.Run(() => 
            _configBridge.GetSavedConfigurationNames());
    }
    
    public async Task<bool> DeleteConfigurationAsync(string name)
    {
        return await Task.Run(() => 
            _configBridge.DeleteConfiguration(name));
    }
}
```

### 路由配置

```cpp
#include "Core/Routing/RouteTableManager.h"
#include "Core/Routing/RouteConfigurator.h"

using namespace WVLAN::Core;

// 1. 获取路由表管理器
auto& router = RouteTableManager::Instance();

// 2. 添加路由
RouteEntry route;
route.Destination = "10.10.0.0";
route.Netmask = "255.255.255.0";
route.Gateway = "10.10.0.1";
route.Metric = 100;

auto result = router.AddRoute(route);
if (result.Success) {
    std::cout << "路由添加成功" << std::endl;
}

// 3. 删除路由
router.DeleteRoute("10.10.0.0", "255.255.255.0");

// 4. 自动配置 AllowedIPs 路由
RouteConfigurator configurator;
configurator.ConfigureRoutesForInterface("WVLAN0", config.Peers);
```

### 日志系统

```cpp
#include "Utils/Logger.h"

using namespace WVLAN::Utils;

// 设置日志级别
Logger::SetLogLevel(LogLevel::Debug);

// 写入日志
Logger::Info("接口 WVLAN0 已创建");
Logger::Debug("私钥：{}", privateKey);
Logger::Warning("端口已被占用，使用备用端口 {}", altPort);
Logger::Error("连接失败：{}", errorMessage);

// C# 中访问
Logger.Info("从 C# 写入日志信息");
```

## 错误处理

### 错误码定义
```cpp
enum class WireGuardError {
    Success = 0,                  // 成功
    DriverNotInstalled = 1,       // 驱动未安装
    InvalidConfig = 2,            // 配置无效
    InterfaceNotFound = 3,        // 接口不存在
    CreateFailed = 4,             // 创建失败
    StartFailed = 5,              // 启动失败
    StopFailed = 6,               // 停止失败
    DeleteFailed = 7,             // 删除失败
    AccessDenied = 8              // 访问被拒绝
};
```

### 异常处理最佳实践
```cpp
try {
    auto& manager = WireGuardManager::Instance();
    auto result = manager.CreateInterface(config);
    
    if (!result.Success) {
        switch (result.ErrorCode) {
            case WireGuardError::DriverNotInstalled:
                // 提示安装驱动
                break;
            case WireGuardError::AccessDenied:
                // 提示以管理员身份运行
                break;
            default:
                Logger.Error("未知错误：{}", static_cast<int>(result.ErrorCode));
        }
    }
}
catch (const std::exception& ex) {
    Logger.Error("异常：{}", ex.what());
}
```

## 安全注意事项

### 密钥管理
```cpp
#include "Utils/SecurityUtils.h"

using namespace WVLAN::Utils;

// 生成新密钥对
KeyPair keyPair = SecurityUtils::GenerateWireGuardKeyPair();

// 私钥应该安全存储
// 建议使用 Windows DPAPI 加密存储
std::string encryptedPrivateKey = EncryptWithDPAPI(keyPair.PrivateKey);
SaveToSecureStorage(encryptedPrivateKey);

// 使用时解密
std::string decryptedPrivateKey = DecryptFromDPAPI(encryptedPrivateKey);
```

### 内存清理
```cpp
// 敏感数据使用后应清空
std::vector<uint8_t> sensitiveData(32);
GenerateRandomBytes(sensitiveData.size(), sensitiveData.data());

// 使用后清零
std::fill(sensitiveData.begin(), sensitiveData.end(), 0);
```

## 性能优化建议

### 异步操作
```csharp
// 避免阻塞 UI 线程
private async void OnConnectClicked(object sender, RoutedEventArgs e)
{
    btnConnect.IsEnabled = false;
    
    try
    {
        await Task.Run(() => _controller.ConnectAsync());
        ShowStatus("已连接");
    }
    finally
    {
        btnConnect.IsEnabled = true;
    }
}
```

### 连接池
```cpp
// 复用配置对象减少分配
class ConfigurationPool
{
private:
    std::deque<NetworkConfig> _pool;
    
public:
    NetworkConfig& Acquire()
    {
        if (_pool.empty()) {
            return _pool.emplace_back();
        }
        auto& config = _pool.front();
        _pool.pop_front();
        return config;
    }
    
    void Release(NetworkConfig& config)
    {
        config.Reset();
        _pool.push_back(std::move(config));
    }
};
```

## 常见问题 FAQ

### Q1: 如何获取 WireGuard 密钥？
```bash
# 使用命令行工具 wg genkey
wg genkey | tee privatekey.txt | wg pubkey

# 或在代码中使用
auto keyPair = SecurityUtils::GenerateWireGuardKeyPair();
```

### Q2: 如何处理"访问被拒绝"错误？
需要以管理员身份运行应用程序。在清单文件中添加：
```xml
<requestedPrivileges xmlns="urn:schemas-microsoft-com:asm.v3">
    <requestedExecutionLevel level="requireAdministrator" uiAccess="false"/>
</requestedPrivileges>
```

### Q3: 如何调试 WireGuard 驱动问题？
```cpp
// 启用详细日志
Logger::SetLogLevel(LogLevel::Debug);

// 查看 Windows 事件日志
// 事件查看器 -> Windows 日志 -> 系统 -> 来源：WireGuard
```

### Q4: 支持哪些 Windows 版本？
- Windows 10 version 1803 及更高版本
- Windows Server 2019 及更高版本

## 贡献指南

### 代码风格
- 头文件使用 `.h` 扩展名
- 源文件使用 `.cpp` 扩展名
- 命名空间 `WVLAN::Core`, `WVLAN::Utils`, `WVLAN::Bridge`
- 类名使用 PascalCase
- 成员变量使用 `_camelCase`
- 注释使用中文

### 提交代码
```bash
# 创建特性分支
git checkout -b feature/new-feature

# 编写代码并提交
git add .
git commit -m "feat: 添加新功能"

# 推送到远程
git push origin feature/new-feature
```

## 许可证

本项目采用 MIT 许可证。详见 LICENSE 文件。

## 联系方式

- 项目地址：https://github.com/wvlan/wvlan-client
- 问题反馈：https://github.com/wvlan/wvlan-client/issues
- 开发团队：WVLAN Team

---

**版本**: 1.0.0  
**最后更新**: 2026-04-15  
**文档维护**: WVLAN 开发团队
