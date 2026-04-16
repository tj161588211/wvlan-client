# WVLAN - Windows Virtual LAN

基于 WireGuard 的虚拟局域网应用，让多台 Windows 设备通过同一配置文件自动组网。

## 🚀 快速开始

### 安装
下载最新版本的安装包:
- [WVLAN-Setup.exe](https://github.com/tj161588211/wvlan-client/releases/latest)

### 使用
1. 运行 `WVLAN-Setup.exe` 安装
2. 启动程序后导入 `.conf` 配置文件
3. 点击"连接"即可加入虚拟局域网

## 📁 项目结构

```
wvlan-client/
├── wvlan-client/          # C++ 核心引擎 + WPF GUI
│   ├── src/
│   │   ├── Bridge/        # C++/CLI 桥接层
│   │   ├── Core/          # WireGuard 封装、配置管理、路由
│   │   └── Utils/         # 工具类
│   └── tests/             # 单元测试
├── control-server/        # Go 微服务后端
│   ├── cmd/               # 入口
│   ├── internal/          # 业务逻辑
│   ├── pkg/               # 公共库
│   └── docker-compose.yml
├── scripts/installer/     # 安装包构建脚本
│   ├── nsis/              # NSIS 脚本
│   └── innosetup/         # Inno Setup 脚本
└── .github/workflows/     # CI/CD
```

## 🔧 技术栈

| 模块 | 技术栈 |
|------|--------|
| **GUI** | WPF (.NET 8) + XAML |
| **核心** | C++17 + WireGuard-for-C |
| **桥接** | C++/CLI |
| **后端** | Go 1.21 + Gin + gRPC |
| **数据库** | PostgreSQL + Redis |
| **部署** | Docker Compose |
| **CI/CD** | GitHub Actions |

## 📊 开发进度

- ✅ 产品需求文档 (PRD)
- ✅ 架构设计文档
- ✅ UI 设计规范 + Fluent Design
- ✅ C++ WireGuard 核心引擎 (~13.5 万行)
- ✅ WPF 图形界面
- ✅ Go 控制服务器
- ✅ CI/CD 流水线
- ✅ 安装包脚本 (NSIS + Inno Setup)

## 🛠️ 本地构建

### 前端
```bash
cd wvlan-client/src/WpfClient
dotnet build --configuration Release
```

### 后端
```bash
cd control-server
docker-compose up -d
```

### 安装包
```powershell
cd scripts/installer
.uild-installer.ps1 -Version "1.0.0" -Release
```

## 📝 许可证

MIT License

---

**开发者**: Tommy总 & Hermes Agent Team  
**版本**: v1.0.0
