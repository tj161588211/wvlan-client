package auth

import (
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
)

func TestNewTokenManager(t *testing.T) {
	manager := NewTokenManager("test-secret", 24*time.Hour, "test-issuer")
	
	assert.NotNil(t, manager)
	assert.Equal(t, []byte("test-secret"), manager.secretKey)
	assert.Equal(t, 24*time.Hour, manager.expireTime)
	assert.Equal(t, "test-issuer", manager.issuer)
}

func TestGenerateToken(t *testing.T) {
	manager := NewTokenManager("test-secret", 24*time.Hour, "test-issuer")
	
	token, err := manager.GenerateToken("user-123", "testuser", "test@example.com", "user")
	
	assert.NoError(t, err)
	assert.NotEmpty(t, token)
	assert.Contains(t, token, ".") // JWT 包含三个部分
}

func TestParseToken(t *testing.T) {
	manager := NewTokenManager("test-secret", 24*time.Hour, "test-issuer")
	
	// 生成令牌
	tokenString, err := manager.GenerateToken("user-123", "testuser", "test@example.com", "user")
	assert.NoError(t, err)
	
	// 解析令牌
	claims, err := manager.ParseToken(tokenString)
	
	assert.NoError(t, err)
	assert.NotNil(t, claims)
	assert.Equal(t, "user-123", claims.UserID)
	assert.Equal(t, "testuser", claims.Username)
	assert.Equal(t, "test@example.com", claims.Email)
	assert.Equal(t, "user", claims.Role)
}

func TestParseToken_InvalidToken(t *testing.T) {
	manager := NewTokenManager("test-secret", 24*time.Hour, "test-issuer")
	
	// 测试无效令牌
	_, err := manager.ParseToken("invalid.token.here")
	
	assert.Error(t, err)
}

func TestParseToken_WrongSecret(t *testing.T) {
	manager := NewTokenManager("test-secret", 24*time.Hour, "test-issuer")
	
	// 生成令牌
	tokenString, _ := manager.GenerateToken("user-123", "testuser", "test@example.com", "user")
	
	// 使用不同的密钥解析
	wrongManager := NewTokenManager("wrong-secret", 24*time.Hour, "test-issuer")
	_, err := wrongManager.ParseToken(tokenString)
	
	assert.Error(t, err)
}

func TestValidateToken(t *testing.T) {
	manager := NewTokenManager("test-secret", 24*time.Hour, "test-issuer")
	
	// 生成有效令牌
	validToken, _ := manager.GenerateToken("user-123", "testuser", "test@example.com", "user")
	
	// 验证有效令牌
	assert.True(t, manager.ValidateToken(validToken))
	
	// 验证无效令牌
	assert.False(t, manager.ValidateToken("invalid.token"))
}

func TestGetTokenExpiration(t *testing.T) {
	manager := NewTokenManager("test-secret", 24*time.Hour, "test-issuer")
	
	tokenString, _ := manager.GenerateToken("user-123", "testuser", "test@example.com", "user")
	
	expiration, err := manager.GetTokenExpiration(tokenString)
	
	assert.NoError(t, err)
	assert.True(t, expiration.After(time.Now()))
}
