package auth

import (
	"errors"
	"time"

	"github.com/golang-jwt/jwt/v5"
)

// Claims JWT 声明结构
type Claims struct {
	UserID   string `json:"user_id"`
	Username string `json:"username"`
	Email    string `json:"email"`
	Role     string `json:"role"`
	jwt.RegisteredClaims
}

// TokenManager JWT 令牌管理器
type TokenManager struct {
	secretKey  []byte
	expireTime time.Duration
	issuer     string
}

// NewTokenManager 创建新的令牌管理器
func NewTokenManager(secretKey string, expireTime time.Duration, issuer string) *TokenManager {
	return &TokenManager{
		secretKey:  []byte(secretKey),
		expireTime: expireTime,
		issuer:     issuer,
	}
}

// GenerateToken 生成 JWT 令牌
func (m *TokenManager) GenerateToken(userID, username, email, role string) (string, error) {
	now := time.Now()
	expireAt := now.Add(m.expireTime)

	claims := Claims{
		UserID:   userID,
		Username: username,
		Email:    email,
		Role:     role,
		RegisteredClaims: jwt.RegisteredClaims{
			Issuer:    m.issuer,
			Subject:   userID,
			ExpiresAt: jwt.NewNumericDate(expireAt),
			IssuedAt:  jwt.NewNumericDate(now),
			NotBefore: jwt.NewNumericDate(now),
			ID:        "", // 可以由 UUID 生成
		},
	}

	token := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)
	tokenString, err := token.SignedString(m.secretKey)
	if err != nil {
		return "", err
	}

	return tokenString, nil
}

// ParseToken 解析并验证 JWT 令牌
func (m *TokenManager) ParseToken(tokenString string) (*Claims, error) {
	token, err := jwt.ParseWithClaims(tokenString, &Claims{}, func(token *jwt.Token) (interface{}, error) {
		// 验证签名算法
		if _, ok := token.Method.(*jwt.SigningMethodHMAC); !ok {
			return nil, errors.New("unexpected signing method")
		}
		return m.secretKey, nil
	})

	if err != nil {
		return nil, err
	}

	claims, ok := token.Claims.(*Claims)
	if !ok || !token.Valid {
		return nil, errors.New("invalid token")
	}

	return claims, nil
}

// ValidateToken 验证令牌是否有效
func (m *TokenManager) ValidateToken(tokenString string) bool {
	_, err := m.ParseToken(tokenString)
	return err == nil
}

// GetTokenExpiration 获取令牌过期时间
func (m *TokenManager) GetTokenExpiration(tokenString string) (time.Time, error) {
	claims, err := m.ParseToken(tokenString)
	if err != nil {
		return time.Time{}, err
	}
	return claims.ExpiresAt.Time, nil
}
