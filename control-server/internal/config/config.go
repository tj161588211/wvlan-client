package config

import (
	"fmt"
	"os"
	"strconv"
	"time"

	"github.com/joho/godotenv"
	"go.uber.org/zap"
)

// Config 应用配置结构
type Config struct {
	App      AppConfig
	DB       DBConfig
	Redis    RedisConfig
	JWT      JWTConfig
	gRPC     gRPCConfig
	Log      LogConfig
	Swagger  SwaggerConfig
}

// AppConfig 应用配置
type AppConfig struct {
	Env   string
	Port  int
	Name  string
	Debug bool
}

// DBConfig 数据库配置
type DBConfig struct {
	Host            string
	Port            int
	User            string
	Password        string
	DBName          string
	SSLMode         string
	MaxOpenConns    int
	MaxIdleConns    int
	ConnMaxLifetime time.Duration
	DSN             string // 连接字符串，由其他字段生成
}

// RedisConfig Redis 配置
type RedisConfig struct {
	Host        string
	Port        int
	Password    string
	DB          int
	PoolSize    int
	DialTimeout time.Duration
	ReadTimeout time.Duration
	WriteTimeout time.Duration
}

// JWTConfig JWT 配置
type JWTConfig struct {
	Secret  string
	Expire  time.Duration
	Issuer  string
}

// gRPCConfig gRPC 配置
type gRPCConfig struct {
	Port           int
	MaxSendMsgSize int
	MaxRecvMsgSize int
}

// LogConfig 日志配置
type LogConfig struct {
	Level  string
	Format string
}

// SwaggerConfig Swagger 配置
type SwaggerConfig struct {
	Enabled bool
}

// Load 加载配置
func Load(envPath string) (*Config, error) {
	// 加载 .env 文件
	if envPath != "" {
		if err := godotenv.Load(envPath); err != nil {
			// 如果文件不存在，继续执行（生产环境可能不需要 .env 文件）
			if !os.IsNotExist(err) {
				return nil, fmt.Errorf("加载环境变量文件失败: %w", err)
			}
		}
	}

	cfg := &Config{}

	// 加载应用配置
	if err := cfg.loadAppConfig(); err != nil {
		return nil, err
	}

	// 加载数据库配置
	if err := cfg.loadDBConfig(); err != nil {
		return nil, err
	}

	// 加载 Redis 配置
	if err := cfg.loadRedisConfig(); err != nil {
		return nil, err
	}

	// 加载 JWT 配置
	if err := cfg.loadJWTConfig(); err != nil {
		return nil, err
	}

	// 加载 gRPC 配置
	if err := cfg.loadGRPCConfig(); err != nil {
		return nil, err
	}

	// 加载日志配置
	if err := cfg.loadLogConfig(); err != nil {
		return nil, err
	}

	// 加载 Swagger 配置
	if err := cfg.loadSwaggerConfig(); err != nil {
		return nil, err
	}

	return cfg, nil
}

// loadAppConfig 加载应用配置
func (c *Config) loadAppConfig() error {
	c.App.Env = getEnv("APP_ENV", "development")
	c.App.Name = getEnv("APP_NAME", "control-server")
	
	portStr := getEnv("APP_PORT", "8080")
	port, err := strconv.Atoi(portStr)
	if err != nil {
		return fmt.Errorf("无效的 APP_PORT: %w", err)
	}
	c.App.Port = port
	
	c.App.Debug = c.App.Env == "development"
	return nil
}

// loadDBConfig 加载数据库配置
func (c *Config) loadDBConfig() error {
	c.DB.Host = getEnv("DB_HOST", "localhost")
	c.DB.User = getEnv("DB_USER", "postgres")
	c.DB.Password = getEnv("DB_PASSWORD", "postgres")
	c.DB.DBName = getEnv("DB_NAME", "control_server")
	c.DB.SSLMode = getEnv("DB_SSL_MODE", "disable")
	
	portStr := getEnv("DB_PORT", "5432")
	port, err := strconv.Atoi(portStr)
	if err != nil {
		return fmt.Errorf("无效的 DB_PORT: %w", err)
	}
	c.DB.Port = port

	maxOpenConns, _ := strconv.Atoi(getEnv("DB_MAX_OPEN_CONNS", "25"))
	c.DB.MaxOpenConns = maxOpenConns

	maxIdleConns, _ := strconv.Atoi(getEnv("DB_MAX_IDLE_CONNS", "5"))
	c.DB.MaxIdleConns = maxIdleConns

	connLifetimeStr := getEnv("DB_CONN_MAX_LIFETIME", "5m")
	connLifetime, err := time.ParseDuration(connLifetimeStr)
	if err != nil {
		return fmt.Errorf("无效的 DB_CONN_MAX_LIFETIME: %w", err)
	}
	c.DB.ConnMaxLifetime = connLifetime

	// 生成 DSN
	c.DB.DSN = fmt.Sprintf(
		"host=%s port=%d user=%s password=%s dbname=%s sslmode=%s",
		c.DB.Host, c.DB.Port, c.DB.User, c.DB.Password, c.DB.DBName, c.DB.SSLMode,
	)
	return nil
}

