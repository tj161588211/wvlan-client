package api

import (
	"net/http"

	"github.com/gin-gonic/gin"
	swaggerFiles "github.com/swaggo/files"
	ginSwagger "github.com/swaggo/gin-swagger"

	"github.com/virtual-lan/control-server/internal/api/handlers"
	"github.com/virtual-lan/control-server/internal/api/middleware"
	"github.com/virtual-lan/control-server/internal/config"
)

// SetupRouter 设置路由器
func SetupRouter(cfg *config.Config) *gin.Engine {
	router := gin.New()

	// 使用自定义的 Recovery 中间件代替 gin.Recovery
	router.Use(middleware.Recovery())
	router.Use(middleware.Logger())
	router.Use(middleware.CORS())
	router.Use(middleware.RequestID())

	// 健康检查接口
	router.GET("/health", func(c *gin.Context) {
		c.JSON(http.StatusOK, gin.H{
			"status":  "ok",
			"service": cfg.App.Name,
			"version": "1.0.0",
		})
	})

	// API v1 路由组
	v1 := router.Group("/api/v1")
	{
		// 公开路由（无需认证）
		public := v1.Group("")
		{
			// 用户相关公开路由
			users := public.Group("/users")
			{
				users.POST("/register", handlers.NewUserHandler().Register)
				users.POST("/login", handlers.NewUserHandler().Login)
			}
		}

		// 需要认证的路由
		auth := v1.Group("")
		auth.Use(middleware.NewJWTMiddleware(
			cfg.JWT.Secret,
			int(cfg.JWT.Expire.Seconds()),
			cfg.JWT.Issuer,
		).Auth())
		{
			// 用户资料
			auth.GET("/users/profile", handlers.NewUserHandler().GetProfile)
			auth.PUT("/users/profile", handlers.NewUserHandler().UpdateProfile)
			auth.POST("/users/change-password", handlers.NewUserHandler().ChangePassword)

			// 设备管理
			devices := auth.Group("/devices")
			{
				devices.GET("", handlers.NewDeviceHandler().List)
				devices.POST("", handlers.NewDeviceHandler().Register)
				devices.GET("/:id", handlers.NewDeviceHandler().Get)
				devices.PUT("/:id", handlers.NewDeviceHandler().Update)
				devices.DELETE("/:id", handlers.NewDeviceHandler().Delete)
				devices.PUT("/:id/status", handlers.NewDeviceHandler().UpdateStatus)
			}

			// 配置管理
			configs := auth.Group("/configs")
			{
				configs.GET("", handlers.NewConfigHandler().List)
				configs.POST("", handlers.NewConfigHandler().Create)
				configs.GET("/:id", handlers.NewConfigHandler().Get)
				configs.PUT("/:id", handlers.NewConfigHandler().Update)
				configs.DELETE("/:id", handlers.NewConfigHandler().Delete)
				configs.POST("/:id/publish", handlers.NewConfigHandler().Publish)
			}

			// 连接管理
			connections := auth.Group("/connections")
			{
				connections.GET("", handlers.NewConnectionHandler().GetHistory)
				connections.GET("/stats/:device_id", handlers.NewConnectionHandler().GetStats)
				connections.GET("/devices/:device_id", handlers.NewConnectionHandler().GetLogsByDevice)
				connections.POST("/record", handlers.NewConnectionHandler().Record)
			}
		}
	}

	// Swagger 文档
	if cfg.Swagger.Enabled {
		router.GET("/swagger/*any", ginSwagger.WrapHandler(swaggerFiles.Handler))
	}

	// 404 处理器
	router.NoRoute(func(c *gin.Context) {
		c.JSON(http.StatusNotFound, gin.H{
			"error": "接口不存在",
		})
	})

	return router
}
