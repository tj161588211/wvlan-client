# Control Server API 文档

## 概述

Control Server 提供 RESTful API 用于管理 Windows 虚拟局域网应用的用户、设备、配置和连接。

**基础 URL**: `http://localhost:8080/api/v1`

## 认证

大多数 API 端点需要 JWT 认证。在请求头中添加：

```
Authorization: Bearer {your-jwt-token}
```

## 响应格式

### 成功响应

```json
{
  "message": "操作成功",
  "data": {...}
}
```

### 错误响应

```json
{
  "error": "错误描述"
}
```

## 用户管理 API

### 注册用户

**POST** `/users/register`

创建新用户账户。

**请求体**:
```json
{
  "username": "string (3-50 字符)",
  "email": "string (有效邮箱)",
  "password": "string (最少 8 字符)"
}
```

**响应**: `201 Created`
```json
{
  "message": "用户注册成功",
  "user_id": "uuid"
}
```

### 用户登录

**POST** `/users/login`

用户登录并获取 JWT 令牌。

**请求体**:
```json
{
  "username": "string",
  "password": "string"
}
```

**响应**: `200 OK`
```json
{
  "message": "登录成功",
  "token": "jwt-token-string",
  "user": {
    "id": "uuid",
    "username": "string",
    "email": "string",
    "role": "user"
  }
}
```

### 获取用户资料

**GET** `/users/profile`

获取当前登录用户的资料信息（需要认证）。

**响应**: `200 OK`
```json
{
  "id": "uuid",
  "username": "string",
  "email": "string",
  "role": "user"
}
```

### 更新用户资料

**PUT** `/users/profile`

更新当前用户的资料（需要认证）。

**请求体**:
```json
{
  "email": "new-email@example.com"
}
```

### 修改密码

**POST** `/users/change-password`

修改当前用户的密码（需要认证）。

**请求体**:
```json
{
  "old_password": "string",
  "new_password": "string (最少 8 字符)"
}
```

## 设备管理 API

### 注册设备

**POST** `/devices`

为用户注册新设备（需要认证）。

**请求体**:
```json
{
  "name": "string (1-100 字符)",
  "type": "windows|linux|macos|other",
  "os_type": "string",
  "version": "string",
  "mac": "string (MAC 地址格式)",
  "public_key": "string (可选)"
}
```

**响应**: `201 Created`
```json
{
  "message": "设备注册成功",
  "device_id": "uuid"
}
```

### 获取设备列表

**GET** `/devices`

获取当前用户的所有设备（需要认证）。

**查询参数**:
- `status` - 过滤状态 (online/offline)

**响应**: `200 OK`
```json
[
  {
    "id": "uuid",
    "user_id": "uuid",
    "name": "string",
    "type": "string",
    "os_type": "string",
    "version": "string",
    "mac": "string",
    "status": "online|offline",
    "last_seen": "ISO8601 时间",
    "created_at": "ISO8601 时间"
  }
]
```

### 获取设备详情

**GET** `/devices/{id}`

获取指定设备的详细信息（需要认证）。

**响应**: `200 OK`
```json
{
  "id": "uuid",
  "user_id": "uuid",
  "name": "string",
  "type": "string",
  "os_type": "string",
  "version": "string",
  "ip": "string",
  "mac": "string",
  "status": "string",
  "last_seen": "ISO8601 时间",
  "created_at": "ISO8601 时间"
}
```

### 更新设备信息

**PUT** `/devices/{id}`

更新设备信息（需要认证）。

**请求体**:
```json
{
  "name": "string",
  "type": "string",
  "version": "string"
}
```

### 删除设备

**DELETE** `/devices/{id}`

删除指定设备（需要认证）。

**响应**: `200 OK`
```json
{
  "message": "设备删除成功"
}
```

### 更新设备状态

**PUT** `/devices/{id}/status`

更新设备的在线状态（需要认证）。

**请求体**:
```json
{
  "status": "online|offline",
  "ip": "string"
}
```

## 配置管理 API

### 创建配置

**POST** `/configs`

创建新的设备配置（需要认证）。

**请求体**:
```json
{
  "device_id": "uuid",
  "name": "string (1-100 字符)",
  "description": "string",
  "type": "network|security|general",
  "content": "string (JSON 格式)"
}
```

**响应**: `201 Created`
```json
{
  "message": "配置创建成功",
  "config_id": "uuid",
  "version": 1
}
```

### 获取配置列表

**GET** `/configs`

获取当前用户的所有配置（需要认证）。

**查询参数**:
- `device_id` - 设备 ID 过滤
- `type` - 配置类型过滤
- `active_only` - 只获取激活的配置 (true/false)

