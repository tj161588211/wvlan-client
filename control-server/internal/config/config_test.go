package config

import (
	"os"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
)

func TestLoad(t *testing.T) {
	// 设置测试环境变量
	os.Setenv("APP_ENV", "test")
	os.Setenv("APP_PORT", "9999")
	os.Setenv("DB_HOST", "test-db")
	os.Setenv("DB_PORT", "5433")
	os.Setenv("REDIS_HOST", "test-redis")
	os.Setenv("JWT_SECRET", "test-secret-key")

	defer func() {
		os.Unsetenv("APP_ENV")
		os.Unsetenv("APP_PORT")
		os.Unsetenv("DB_HOST")
		os.Unsetenv("DB_PORT")
		os.Unsetenv("REDIS_HOST")
		os.Unsetenv("JWT_SECRET")
	}()

	cfg, err := Load("")
	
	assert.NoError(t, err)
	assert.NotNil(t, cfg)
	assert.Equal(t, "test", cfg.App.Env)
	assert.Equal(t, 9999, cfg.App.Port)
	assert.Equal(t, "test-db", cfg.DB.Host)
	assert.Equal(t, 5433, cfg.DB.Port)
	assert.Equal(t, "test-redis", cfg.Redis.Host)
	assert.Equal(t, "test-secret-key", cfg.JWT.Secret)
}

func TestLoad_DefaultValues(t *testing.T) {
	// 清除可能存在的测试环境变量
	os.Unsetenv("APP_PORT")
	os.Unsetenv("DB_PORT")
	os.Unsetenv("REDIS_PORT")

	cfg, err := Load("")
	
	assert.NoError(t, err)
	assert.NotNil(t, cfg)
	assert.Equal(t, 8080, cfg.App.Port)
	assert.Equal(t, 5432, cfg.DB.Port)
	assert.Equal(t, 6379, cfg.Redis.Port)
}

func TestPortAddr(t *testing.T) {
	cfg := &Config{}
	cfg.App.Port = 8080
	
	addr := cfg.PortAddr()
	
	assert.Equal(t, ":8080", addr)
}

func TestGRPCPortAddr(t *testing.T) {
	cfg := &Config{}
	cfg.gRPC.Port = 9090
	
	addr := cfg.GRPCPortAddr()
	
	assert.Equal(t, ":9090", addr)
}

func TestConfig_DB_DSN(t *testing.T) {
	cfg := &Config{}
	cfg.DB.Host = "localhost"
	cfg.DB.Port = 5432
	cfg.DB.User = "postgres"
	cfg.DB.Password = "password"
	cfg.DB.DBName = "test_db"
	cfg.DB.SSLMode = "disable"
	
	expectedDSN := "host=localhost port=5432 user=postgres password=password dbname=test_db sslmode=disable"
	assert.Equal(t, expectedDSN, cfg.DB.DSN)
}

func TestConfig_JWT_Expire(t *testing.T) {
	cfg := &Config{}
	cfg.JWT.Expire = 24 * time.Hour
	
	assert.Equal(t, 24*time.Hour, cfg.JWT.Expire)
}