// loadRedisConfig 加载 Redis 配置
func (c *Config) loadRedisConfig() error {
	c.Redis.Host = getEnv("REDIS_HOST", "localhost")
	c.Redis.Port, _ = strconv.Atoi(getEnv("REDIS_PORT", "6379"))
	c.Redis.Password = getEnv("REDIS_PASSWORD", "")
	c.Redis.DB, _ = strconv.Atoi(getEnv("REDIS_DB", "0"))
	c.Redis.PoolSize, _ = strconv.Atoi(getEnv("REDIS_POOL_SIZE", "10"))

	dialTimeout, _ := time.ParseDuration(getEnv("REDIS_DIAL_TIMEOUT", "5s"))
	c.Redis.DialTimeout = dialTimeout

	readTimeout, _ := time.ParseDuration(getEnv("REDIS_READ_TIMEOUT", "3s"))
	c.Redis.ReadTimeout = readTimeout

	writeTimeout, _ := time.ParseDuration(getEnv("REDIS_WRITE_TIMEOUT", "3s"))
	c.Redis.WriteTimeout = writeTimeout

	return nil
}

// loadJWTConfig 加载 JWT 配置
func (c *Config) loadJWTConfig() error {
	c.JWT.Secret = getEnv("JWT_SECRET", "default-secret-key-change-in-production")
	c.JWT.Issuer = getEnv("JWT_ISSUER", "control-server")
	
	expireHours, _ := strconv.ParseFloat(getEnv("JWT_EXPIRE", "24"), 64)
	c.JWT.Expire = time.Duration(expireHours * float64(time.Hour))
	
	if c.JWT.Secret == "default-secret-key-change-in-production" {
		zap.L().Warn("使用默认 JWT_SECRET，请在生产环境中修改！")
	}
	return nil
}

// loadGRPCConfig 加载 gRPC 配置
func (c *Config) loadGRPCConfig() error {
	portStr := getEnv("GRPC_PORT", "9090")
	port, err := strconv.Atoi(portStr)
	if err != nil {
		return fmt.Errorf("无效的 GRPC_PORT: %w", err)
	}
	c.gRPC.Port = port

	c.gRPC.MaxSendMsgSize, _ = strconv.Atoi(getEnv("GRPC_MAX_SEND_MSG_SIZE", "10485760"))
	c.gRPC.MaxRecvMsgSize, _ = strconv.Atoi(getEnv("GRPC_MAX_RECV_MSG_SIZE", "10485760"))

	return nil
}

// loadLogConfig 加载日志配置
func (c *Config) loadLogConfig() error {
	c.Log.Level = getEnv("LOG_LEVEL", "info")
	c.Log.Format = getEnv("LOG_FORMAT", "json")
	return nil
}

// loadSwaggerConfig 加载 Swagger 配置
func (c *Config) loadSwaggerConfig() error {
	swaggerEnabled := getEnv("SWAGGER_ENABLED", "true")
	c.Swagger.Enabled = swaggerEnabled == "true"
	return nil
}

// PortAddr 返回监听地址
func (c *Config) PortAddr() string {
	return fmt.Sprintf(":%d", c.App.Port)
}

// GRPCPortAddr 返回 gRPC 监听地址
func (c *Config) GRPCPortAddr() string {
	return fmt.Sprintf(":%d", c.gRPC.Port)
}

// getEnv 获取环境变量，提供默认值
func getEnv(key, defaultValue string) string {
	if value, exists := os.LookupEnv(key); exists {
		return value
	}
	return defaultValue
}
