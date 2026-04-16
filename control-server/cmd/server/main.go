package main

import (
	"context"
	"fmt"
	"net/http"
	"os"
	"os/signal"
	"syscall"
	"time"

	"github.com/virtual-lan/control-server/internal/api"
	"github.com/virtual-lan/control-server/internal/config"
	"github.com/virtual-lan/control-server/internal/database"
	"github.com/virtual-lan/control-server/internal/grpc"
	"github.com/virtual-lan/control-server/internal/redis"
	"github.com/virtual-lan/control-server/pkg/logger"
	"go.uber.org/zap"
)

// Version 应用版本
var Version = "1.0.0"

func main() {
	// 加载配置
	cfg, err := config.Load(".env")
	if err != nil {
		fmt.Printf("加载配置失败：%v\n", err)
		os.Exit(1)
	}

	// 初始化日志
	logCfg := logger.Config{
		Level:  cfg.Log.Level,
		Format: cfg.Log.Format,
	}
	if err := logger.Init(logCfg); err != nil {
		fmt.Printf("初始化日志失败：%v\n", err)
		os.Exit(1)
	}
	defer logger.Sync()

	zap.L().Info("控制服务器启动",
		zap.String("version", Version),
		zap.String("env", cfg.App.Env),
	)

	// 初始化数据库
	dbCfg := database.Config{
		DSN:             cfg.DB.DSN,
		MaxOpenConns:    cfg.DB.MaxOpenConns,
		MaxIdleConns:    cfg.DB.MaxIdleConns,
		ConnMaxLifetime: cfg.DB.ConnMaxLifetime.String(),
		Debug:           cfg.App.Debug,
	}
	if _, err := database.InitDB(dbCfg); err != nil {
		zap.L().Fatal("初始化数据库失败", zap.Error(err))
	}
	defer database.Close()
	zap.L().Info("数据库连接成功")

	// 初始化 Redis
	redisCfg := redis.Config{
		Host:         cfg.Redis.Host,
		Port:         cfg.Redis.Port,
		Password:     cfg.Redis.Password,
		DB:           cfg.Redis.DB,
		PoolSize:     cfg.Redis.PoolSize,
		DialTimeout:  cfg.Redis.DialTimeout,
		ReadTimeout:  cfg.Redis.ReadTimeout,
		WriteTimeout: cfg.Redis.WriteTimeout,
	}
	if _, err := redis.InitRedis(redisCfg); err != nil {
		zap.L().Warn("Redis 连接失败，将继续运行", zap.Error(err))
	} else {
		defer redis.Close()
		zap.L().Info("Redis 连接成功")
	}

	// 启动 gRPC 服务器
	grpcServer := grpc.NewServer(cfg)
	go func() {
		if err := grpcServer.Start(); err != nil {
			zap.L().Error("gRPC 服务器启动失败", zap.Error(err))
		}
	}()
	defer grpcServer.Stop()

	// 设置 RESTful API 路由器
	router := api.SetupRouter(cfg)

	// 创建 HTTP 服务器
	srv := &http.Server{
		Addr:         cfg.PortAddr(),
		Handler:      router,
		ReadTimeout:  15 * time.Second,
		WriteTimeout: 15 * time.Second,
		IdleTimeout:  60 * time.Second,
	}

	// 优雅关闭
	go func() {
		zap.L().Info("HTTP 服务器启动", zap.String("address", cfg.PortAddr()))
		if err := srv.ListenAndServe(); err != nil && err != http.ErrServerClosed {
			zap.L().Fatal("HTTP 服务器启动失败", zap.Error(err))
		}
	}()

	// 等待中断信号
	quit := make(chan os.Signal, 1)
	signal.Notify(quit, syscall.SIGINT, syscall.SIGTERM)
	<-quit

	zap.L().Info("正在关闭服务器...")

	// 设置超时时间
	ctx, cancel := context.WithTimeout(context.Background(), 30*time.Second)
	defer cancel()

	// 关闭 HTTP 服务器
	if err := srv.Shutdown(ctx); err != nil {
		zap.L().Fatal("服务器强制关闭", zap.Error(err))
	}

	zap.L().Info("服务器已关闭")
}
