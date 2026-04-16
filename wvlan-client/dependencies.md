# 依赖说明 - WVLAN WireGuard 框架

## 项目依赖

### 1. 开发环境依赖

#### 编译器工具链
- **Visual Studio 2022** (版本 17.0+)
  - 工作负载要求:
    - 使用 C++ 的桌面开发
    - .NET 桌面开发
  - 组件要求:
    - MSVC v143 - VS 2022 C++ x64/x86生成工具
    - Windows 10/11 SDK (10.0.19041.0+)
    - C++/CLI 支持

- **CMake** 3.20+
  - 下载地址：https://cmake.org/download/
  - 用于跨平台构建配置

- **Git** 2.30+
  - 用于依赖管理和版本控制

### 2. 第三方库依赖

#### WireGuard-for-C (必需)
- **描述**: WireGuard 官方 C 语言实现
- **版本**: 0.0.20230826 或更新
- **来源**: https://git.zx2c4.com/WireGuard-for-C
- **集成方式**:
  ```bash
  git clone https://git.zx2c4.com/WireGuard-for-C lib/wireguard-c-src
  ```
- **用途**: 提供 WireGuard 协议实现，包括加密、密钥交换、网络隧道等

#### Google Test (测试)
- **描述**: C++ 单元测试框架
- **版本**: 1.12.1+
- **来源**: https://github.com/google/googletest
- **集成方式**: CMake FetchContent 自动下载
- **用途**: 单元测试

#### 可选：libsodium (加密)
- **描述**: 现代加密库
- **版本**: 1.0.18+
- **来源**: https://github.com/jedisct1/libsodium
- **用途**: 增强加密功能（当前使用 Windows Crypto API）

### 3. Windows SDK 依赖

#### 必需 API
```cmake
# 网络相关
ws2_32.lib      # Winsock 2 网络 API
iphlpapi.lib    # IP 辅助 API (路由表、接口管理)

# 安全相关
advapi32.lib    # 高级 Windows API (注册表、服务)
crypt32.lib     # 加密 API (Base64、证书)

# 系统相关
kernel32.lib    # 核心系统 API
user32.lib      # 用户界面 API (可选)
```

#### 必需头文件
```cpp
<winsock2.h>      # Socket 编程
<iphlpapi.h>      # IP 辅助功能
<windows.h>       # Windows API
<wincrypt.h>      # 加密 API
<ws2tcpip.h>      # TCP/IP Socket 扩展
```

### 4. .NET 依赖 (C++/CLI桥接层)

#### .NET 运行时
- **.NET 6.0** 或 **.NET 7.0**
- 用于 C++/CLI 桥接层
- 目标框架：net6.0-windows 或 net7.0-windows

#### 程序集引用
```cpp
// 在 C++/CLI 文件中
using namespace System;
using namespace System::IO;
using namespace System::Net;
using namespace System::Threading;
using namespace System::Collections::Generic;
```

## 依赖安装指南

### Windows 环境设置

#### 1. 安装 Visual Studio 2022
```powershell
# 下载 Visual Studio Installer
winget install Microsoft.VisualStudio.2022.Community

# 安装后添加必要组件
# 运行 Visual Studio Installer -> 修改 -> 勾选:
# - 使用 C++ 的桌面开发
# - .NET 桌面开发
# - Windows 10/11 SDK
```

#### 2. 安装 CMake
```powershell
# 使用 winget
winget install Kitware.CMake

# 或使用 Chocolatey
choco install cmake --installargs 'ADD_CMAKE_TO_PATH=System'

# 验证安装
cmake --version  # 应显示 3.20+
```

#### 3. 获取 WireGuard-for-C
```bash
# 克隆仓库
git clone https://git.zx2c4.com/WireGuard-for-C lib/wireguard-c-src

# 切换到稳定版本
cd lib/wireguard-c-src
git checkout 0.0.20230826

# 构建库 (可选，CMake 会自动处理)
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

#### 4. 验证环境
```powershell
# 检查工具链
cl   # 应显示 MSVC 版本
cmake --version
git --version

