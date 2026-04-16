# WireGuard-for-C 依赖说明

## 概述

本项目使用 WireGuard-for-C 作为核心网络库。以下是依赖获取和集成的详细说明。

## 依赖列表

### 必需依赖

| 依赖名称 | 版本 | 说明 | 获取方式 |
|---------|------|------|---------|
| WireGuard-for-C | latest | WireGuard 用户空间库 | 子模块/源码下载 |
| Windows SDK | 10.0.19041.0+ | Windows API 支持 | Visual Studio 安装 |
| .NET SDK | 6.0+ | C# 编译支持 | 微软官网下载 |
| CMake | 3.20+ | 构建系统 | cmake.org |

### 可选依赖

| 依赖名称 | 版本 | 说明 |
|---------|------|------|
| Windows Driver Kit | latest | 驱动开发工具包 |
| vcpkg | latest | C++ 包管理器 |

## WireGuard-for-C 获取方式

### 方式一：Git 子模块（推荐）

```bash
# 初始化子模块
git submodule update --init --recursive

# 或手动添加
git submodule add https://git.zx2c4.com/WireGuard-for-C wireguard-c-src
```

### 方式二：源码下载

1. 访问 https://git.zx2c4.com/WireGuard-for-C/snapshot/WireGuard-for-C-master.zip
2. 解压到 `wireguard-c-src/` 目录

### 方式三：vcpkg 安装

```bash
vcpkg install wireguard
```

## 编译 WireGuard-for-C

```bash
cd wireguard-c-src
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

输出文件位置：
- `libwg.dll` - 动态库
- `libwg.lib` - 导入库
- `libwg-static.lib` - 静态库（如启用）

## 集成到项目

### 1. 复制库文件

```powershell
# 复制 DLL 到输出目录
Copy-Item wireguard-c-src/build/libwg.dll ./bin/Release/

# 复制导入库到 lib 目录
Copy-Item wireguard-c-src/build/libwg.lib ./lib/
```

### 2. 配置 CMake

```cmake
# 在 CMakeLists.txt 中添加
set(WIREGUARD_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/wireguard-c-src")
include_directories("${WIREGUARD_SOURCE_DIR}/src")
link_directories("${WMAKE_CURRENT_SOURCE_DIR}/lib")
```

## 驱动安装

WireGuard 驱动需要以管理员权限安装：

```powershell
# 使用官方安装程序
.\wireguard-c-src\tools\WireGuardInstaller.exe

# 或使用 sc 命令
sc create WireGuard type= kernel binPath= "C:\Path\To\wireguard.sys"
sc start WireGuard
```

## 常见问题

### Q: 编译时找不到 libwg.lib

A: 确保 WireGuard-for-C 已正确编译，检查输出目录。

### Q: 运行时找不到 libwg.dll

A: 将 libwg.dll 复制到可执行文件目录或系统 PATH 中。

### Q: 驱动安装失败

A: 确保以管理员权限运行，并检查 Windows 驱动程序签名设置。

## 更新依赖

```bash
# 更新子模块
git submodule update --remote wireguard-c-src

# 重新编译
cd wireguard-c-src
git pull
mkdir build && cd build
cmake .. && cmake --build .
```