**响应**: `200 OK`
```json
[
  {
    "id": "uuid",
    "user_id": "uuid",
    "device_id": "uuid",
    "name": "string",
    "description": "string",
    "type": "string",
    "version": 1,
    "is_active": true,
    "created_at": "ISO8601 时间"
  }
]
```

### 获取配置详情

**GET** `/configs/{id}`

获取指定配置的详细信息（需要认证）。

**响应**: `200 OK`
```json
{
  "id": "uuid",
  "user_id": "uuid",
  "device_id": "uuid",
  "name": "string",
  "description": "string",
  "type": "string",
  "content": "string (JSON)",
  "version": 1,
  "is_active": true,
  "created_at": "ISO8601 时间"
}
```

### 更新配置

**PUT** `/configs/{id}`

更新指定配置（需要认证）。

**请求体**:
```json
{
  "name": "string",
  "description": "string",
  "content": "string",
  "is_active": true
}
```

### 删除配置

**DELETE** `/configs/{id}`

删除指定配置（需要认证）。

**响应**: `200 OK`
```json
{
  "message": "配置删除成功"
}
```

### 发布配置

**POST** `/configs/{id}/publish`

将配置发布到指定设备（需要认证）。

**请求体**:
```json
{
  "device_ids": ["uuid1", "uuid2"]
}
```

**响应**: `200 OK`
```json
{
  "message": "配置发布成功",
  "published_count": 2
}
```

## 连接管理 API

### 获取连接历史

**GET** `/connections`

获取用户的设备连接历史记录（需要认证）。

**查询参数**:
- `device_id` - 设备 ID 过滤
- `start_time` - 开始时间 (RFC3339)
- `end_time` - 结束时间 (RFC3339)
- `limit` - 返回数量限制 (默认 100)
- `offset` - 偏移量 (默认 0)

**响应**: `200 OK`
```json
{
  "total": 150,
  "logs": [
    {
      "id": "uuid",
      "user_id": "uuid",
      "device_id": "uuid",
      "connection_id": "string",
      "action": "connect|disconnect|reconnect",
      "status": "success|failed",
      "client_ip": "string",
      "duration": 3600000,
      "connection_time": "ISO8601 时间"
    }
  ]
}
```

### 获取连接统计

**GET** `/connections/stats/{device_id}`

获取设备的连接统计信息（需要认证）。

**响应**: `200 OK`
```json
{
  "device_id": "uuid",
  "total_connections": 150,
  "successful_count": 145,
  "failed_count": 5,
  "avg_duration": 3600000,
  "last_connect_time": "ISO8601 时间",
  "current_status": "online"
}
```

### 记录连接事件

**POST** `/connections/record`

记录连接事件（内部使用）。

**请求体**:
```json
{
  "user_id": "uuid",
  "device_id": "uuid",
  "connection_id": "string",
  "action": "connect|disconnect|reconnect",
  "status": "success|failed",
  "client_ip": "string",
  "user_agent": "string",
  "error_message": "string",
  "duration": 3600000
}
```

## 错误代码

| 状态码 | 说明 |
|--------|------|
| 200 | 成功 |
| 201 | 创建成功 |
| 400 | 请求参数错误 |
| 401 | 未授权 |
| 403 | 禁止访问 |
| 404 | 资源不存在 |
| 409 | 资源冲突 |
| 429 | 请求过于频繁 |
| 500 | 服务器内部错误 |

## 示例请求

### 完整流程示例

1. **注册用户**
```bash
curl -X POST http://localhost:8080/api/v1/users/register \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser",
    "email": "test@example.com",
    "password": "password123"
  }'
```

2. **用户登录**
```bash
curl -X POST http://localhost:8080/api/v1/users/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser",
    "password": "password123"
  }'
```

3. **注册设备** (使用上一步获得的 token)
```bash
curl -X POST http://localhost:8080/api/v1/devices \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_JWT_TOKEN" \
  -d '{
    "name": "Windows Desktop",
    "type": "windows",
    "os_type": "Windows 11",
    "version": "22H2",
    "mac": "00:1A:2B:3C:4D:5E"
  }'
```

4. **创建配置**
```bash
curl -X POST http://localhost:8080/api/v1/configs \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_JWT_TOKEN" \
  -d '{
    "device_id": "DEVICE_ID_FROM_PREVIOUS_STEP",
    "name": "网络配置",
    "type": "network",
    "content": "{\"subnet\": \"10.0.0.0/24\", \"gateway\": \"10.0.0.1\"}"
  }'
```

## Swagger UI

启动服务后，访问 http://localhost:8080/swagger/index.html 查看交互式 API 文档。