# 检查 SDK
dir "C:\Program Files (x86)\Windows Kits\10\Include"
```

## 依赖版本兼容性

| 组件 | 最低版本 | 推荐版本 | 备注 |
|------|---------|---------|------|
| Visual Studio | 2022 17.0 | 2022 17.8+ | 需要 C++17 支持 |
| CMake | 3.20 | 3.27+ | FetchContent 需要 |
| Windows SDK | 10.0.19041 | 10.0.22621 | Win10 1803+ |
| .NET | 6.0 | 7.0 | LTS 版本推荐 |
| WireGuard-for-C | 0.0.20220401 | 0.0.20230826 | API 稳定性 |
| Google Test | 1.10.0 | 1.12.1 | 测试框架 |

## 构建依赖树

```
wvlan-client
├── wireguard-c (libwg)
│   ├── libsodium (可选，内置)
│   └── zlib (可选)
├── gtest (测试)
│   └── gmock
├── Windows SDK
│   ├── ws2_32 (网络)
│   ├── iphlpapi (路由)
│   ├── advapi32 (安全)
│   └── crypt32 (加密)
└── .NET Runtime (桥接层)
```

## 依赖管理策略

### 1. WireGuard 库
- **开发环境**: 从源码构建
- **生产环境**: 使用预编译库
- **版本锁定**: 在 CMakeLists.txt 中指定版本标签

### 2. Google Test
- **自动下载**: 通过 CMake FetchContent
- **隔离构建**: 排除在主要安装之外
- **条件编译**: 仅当 WVLAN_BUILD_TESTS=ON 时包含

### 3. 系统库
- **系统提供**: 使用 Windows SDK 自带库
- **版本检测**: CMake 检查 SDK 版本
- **回退策略**: 提供降级选项

## 常见问题

### Q1: WireGuard 库找不到
**解决方案**:
```bash
# 方法 1: 从源码构建
cmake .. -DWVLAN_BUILD_WIREGUARD=ON

# 方法 2: 指定库路径
cmake .. -DWIREGUARD_LIB_PATH=/path/to/libwg.lib
```

### Q2: C++/CLI 编译错误
**解决方案**:
- 确保安装了".NET 桌面开发"工作负载
- 检查目标框架版本设置
- 避免在 C++/CLI 文件中使用 C++17 特性

### Q3: 链接错误 - 找不到 iphlpapi.lib
**解决方案**:
- 安装 Windows SDK
- 检查 CMakeLists.txt 中的 target_link_libraries

### Q4: Google Test 下载失败
**解决方案**:
```bash
# 手动下载
git clone https://github.com/google/googletest.git lib/googletest

# 禁用测试
cmake .. -DWVLAN_BUILD_TESTS=OFF
```

## 离线安装

### 准备依赖包
```bash
# 1. 下载 WireGuard-for-C
git clone --depth 1 --branch 0.0.20230826 \
    https://git.zx2c4.com/WireGuard-for-C \
    offline-deps/wireguard-c-src

# 2. 下载 Google Test
git clone --depth 1 --branch release-1.12.1 \
    https://github.com/google/googletest \
    offline-deps/googletest

# 3. 复制整个依赖目录到离线机器
```

### 离线构建
```bash
# 在离线机器上
cmake .. \
    -DWVLAN_BUILD_WIREGUARD=ON \
    -DGTEST_SOURCE_DIR=../offline-deps/googletest
```

## 持续集成依赖

### GitHub Actions
```yaml
- name: Setup dependencies
  run: |
    # WireGuard 自动构建
    cmake -DWVLAN_BUILD_WIREGUARD=ON
    
    # Google Test 自动下载
    # 由 CMake FetchContent 处理
```

### Azure DevOps
```yaml
- task: CMake@1
  inputs:
    workingDirectory: 'build'
    command: configure
    args: '-DWVLAN_BUILD_WIREGUARD=ON -DWVLAN_BUILD_TESTS=ON'
```

## 许可证信息

| 依赖 | 许可证 | 备注 |
|------|--------|------|
| WireGuard-for-C | GPL-2.0 | 核心协议实现 |
| Google Test | BSD-3-Clause | 测试框架 |
| libsodium | ISC | 加密库 (可选) |
| Windows SDK | Microsoft | 系统 API |

---

**版本**: 1.0.0  
**最后更新**: 2026-04-15  
**维护者**: WVLAN 开发团队
