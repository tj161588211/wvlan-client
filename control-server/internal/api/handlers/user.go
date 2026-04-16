package handlers

import (
	"net/http"
	"strconv"

	"github.com/gin-gonic/gin"
	"github.com/virtual-lan/control-server/internal/models"
	"go.uber.org/zap"
)

// UserHandler 用户处理器
type UserHandler struct{}

// NewUserHandler 创建用户处理器
func NewUserHandler() *UserHandler {
	return &UserHandler{}
}

// Register 用户注册
// @Summary 用户注册
// @Description 创建新用户账户
// @Tags 用户管理
// @Accept json
// @Produce json
// @Param request body RegisterRequest true "注册信息"
// @Success 201 {object} map[string]string
// @Failure 400 {object} map[string]string
// @Failure 409 {object} map[string]string
// @Router /api/v1/users/register [post]
func (h *UserHandler) Register(c *gin.Context) {
	var req RegisterRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	// 验证输入
	if err := req.Validate(); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	// TODO: 调用服务层创建用户
	// user := models.NewUser(req.Username, req.Email, hashedPassword, "user")
	// if err := userService.Create(user); err != nil {
	// 	if err == ErrUserExists {
	// 		c.JSON(http.StatusConflict, gin.H{"error": "用户已存在"})
	// 		return
	// 	}
	// 	c.JSON(http.StatusInternalServerError, gin.H{"error": "创建用户失败"})
	// 	return
	// }

	c.JSON(http.StatusCreated, gin.H{
		"message": "用户注册成功",
		"user_id": "mock-user-id",
	})
}

// Login 用户登录
// @Summary 用户登录
// @Description 用户登录并获取 JWT 令牌
// @Tags 用户管理
// @Accept json
// @Produce json
// @Param request body LoginRequest true "登录信息"
// @Success 200 {object} LoginResponse
// @Failure 400 {object} map[string]string
// @Failure 401 {object} map[string]string
// @Router /api/v1/users/login [post]
func (h *UserHandler) Login(c *gin.Context) {
	var req LoginRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	// 验证输入
	if err := req.Validate(); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	// TODO: 验证用户凭据并生成 JWT
	// user, err := userService.FindByUsername(req.Username)
	// if err != nil || !verifyPassword(user.Password, req.Password) {
	// 	c.JSON(http.StatusUnauthorized, gin.H{"error": "用户名或密码错误"})
	// 	return
	// }
	//
	// token, err := tokenManager.GenerateToken(user.ID, user.Username, user.Email, user.Role)
	// if err != nil {
	// 	c.JSON(http.StatusInternalServerError, gin.H{"error": "生成令牌失败"})
	// 	return
	// }

	c.JSON(http.StatusOK, LoginResponse{
		Message: "登录成功",
		Token:   "mock-jwt-token",
		User: UserInfo{
			ID:       "mock-user-id",
			Username: req.Username,
			Email:    "user@example.com",
			Role:     "user",
		},
	})
}

// GetProfile 获取用户资料
// @Summary 获取用户资料
// @Description 获取当前登录用户的资料信息
// @Tags 用户管理
// @Produce json
// @Security BearerAuth
// @Success 200 {object} UserInfo
// @Failure 401 {object} map[string]string
// @Router /api/v1/users/profile [get]
func (h *UserHandler) GetProfile(c *gin.Context) {
	// 从上下文获取用户信息（由 JWT 中间件设置）
	userID := c.GetString("user_id")
	username := c.GetString("username")

	if userID == "" || username == "" {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "未授权"})
		return
	}

	// TODO: 从数据库获取用户信息
	// user, err := userService.FindByID(userID)

	c.JSON(http.StatusOK, UserInfo{
		ID:       userID,
		Username: username,
		Email:    "user@example.com",
		Role:     "user",
	})
}

// UpdateProfile 更新用户资料
// @Summary 更新用户资料
// @Description 更新当前用户的资料信息
// @Tags 用户管理
// @Accept json
// @Produce json
// @Security BearerAuth
// @Param request body UpdateProfileRequest true "更新信息"
// @Success 200 {object} map[string]string
// @Failure 400 {object} map[string]string
// @Failure 401 {object} map[string]string
// @Router /api/v1/users/profile [put]
func (h *UserHandler) UpdateProfile(c *gin.Context) {
	var req UpdateProfileRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	userID := c.GetString("user_id")
	if userID == "" {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "未授权"})
		return
	}

	// TODO: 更新用户信息
	// if err := userService.Update(userID, req); err != nil {
	// 	c.JSON(http.StatusInternalServerError, gin.H{"error": "更新失败"})
	// 	return
	// }

	c.JSON(http.StatusOK, gin.H{"message": "资料更新成功"})
}

// ChangePassword 修改密码
// @Summary 修改密码
// @Description 修改当前用户的密码
// @Tags 用户管理
// @Accept json
// @Produce json
// @Security BearerAuth
// @Param request body ChangePasswordRequest true "密码信息"
// @Success 200 {object} map[string]string
// @Failure 400 {object} map[string]string
// @Failure 401 {object} map[string]string
// @Router /api/v1/users/change-password [post]
func (h *UserHandler) ChangePassword(c *gin.Context) {
	var req ChangePasswordRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	if err := req.Validate(); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	userID := c.GetString("user_id")
	if userID == "" {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "未授权"})
		return
	}

	// TODO: 验证旧密码并更新
	// user, err := userService.FindByID(userID)
	// if err != nil || !verifyPassword(user.Password, req.OldPassword) {
	// 	c.JSON(http.StatusBadRequest, gin.H{"error": "旧密码错误"})
	// 	return
	// }
	//
	// hashedPassword, _ := hashPassword(req.NewPassword)
	// if err := userService.ChangePassword(userID, hashedPassword); err != nil {
	// 	c.JSON(http.StatusInternalServerError, gin.H{"error": "修改密码失败"})
	// 	return
	// }

	c.JSON(http.StatusOK, gin.H{"message": "密码修改成功"})
}

// RegisterRequest 注册请求
type RegisterRequest struct {
	Username string `json:"username" binding:"required,min=3,max=50"`
	Email    string `json:"email" binding:"required,email"`
	Password string `json:"password" binding:"required,min=8,max=100"`
}

func (r *RegisterRequest) Validate() error {
	// 可以在这里添加额外的验证逻辑
	return nil
}

// LoginRequest 登录请求
type LoginRequest struct {
	Username string `json:"username" binding:"required"`
	Password string `json:"password" binding:"required"`
}

func (r *LoginRequest) Validate() error {
	return nil
}

// LoginResponse 登录响应
type LoginResponse struct {
	Message string   `json:"message"`
	Token   string   `json:"token"`
	User    UserInfo `json:"user"`
}

// UserInfo 用户信息
type UserInfo struct {
	ID       string `json:"id"`
	Username string `json:"username"`
	Email    string `json:"email"`
	Role     string `json:"role"`
}

// UpdateProfileRequest 更新资料请求
type UpdateProfileRequest struct {
	Email *string `json:"email,omitempty"`
}

// ChangePasswordRequest 修改密码请求
type ChangePasswordRequest struct {
	OldPassword string `json:"old_password" binding:"required"`
	NewPassword string `json:"new_password" binding:"required,min=8,max=100"`
}

func (r *ChangePasswordRequest) Validate() error {
	if r.OldPassword == r.NewPassword {
		return nil
	}
	return nil
}
