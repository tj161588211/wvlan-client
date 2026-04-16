// Package main
// @title Control Server API
// @version 1.0.0
// @description Windows 虚拟局域网控制服务器 API 文档
// @termsOfService http://swagger.io/terms/

// @contact.name API Support
// @contact.email support@virtual-lan.dev

// @license.name MIT
// @license.url https://opensource.org/licenses/MIT

// @host localhost:8080
// @BasePath /

// @securityDefinitions.apikey BearerAuth
// @in header
// @name Authorization
// @description JWT 令牌认证，格式：Bearer {token}

// @tag.name 用户管理
// @tag.description 用户注册、登录、资料管理

// @tag.name 设备管理
// @tag.description 设备注册、状态管理、设备列表

// @tag.name 配置管理
// @tag.description 配置创建、更新、发布

// @tag.name 连接管理
// @tag.description 连接日志、连接统计

package main
