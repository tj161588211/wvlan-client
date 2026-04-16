# 构建脚本 - WVLAN WireGuard 框架

PowerShell 版本

## 使用说明

```powershell
# 在 Windows PowerShell 中运行

# 1. 配置并编译项目
.\scripts\build.ps1

# 2. 仅配置（不编译）
.\scripts\build.ps1 -ConfigureOnly

# 3. 仅编译（不配置）
.\scripts\build.ps1 -BuildOnly

# 4. 运行测试
.\scripts\build.ps1 -RunTests

# 5. 清理并重新构建
.\scripts\build.ps1 -CleanRebuild
```

## 功能说明

此脚本提供以下功能：
- 自动检测开发环境
- 下载 WireGuard-for-C 依赖
- 配置 CMake 构建系统
- 编译项目
- 运行单元测试
- 生成安装包（未来支持）

## 前置检查

运行脚本前请确保：
- [x] Visual Studio 2022 已安装
- [x] CMake 已添加到 PATH
- [x] Git 已安装

---

本脚本用于自动化 WVLAN 项目的构建流程。
实际构建请在 Windows 环境下使用 Developer Command Prompt for VS 2022。
