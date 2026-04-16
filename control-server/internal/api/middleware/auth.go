package middleware

import (
	"net/http"
	"strings"

	"github.com/gin-gonic/gin"
	"github.com/golang-jwt/jwt/v5"
	"github.com/virtual-lan/control-server/pkg/auth"
)

// JWTConfig JWT 中间件配置
type JWTConfig struct {
	SecretKey  string
	ExpireTime int
	Issuer     string
}

// JWTMiddleware JWT 认证中间件
type JWTMiddleware struct {
	tokenManager *auth.TokenManager
}

// NewJWTMiddleware 创建 JWT 中间件
func NewJWTMiddleware(secretKey string, expireTime int, issuer string) *JWTMiddleware {
	return &JWTMiddleware{
		tokenManager: auth.NewTokenManager(secretKey, expireTime, issuer),
	}
}

// Auth 认证中间件
// 验证 JWT 令牌，并将用户信息注入到上下文
func (m *JWTMiddleware) Auth() gin.HandlerFunc {
	return func(c *gin.Context) {
		authHeader := c.GetHeader("Authorization")
		if authHeader == "" {
			c.JSON(http.StatusUnauthorized, gin.H{
				"error": "缺少 Authorization 头",
			})
			c.Abort()
			return
		}

		// 提取 Bearer 令牌
		parts := strings.SplitN(authHeader, " ", 2)
		if !(len(parts) == 2 && parts[0] == "Bearer") {
			c.JSON(http.StatusUnauthorized, gin.H{
				"error": "Authorization 头格式错误",
			})
			c.Abort()
			return
		}

		tokenString := parts[1]

		// 解析并验证令牌
		claims, err := m.tokenManager.ParseToken(tokenString)
		if err != nil {
			if err == jwt.ErrTokenExpired {
				c.JSON(http.StatusUnauthorized, gin.H{
					"error": "令牌已过期",
				})
			} else {
				c.JSON(http.StatusUnauthorized, gin.H{
					"error": "无效的令牌",
				})
			}
			c.Abort()
			return
		}

		// 将用户信息注入到上下文
		c.Set("user_id", claims.UserID)
		c.Set("username", claims.Username)
		c.Set("email", claims.Email)
		c.Set("role", claims.Role)

		c.Next()
	}
}

// AdminOnly 管理员权限中间件
// 仅允许管理员访问
func (m *JWTMiddleware) AdminOnly() gin.HandlerFunc {
	return func(c *gin.Context) {
		role := c.GetString("role")
		if role != "admin" {
			c.JSON(http.StatusForbidden, gin.H{
				"error": "需要管理员权限",
			})
			c.Abort()
			return
		}
		c.Next()
	}
}

// OptionalAuth 可选认证中间件
// 如果提供了令牌则验证，否则继续（用于公开接口）
func (m *JWTMiddleware) OptionalAuth() gin.HandlerFunc {
	return func(c *gin.Context) {
		authHeader := c.GetHeader("Authorization")
		if authHeader == "" {
			c.Next()
			return
		}

		parts := strings.SplitN(authHeader, " ", 2)
		if !(len(parts) == 2 && parts[0] == "Bearer") {
			c.Next()
			return
		}

		tokenString := parts[1]
		claims, err := m.tokenManager.ParseToken(tokenString)
		if err == nil {
			c.Set("user_id", claims.UserID)
			c.Set("username", claims.Username)
			c.Set("email", claims.Email)
			c.Set("role", claims.Role)
		}

		c.Next()
	}
}

// GetUserID 从上下文中获取用户 ID
func GetUserID(c *gin.Context) string {
	return c.GetString("user_id")
}

// GetUsername 从上下文中获取用户名
func GetUsername(c *gin.Context) string {
	return c.GetString("username")
}

// GetRole 从上下文中获取用户角色
func GetRole(c *gin.Context) string {
	return c.GetString("role")
}

// IsAdmin 检查用户是否为管理员
func IsAdmin(c *gin.Context) bool {
	return GetRole(c) == "admin"
}
