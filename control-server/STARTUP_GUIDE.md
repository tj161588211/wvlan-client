# 控制服务器本地启动指南

## 前置要求

确保您的系统已安装以下软件：

- **Go 1.21+**: [下载](https://golang.org/dl/)
- **PostgreSQL 15+**: [下载](https://www.postgresql.org/download/)
- **Redis 7+**: [下载](https://redis.io/download/)
- **Docker 20.10+** (可选): [下载](https://www.docker.com/get-started)
- **Docker Compose 2.0+** (可选)

## 快速开始

### 方法一：使用 Docker Compose (推荐)

这是最简单的方式，会自动启动所有依赖服务。

```bash
# 1. 进入项目目录
cd /root/.hermes/hermes-agent/control-server

# 2. 复制环境变量配置
cp .env.example .env

# 3. 编辑 .env 文件，根据需要修改配置
# 特别要修改 JWT_SECRET 为随机字符串

# 4. 启动所有服务
docker-compose up -d

# 5. 查看日志
docker-compose logs -f

# 6. 访问 API
# REST API: http://localhost:8080
# Swagger 文档：http://localhost:8080/swagger/index.html
# 健康检查：http://localhost:8080/health
```

### 方法二：本地开发环境

#### 1. 安装依赖

```bash
cd /root/.hermes/hermes-agent/control-server

# 下载 Go 模块依赖
go mod download

# 或者安装所有依赖
make install
```

#### 2. 配置环境变量

```bash
# 复制环境变量示例文件
cp .env.example .env

# 编辑 .env 文件
# 确保数据库和 Redis 已启动并可用
```

#### 3. 初始化数据库

```bash
# 创建数据库（如果不存在）
psql -U postgres -c "CREATE DATABASE control_server;"

# 运行数据库迁移
sql-migrate up -config=migrations/migrate.yml

# 或者使用 make
make migrate-up
```

#### 4. 启动服务

```bash
# 开发模式
go run cmd/server/main.go

# 或者使用 make
make run
```

#### 5. 验证服务

```bash
# 健康检查
curl http://localhost:8080/health

# 用户注册
curl -X POST http://localhost:8080/api/v1/users/register \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser",
    "email": "test@example.com",
    "password": "password123"
  }'

# 用户登录
curl -X POST http://localhost:8080/api/v1/users/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser",
    "password": "password123"
  }'
```

## 服务端口

| 服务 | 端口 | 说明 |
|------|------|------|
| REST API | 8080 | HTTP API 服务 |
| gRPC | 9090 | 内部通信服务 |
| PostgreSQL | 5432 | 数据库 |
| Redis | 6379 | 缓存 |

## API 文档

启动服务后，访问以下地址查看 API 文档：

- **Swagger UI**: http://localhost:8080/swagger/index.html
- **OpenAPI JSON**: http://localhost:8080/swagger/openapi.json

## 常用命令

### 开发

```bash
# 运行测试
make test

# 生成测试覆盖率报告
make test-cover

# 格式化代码
make fmt

# 运行代码检查
make lint
```

### 数据库

```bash
# 执行迁移
make migrate-up

# 回滚迁移
make migrate-down

# 创建新迁移
make migrate-new
```

### Docker

```bash
# 构建镜像
make docker-build

# 启动服务
make docker-compose-up

# 查看日志
make docker-compose-logs

# 停止服务
make docker-compose-down
```

## 配置说明

### 环境变量

| 变量名 | 说明 | 默认值 |
|--------|------|--------|
| APP_ENV | 运行环境 | development |
| APP_PORT | API 端口 | 8080 |
| GRPC_PORT | gRPC 端口 | 9090 |
| DB_HOST | 数据库主机 | localhost |
| DB_PORT | 数据库端口 | 5432 |
| DB_USER | 数据库用户 | postgres |
| DB_PASSWORD | 数据库密码 | postgres |
| DB_NAME | 数据库名称 | control_server |
| REDIS_HOST | Redis 主机 | localhost |
| REDIS_PORT | Redis 端口 | 6379 |
| JWT_SECRET | JWT 密钥 | **必须修改** |
| JWT_EXPIRE | JWT 过期时间 (小时) | 24 |

### 生产环境配置

生产环境部署时，请确保：

1. **修改 JWT_SECRET**：使用强随机字符串
2. **启用 SSL/TLS**：配置 HTTPS
3. **设置正确的日志级别**：LOG_LEVEL=info 或 warn
4. **配置数据库连接池**：根据服务器性能调整
5. **启用 Redis 密码**：设置 REDIS_PASSWORD

## 故障排查

### 数据库连接失败

```bash
# 检查 PostgreSQL 是否运行
pg_isready -h localhost -p 5432

# 检查数据库是否存在
psql -U postgres -l

# 查看数据库日志
tail -f /var/log/postgresql/postgresql-15-main.log
```

### Redis 连接失败

```bash
# 检查 Redis 是否运行
redis-cli ping

# 查看 Redis 日志
tail -f /var/log/redis/redis-server.log
```

### 端口占用

```bash
# 检查端口占用
lsof -i :8080
lsof -i :9090

# 杀死占用进程
kill -9 <PID>
```

## 下一步

1. 查看 [API 文档](http://localhost:8080/swagger/index.html) 了解可用接口
2. 实现业务逻辑（当前为 Mock 实现）
3. 配置生产环境
4. 部署到服务器

## 支持

如有问题，请查看：
- [README.md](./README.md) - 项目说明
- [docs/api/](./docs/api/) - API 文档
- [migrations/](./migrations/) - 数据库迁移脚本
