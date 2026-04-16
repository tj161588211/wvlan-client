# Control Server 项目文件清单

## 项目结构

```
control-server/
├── cmd/
│   └── server/
│       └── main.go                    # 应用入口点
├── internal/
│   ├── api/
│   │   ├── handlers/
│   │   │   ├── user.go               # 用户处理器
│   │   │   ├── device.go             # 设备处理器
│   │   │   ├── config.go             # 配置处理器
│   │   │   └── connection.go         # 连接日志处理器
│   │   ├── middleware/
│   │   │   ├── auth.go               # JWT 认证中间件
│   │   │   └── common.go             # 通用中间件
│   │   └── router.go                 # 路由配置
│   ├── config/
│   │   └── config.go                 # 配置管理
│   ├── database/
│   │   └── database.go               # 数据库连接
│   ├── grpc/
│   │   ├── server.go                 # gRPC 服务器
│   │   └── services/
│   │       └── device.go             # gRPC 服务实现
│   ├── models/
│   │   └── models.go                 # 数据模型定义
│   ├── redis/
│   │   └── redis.go                  # Redis 连接
│   └── utils/                        # 工具函数目录
├── pkg/
│   ├── auth/
│   │   ├── jwt.go                    # JWT 认证
│   │   └── jwt_test.go               # JWT 测试
│   └── logger/
│       └── logger.go                 # 日志工具
├── proto/
│   └── services.proto                # gRPC 协议定义
├── migrations/
│   ├── migrate.yml                   # 迁移配置
│   ├── 001_create_users.sql          # 用户表迁移
│   ├── 002_create_devices.sql        # 设备表迁移
│   ├── 003_create_configs.sql        # 配置表迁移
│   ├── 004_create_connection_logs.sql # 连接日志表迁移
│   └── 005_create_indexes.sql        # 索引优化迁移
├── tests/
│   └── handlers_test.go              # 处理器测试
├── docker/                           # Docker 相关目录
├── docs/
│   └── api/
│       └── swagger.go                # Swagger 文档
├── .env.example                      # 环境变量示例
├── .gitignore                        # Git 忽略文件
├── Dockerfile                        # Docker 镜像配置
├── docker-compose.yml                # Docker Compose 配置
├── go.mod                            # Go 模块定义
├── go.sum                            # Go 依赖校验
├── Makefile                          # 构建脚本
├── README.md                         # 项目说明
└── STARTUP_GUIDE.md                  # 启动指南
```

## 核心文件说明

### 1. 应用入口
- **cmd/server/main.go**: 应用主入口，负责初始化所有组件并启动服务

### 2. 配置管理
- **internal/config/config.go**: 配置加载和验证，支持环境变量

### 3. 数据模型
- **internal/models/models.go**: 定义用户、设备、配置、连接日志等实体

### 4. API 层
- **internal/api/router.go**: 路由配置和中间件设置
- **internal/api/handlers/*.go**: 各模块的 HTTP 请求处理器
- **internal/api/middleware/*.go**: JWT 认证、CORS、日志等中间件

### 5. gRPC 服务
- **proto/services.proto**: gRPC 服务定义（Protocol Buffers）
- **internal/grpc/server.go**: gRPC 服务器启动和管理
- **internal/grpc/services/device.go**: gRPC 服务实现

### 6. 数据库
- **internal/database/database.go**: 数据库连接管理
- **migrations/*.sql**: 数据库迁移脚本

### 7. 缓存
- **internal/redis/redis.go**: Redis 连接管理

### 8. 认证
- **pkg/auth/jwt.go**: JWT 令牌生成和验证

### 9. 日志
- **pkg/logger/logger.go**: 基于 zap 的日志工具

### 10. 测试
- **pkg/auth/jwt_test.go**: JWT 单元测试
- **internal/config/config_test.go**: 配置单元测试
- **tests/handlers_test.go**: API 处理器集成测试

### 11. 部署
- **Dockerfile**: 多阶段构建的 Docker 镜像
- **docker-compose.yml**: 完整的服务编排
- **Makefile**: 常用命令快捷方式

## 技术栈

- **语言**: Go 1.21
- **Web 框架**: Gin
- **ORM**: GORM
- **数据库**: PostgreSQL 15
- **缓存**: Redis 7
- **认证**: JWT (golang-jwt)
- **RPC**: gRPC + Protocol Buffers
- **日志**: zap
- **测试**: testing + testify
- **文档**: Swagger/OpenAPI
- **容器**: Docker + Docker Compose

## 主要功能

1. **用户管理**: 注册、登录、JWT 认证、资料管理
2. **设备管理**: 设备注册、状态更新、设备发现
3. **配置管理**: 配置创建、版本控制、配置分发
4. **连接管理**: 连接日志、状态监控、统计分析

## 快速参考

```bash
# 启动服务
make run

# 运行测试
make test

# 构建 Docker 镜像
make docker-build

# 启动完整环境
make docker-compose-up

# 查看 API 文档
# http://localhost:8080/swagger/index.html
```
