package handlers

import (
	"net/http"
	"strconv"

	"github.com/gin-gonic/gin"
	"github.com/virtual-lan/control-server/internal/models"
)

// DeviceHandler 设备处理器
type DeviceHandler struct{}

// NewDeviceHandler 创建设备处理器
func NewDeviceHandler() *DeviceHandler {
	return &DeviceHandler{}
}

// Register 设备注册
// @Summary 注册设备
// @Description 为用户注册新设备
// @Tags 设备管理
// @Accept json
// @Produce json
// @Security BearerAuth
// @Param request body RegisterDeviceRequest true "设备信息"
// @Success 201 {object} RegisterDeviceResponse
// @Failure 400 {object} map[string]string
// @Failure 401 {object} map[string]string
// @Router /api/v1/devices [post]
func (h *DeviceHandler) Register(c *gin.Context) {
	var req RegisterDeviceRequest
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

	// TODO: 创建设备
	// device := models.NewDevice(userID, req.Name, req.Type, req.OSType, req.Version, req.MAC)
	// if err := deviceService.Create(device); err != nil {
	// 	c.JSON(http.StatusInternalServerError, gin.H{"error": "创建设备失败"})
	// 	return
	// }

	c.JSON(http.StatusCreated, RegisterDeviceResponse{
		Message:  "设备注册成功",
		DeviceID: "mock-device-id",
	})
}

// List 获取设备列表
// @Summary 获取设备列表
// @Description 获取当前用户的所有设备
// @Tags 设备管理
// @Produce json
// @Security BearerAuth
// @Param status query string false "状态过滤 (online/offline)"
// @Success 200 {object} []DeviceResponse
// @Failure 401 {object} map[string]string
// @Router /api/v1/devices [get]
func (h *DeviceHandler) List(c *gin.Context) {
	userID := c.GetString("user_id")
	if userID == "" {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "未授权"})
		return
	}

	status := c.Query("status")

	// TODO: 查询设备列表
	// devices, err := deviceService.ListByUser(userID, status)

	c.JSON(http.StatusOK, []DeviceResponse{
		{
			ID:        "mock-device-id",
			UserID:    userID,
			Name:      "测试设备",
			Type:      "windows",
			OSType:    "Windows 11",
			Version:   "22H2",
			MAC:       "00:1A:2B:3C:4D:5E",
			Status:    "online",
			LastSeen:  "2026-04-15T12:00:00Z",
			CreatedAt: "2026-04-01T10:00:00Z",
		},
	})
}

// Get 获取设备详情
// @Summary 获取设备详情
// @Description 获取指定设备的详细信息
// @Tags 设备管理
// @Produce json
// @Security BearerAuth
// @Param id path string true "设备 ID"
// @Success 200 {object} DeviceResponse
// @Failure 400 {object} map[string]string
// @Failure 404 {object} map[string]string
// @Router /api/v1/devices/{id} [get]
func (h *DeviceHandler) Get(c *gin.Context) {
	deviceID := c.Param("id")
	if deviceID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "设备 ID 不能为空"})
		return
	}

	userID := c.GetString("user_id")
	if userID == "" {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "未授权"})
		return
	}

	// TODO: 查询设备详情
	// device, err := deviceService.FindByID(deviceID, userID)

	c.JSON(http.StatusOK, DeviceResponse{
		ID:        deviceID,
		UserID:    userID,
		Name:      "测试设备",
		Type:      "windows",
		OSType:    "Windows 11",
		Version:   "22H2",
		IP:        "192.168.1.100",
		MAC:       "00:1A:2B:3C:4D:5E",
		Status:    "online",
		LastSeen:  "2026-04-15T12:00:00Z",
		CreatedAt: "2026-04-01T10:00:00Z",
	})
}

