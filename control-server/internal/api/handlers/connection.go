package handlers

import (
	"net/http"

	"github.com/gin-gonic/gin"
)

// ConnectionHandler 连接日志处理器
type ConnectionHandler struct{}

// NewConnectionHandler 创建连接日志处理器
func NewConnectionHandler() *ConnectionHandler {
	return &ConnectionHandler{}
}

// GetHistory 获取连接历史
// @Summary 获取连接历史
// @Description 获取用户的设备连接历史记录
// @Tags 连接管理
// @Produce json
// @Security BearerAuth
// @Param device_id query string false "设备 ID 过滤"
// @Param start_time query string false "开始时间 (RFC3339)"
// @Param end_time query string false "结束时间 (RFC3339)"
// @Param limit query int false "返回数量限制" default(100)
// @Param offset query int false "偏移量" default(0)
// @Success 200 {object} ConnectionHistoryResponse
// @Failure 401 {object} map[string]string
// @Router /api/v1/connections [get]
func (h *ConnectionHandler) GetHistory(c *gin.Context) {
	userID := c.GetString("user_id")
	if userID == "" {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "未授权"})
		return
	}

	deviceID := c.Query("device_id")
	startTime := c.Query("start_time")
	endTime := c.Query("end_time")
	
	limit := 100
	if l := c.GetInt("limit"); l > 0 {
		limit = l
	}
	offset := c.GetInt("offset")

	// TODO: 查询连接历史
	// logs, total, err := connectionService.GetHistory(userID, deviceID, startTime, endTime, limit, offset)

	c.JSON(http.StatusOK, ConnectionHistoryResponse{
		Total:   2,
		Logs: []ConnectionLogResponse{
			{
				ID:           "mock-log-id-1",
				UserID:       userID,
				DeviceID:     "mock-device-id",
				ConnectionID: "conn-123",
				Action:       "connect",
				Status:       "success",
				ClientIP:     "192.168.1.100",
				Duration:     3600000,
				ConnectionTime: "2026-04-15T10:00:00Z",
			},
			{
				ID:           "mock-log-id-2",
				UserID:       userID,
				DeviceID:     "mock-device-id",
				ConnectionID: "conn-123",
				Action:       "disconnect",
				Status:       "success",
				ClientIP:     "192.168.1.100",
				Duration:     3600000,
				ConnectionTime: "2026-04-15T11:00:00Z",
			},
		},
	})
}

// Record 记录连接事件
// @Summary 记录连接事件
// @Description gRPC 内部使用，记录设备连接事件
// @Tags 连接管理
// @Accept json
// @Produce json
// @Param request body RecordConnectionRequest true "连接事件"
// @Success 200 {object} map[string]string
// @Router /api/v1/connections/record [post]
func (h *ConnectionHandler) Record(c *gin.Context) {
	var req RecordConnectionRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	if err := req.Validate(); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	// TODO: 记录连接日志
	// log := models.NewConnectionLog(req.UserID, req.DeviceID, req.ConnectionID, 
	// 	req.Action, req.Status, req.ClientIP, req.UserAgent)
	// log.ErrorMsg = req.ErrorMessage
	// log.Duration = req.Duration
	// if err := connectionService.Record(log); err != nil {
	// 	c.JSON(http.StatusInternalServerError, gin.H{"error": "记录失败"})
	// 	return
	// }

	c.JSON(http.StatusOK, gin.H{"message": "连接事件已记录"})
}

// GetStats 获取连接统计
// @Summary 获取连接统计
// @Description 获取设备的连接统计信息
// @Tags 连接管理
// @Produce json
// @Security BearerAuth
// @Param device_id path string true "设备 ID"
// @Success 200 {object} ConnectionStatsResponse
// @Failure 404 {object} map[string]string
// @Router /api/v1/connections/stats/{device_id} [get]
func (h *ConnectionHandler) GetStats(c *gin.Context) {
	deviceID := c.Param("device_id")
	if deviceID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "设备 ID 不能为空"})
		return
	}

	userID := c.GetString("user_id")
	if userID == "" {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "未授权"})
		return
	}

	// TODO: 获取连接统计
	// stats, err := connectionService.GetStats(deviceID, userID)

	c.JSON(http.StatusOK, ConnectionStatsResponse{
		DeviceID:        deviceID,
		TotalConnections: 150,
		SuccessfulCount:  145,
		FailedCount:      5,
		AvgDuration:     3600000,
		LastConnectTime: "2026-04-15T10:00:00Z",
		CurrentStatus:   "online",
	})
}

// GetLogsByDevice 获取设备的连接日志
// @Summary 获取设备的连接日志
// @Description 获取指定设备的连接日志
// @Tags 连接管理
// @Produce json
// @Security BearerAuth
// @Param device_id path string true "设备 ID"
// @Param limit query int false "返回数量" default(50)
// @Success 200 {object} ConnectionHistoryResponse
// @Router /api/v1/connections/devices/{device_id} [get]
func (h *ConnectionHandler) GetLogsByDevice(c *gin.Context) {
	deviceID := c.Param("device_id")
	if deviceID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "设备 ID 不能为空"})
		return
	}

	userID := c.GetString("user_id")
	if userID == "" {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "未授权"})
		return
	}

	limit := c.GetInt("limit")
	if limit <= 0 {
		limit = 50
	}

	// TODO: 查询连接日志
	// logs, err := connectionService.GetByDevice(deviceID, userID, limit)

	c.JSON(http.StatusOK, ConnectionHistoryResponse{
		Total: len([]ConnectionLogResponse{}),
		Logs:  []ConnectionLogResponse{},
	})
}

// RecordConnectionRequest 记录连接请求
type RecordConnectionRequest struct {
	UserID      string `json:"user_id" binding:"required"`
	DeviceID    string `json:"device_id" binding:"required"`
	ConnectionID string `json:"connection_id" binding:"required"`
	Action      string `json:"action" binding:"required,oneof=connect disconnect reconnect"`
	Status      string `json:"status" binding:"required,oneof=success failed"`
	ClientIP    string `json:"client_ip"`
	UserAgent   string `json:"user_agent"`
	ErrorMessage string `json:"error_message"`
	Duration    int64  `json:"duration"`
}

func (r *RecordConnectionRequest) Validate() error {
	return nil
}

// ConnectionLogResponse 连接日志响应
type ConnectionLogResponse struct {
	ID             string `json:"id"`
	UserID         string `json:"user_id"`
	DeviceID       string `json:"device_id"`
	ConnectionID   string `json:"connection_id"`
	Action         string `json:"action"`
	Status         string `json:"status"`
	ClientIP       string `json:"client_ip"`
	UserAgent      string `json:"user_agent,omitempty"`
	ErrorMessage   string `json:"error_message,omitempty"`
	Duration       int64  `json:"duration"`
	ConnectionTime string `json:"connection_time"`
}

// ConnectionHistoryResponse 连接历史响应
type ConnectionHistoryResponse struct {
	Total int                     `json:"total"`
	Logs  []ConnectionLogResponse `json:"logs"`
}

// ConnectionStatsResponse 连接统计响应
type ConnectionStatsResponse struct {
	DeviceID         string `json:"device_id"`
	TotalConnections int    `json:"total_connections"`
	SuccessfulCount  int    `json:"successful_count"`
	FailedCount      int    `json:"failed_count"`
	AvgDuration      int64  `json:"avg_duration"`
	LastConnectTime  string `json:"last_connect_time"`
	CurrentStatus    string `json:"current_status"`
}
