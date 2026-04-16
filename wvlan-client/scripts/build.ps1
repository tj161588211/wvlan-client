# WVLAN WireGuard 框架 - 编译脚本

## 概述
此脚本用于在 Windows 环境下编译 WVLAN WireGuard 驱动集成框架。

## 前置要求

### 1. 开发环境
- **Visual Studio 2022** (推荐 17.0+)
  - 必须安装以下工作负载:
    - "使用 C++ 的桌面开发"
    - ".NET 桌面开发" (用于 C++/CLI 项目)
- **CMake** 3.20 或更高版本
- **Windows SDK** 10.0.19041.0 或更高版本

### 2. 依赖库
- **WireGuard-for-C** - 放置在 `lib/wireguard-c-src/`
- **Google Test** - 自动下载或通过 CMake 配置

### 3. 环境变量
```bash
# 设置 Visual Studio 环境（可选，如果不在 VS Developer Command Prompt 中运行）
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
```

## 编译步骤

### 方式一：使用 CMake GUI
1. 打开 CMake GUI
2. 设置源码目录为项目根目录
3. 设置构建目录（如 `build`）
4. 点击 "Configure"
5. 选择 "Visual Studio 17 2022" 和 x64 平台
6. 点击 "Generate"
7. 打开生成的 `.sln` 文件，在 Visual Studio 中编译

### 方式二：命令行编译

#### 1. 创建构建目录
```bash
mkdir build
cd build
```

#### 2. 配置 CMake
```bash
# 使用 Visual Studio 2022 x64
cmake .. -G "Visual Studio 17 2022" -A x64

# 或使用 Ninja（需要安装 Ninja）
cmake .. -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
```

#### 3. 编译项目
```bash
# 使用 Visual Studio Generator
cmake --build . --config Release

# 或直接在 build 目录使用 msbuild
msbuild WVLAN.sln /p:Configuration=Release /p:Platform=x64
```

#### 4. 运行测试
```bash
# 编译后运行测试
./tests/Release/WVLAN.Tests.exe
```

## 构建配置选项

### CMake 变量
```bash
# 启用调试信息
cmake .. -DCMAKE_BUILD_TYPE=Debug

# 启用代码覆盖率
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCOVERAGE=ON

# 禁用测试
cmake .. -DBUILD_TESTS=OFF

# 使用特定的 WireGuard 源码路径
cmake .. -DWIREGUARD_SOURCE_DIR=/path/to/wireguard-c
```

## 输出文件

编译成功后，将在 `build/` 目录生成以下文件:

```
build/
├── src/
│   ├── Core/
│   │   ├── libWVLAN_Core.lib
│   │   └── WVLAN_Core.dll
│   ├── Bridge/
│   │   ├── WVLAN_Bridge.dll (C++/CLI)
│   │   └── WVLAN_Bridge.pdb
│   └── Utils/
│       └── libWVLAN_Utils.lib
├── tests/
│   └── WVLAN.Tests.exe
└── bin/
    └── WVLAN.Client.exe (最终可执行文件)
```

## 常见问题

### 1. CMake 配置失败
**问题**: CMake 无法找到 Visual Studio
**解决**: 在 VS Developer Command Prompt 中运行，或手动设置 `CMAKE_GENERATOR`

### 2. 找不到 WireGuard 库
**问题**: CMake 错误提示找不到 WireGuard-for-C
**解决**: 
```bash
# 确保 wireguard-c-src 目录存在并包含源码
git clone https://git.zx2c4.com/WireGuard-for-C lib/wireguard-c-src
```

### 3. C++/CLI 编译错误
**问题**: `/clr` 编译选项不支持某些 C++ 特性
**解决**: 确保 C++/CLI 文件只包含必要的托管代码，核心逻辑保持为原生 C++

### 4. 链接错误
**问题**: 找不到 `iphlpapi.lib` 或其他 Windows SDK 库
**解决**: 确保已安装 Windows SDK，并在 CMake 中正确链接

## 清理构建

```bash
# 删除构建目录
rm -rf build/

# 或使用 CMake 清理
cmake --build build/ --target clean
```

## 跨平台编译（未来支持）

### Linux (使用 vcpkg)
```bash
# 安装 vcpkg
git clone https://github.com/microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh

# 安装依赖
./vcpkg/vcpkg install wireguard gtest

# 编译
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
make -j$(nproc)
```

## 持续集成

### GitHub Actions 示例
```yaml
name: Build

on: [push, pull_request]

jobs:
  build:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v2
      
      - name: Configure CMake
        run: cmake -B build -G "Visual Studio 17 2022" -A x64
      
      - name: Build
        run: cmake --build build --config Release
      
      - name: Test
        run: ./build/tests/Release/WVLAN.Tests.exe
```

## 性能优化

### 发布版本优化
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="/O2 /Ob2 /Ot"
```

### 启用 LTO (链接时优化)
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
```

## 调试支持

### 生成调试符号
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="/Zi"
```

### 启用详细编译输出
```bash
cmake --build . --config Release --verbose
```

## 依赖版本管理

### WireGuard-for-C
推荐版本：`0.0.20230826` 或更新
```bash
cd lib/wireguard-c-src
git checkout 0.0.20230826
```

### Google Test
推荐版本：`1.13.0` 或更新
通过 CMake 自动下载，或手动安装

---

**最后更新**: 2026-04-15
**维护者**: WVLAN 开发团队