// Update 更新设备信息
// @Summary 更新设备信息
// @Description 更新指定设备的信息
// @Tags 设备管理
// @Accept json
// @Produce json
// @Security BearerAuth
// @Param id path string true "设备 ID"
// @Param request body UpdateDeviceRequest true "更新信息"
// @Success 200 {object} map[string]string
// @Failure 400 {object} map[string]string
// @Failure 404 {object} map[string]string
// @Router /api/v1/devices/{id} [put]
func (h *DeviceHandler) Update(c *gin.Context) {
	deviceID := c.Param("id")
	if deviceID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "设备 ID 不能为空"})
		return
	}

	var req UpdateDeviceRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	userID := c.GetString("user_id")
	if userID == "" {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "未授权"})
		return
	}

	// TODO: 更新设备信息
	// if err := deviceService.Update(deviceID, userID, req); err != nil {
	// 	c.JSON(http.StatusNotFound, gin.H{"error": "设备不存在"})
	// 	return
	// }

	c.JSON(http.StatusOK, gin.H{"message": "设备信息更新成功"})
}

// Delete 删除设备
// @Summary 删除设备
// @Description 删除指定设备
// @Tags 设备管理
// @Produce json
// @Security BearerAuth
// @Param id path string true "设备 ID"
// @Success 200 {object} map[string]string
// @Failure 404 {object} map[string]string
// @Router /api/v1/devices/{id} [delete]
func (h *DeviceHandler) Delete(c *gin.Context) {
	deviceID := c.Param("id")
	if deviceID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "设备 ID 不能为空"})
		return
	}

	userID := c.GetString("user_id")
	if userID == "" {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "未授权"})
		return
	}

	// TODO: 删除设备
	// if err := deviceService.Delete(deviceID, userID); err != nil {
	// 	c.JSON(http.StatusNotFound, gin.H{"error": "设备不存在"})
	// 	return
	// }

	c.JSON(http.StatusOK, gin.H{"message": "设备删除成功"})
}

// UpdateStatus 更新设备状态
// @Summary 更新设备状态
// @Description 更新设备的在线状态
// @Tags 设备管理
// @Accept json
// @Produce json
// @Security BearerAuth
// @Param id path string true "设备 ID"
// @Param request body UpdateStatusRequest true "状态信息"
// @Success 200 {object} map[string]string
// @Router /api/v1/devices/{id}/status [put]
func (h *DeviceHandler) UpdateStatus(c *gin.Context) {
	deviceID := c.Param("id")
	if deviceID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "设备 ID 不能为空"})
		return
	}

	var req UpdateStatusRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	userID := c.GetString("user_id")
	if userID == "" {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "未授权"})
		return
	}

	// TODO: 更新设备状态
	// if err := deviceService.UpdateStatus(deviceID, userID, req.Status, req.IP); err != nil {
	// 	c.JSON(http.StatusNotFound, gin.H{"error": "设备不存在"})
	// 	return
	// }

	c.JSON(http.StatusOK, gin.H{"message": "状态更新成功"})
}

// RegisterDeviceRequest 设备注册请求
type RegisterDeviceRequest struct {
	Name      string `json:"name" binding:"required,min=1,max=100"`
	Type      string `json:"type" binding:"required"`
	OSType    string `json:"os_type" binding:"required"`
	Version   string `json:"version"`
	MAC       string `json:"mac" binding:"required"`
	PublicKey string `json:"public_key"`
}

func (r *RegisterDeviceRequest) Validate() error {
	// 验证 MAC 地址格式
	return nil
}

// RegisterDeviceResponse 设备注册响应
type RegisterDeviceResponse struct {
	Message  string `json:"message"`
	DeviceID string `json:"device_id"`
}

// DeviceResponse 设备响应
type DeviceResponse struct {
	ID        string `json:"id"`
	UserID    string `json:"user_id"`
	Name      string `json:"name"`
	Type      string `json:"type"`
	OSType    string `json:"os_type"`
	Version   string `json:"version"`
	IP        string `json:"ip,omitempty"`
	MAC       string `json:"mac"`
	Status    string `json:"status"`
	LastSeen  string `json:"last_seen"`
	CreatedAt string `json:"created_at"`
}

// UpdateDeviceRequest 更新设备请求
type UpdateDeviceRequest struct {
	Name    *string `json:"name,omitempty"`
	Type    *string `json:"type,omitempty"`
	Version *string `json:"version,omitempty"`
}

// UpdateStatusRequest 更新状态请求
type UpdateStatusRequest struct {
	Status string `json:"status" binding:"required"`
	IP     string `json:"ip,omitempty"`
}
