# Control Server - Windows 虚拟局域网控制服务器

## 项目概述

控制服务器是 Windows 虚拟局域网应用的核心组件，负责：
- 用户认证与管理
- 设备注册与发现
- 配置分发与管理
- 连接日志记录
- gRPC 内部通信

## 技术栈

- **语言**: Go 1.21
- **Web 框架**: Gin
- **数据库**: PostgreSQL 15
- **缓存**: Redis 7
- **ORM**: GORM
- **认证**: JWT
- **RPC**: gRPC
- **容器化**: Docker + Docker Compose

## 项目结构

```
control-server/
├── cmd/
│   └── server/
│       └── main.go          # 应用入口
├── internal/
│   ├── api/                 # RESTful API
│   │   ├── handlers/        # 请求处理器
│   │   ├── middleware/      # 中间件
│   │   └── router.go        # 路由配置
│   ├── config/              # 配置管理
│   ├── database/            # 数据库连接
│   ├── models/              # 数据模型
│   ├── grpc/                # gRPC 服务
│   │   ├── server.go        # gRPC 服务器
│   │   └── services/        # gRPC 服务实现
│   ├── redis/               # Redis 连接
│   └── utils/               # 工具函数
├── migrations/              # 数据库迁移脚本
├── proto/                   # gRPC 协议定义
├── pkg/                     # 公共包
│   ├── auth/                # 认证相关
│   └── logger/              # 日志工具
├── tests/                   # 测试文件
├── docker/                  # Docker 配置
├── docs/                    # 文档
│   └── api/                 # API 文档
├── .env.example             # 环境变量示例
├── docker-compose.yml       # Docker Compose 配置
├── Dockerfile               # Docker 镜像配置
└── README.md                # 项目说明
```

## 快速开始

### 前置要求

- Go 1.21+
- Docker 20.10+
- Docker Compose 2.0+

### 本地开发

1. **克隆项目**
```bash
cd control-server
```

2. **配置环境变量**
```bash
cp .env.example .env
# 编辑 .env 文件，配置数据库连接等信息
```

3. **安装依赖**
```bash
go mod download
```

4. **运行数据库迁移**
```bash
sql-migrate up -config=migrations/migrate.yml
```

5. **启动服务**
```bash
go run cmd/server/main.go
```

### Docker 部署

1. **启动所有服务**
```bash
docker-compose up -d
```

2. **查看日志**
```bash
docker-compose logs -f
```

3. **停止服务**
```bash
docker-compose down
```

## API 文档

API 文档可通过 Swagger UI 访问：
- 地址：http://localhost:8080/swagger/index.html
- OpenAPI 规范：http://localhost:8080/swagger/openapi.json

## 主要功能

### 用户管理
- 用户注册/登录
- JWT 令牌管理
- 权限控制

### 设备管理
- 设备注册与认证
- 设备状态监控
- 设备发现

### 配置管理
- 网络配置分发
- 配置版本控制
- 配置更新推送

### 连接管理
- 连接日志记录
- 连接状态查询
- 连接统计

## 开发指南

### 代码规范
- 遵循 Go 官方代码规范
- 使用 `gofmt` 格式化代码
- 添加完整的注释和文档

### 测试
```bash
# 运行所有测试
go test ./... -v

# 生成测试覆盖率报告
go test ./... -coverprofile=coverage.out
go tool cover -html=coverage.out
```

### 数据库迁移
```bash
# 创建新迁移
sql-migrate new migration_name

# 执行迁移
sql-migrate up -config=migrations/migrate.yml

# 回滚迁移
sql-migrate down -config=migrations/migrate.yml
```

## 环境变量

| 变量名 | 说明 | 默认值 |
|--------|------|--------|
| APP_ENV | 运行环境 | development |
| APP_PORT | API 服务端口 | 8080 |
| GRPC_PORT | gRPC 服务端口 | 9090 |
| DB_HOST | 数据库主机 | localhost |
| DB_PORT | 数据库端口 | 5432 |
| DB_USER | 数据库用户 | postgres |
| DB_PASSWORD | 数据库密码 | postgres |
| DB_NAME | 数据库名称 | control_server |
| REDIS_HOST | Redis 主机 | localhost |
| REDIS_PORT | Redis 端口 | 6379 |
| JWT_SECRET | JWT 密钥 | 请修改为随机字符串 |
| JWT_EXPIRE | JWT 过期时间 (小时) | 24 |

## 许可证

MIT License
