package models

import (
	"time"

	"github.com/google/uuid"
)

// User 用户模型
type User struct {
	ID        string    `json:"id" gorm:"type:uuid;primary_key"`
	Username  string    `json:"username" gorm:"uniqueIndex;not null"`
	Email     string    `json:"email" gorm:"uniqueIndex;not null"`
	Password  string    `json:"-" gorm:"not null"`
	Role      string    `json:"role" gorm:"default:'user'"` // user, admin
	Status    string    `json:"status" gorm:"default:'active'"` // active, inactive, banned
	CreatedAt time.Time `json:"created_at"`
	UpdatedAt time.Time `json:"updated_at"`
	Devices   []Device  `json:"devices,omitempty" gorm:"foreignKey:UserID"`
}

// TableName 指定表名
func (User) TableName() string {
	return "users"
}

// NewUser 创建新用户
func NewUser(username, email, password, role string) *User {
	return &User{
		ID:       uuid.New().String(),
		Username: username,
		Email:    email,
		Password: password, // 应该已经是加密后的密码
		Role:     role,
		Status:   "active",
	}
}

// Device 设备模型
type Device struct {
	ID           string    `json:"id" gorm:"type:uuid;primary_key"`
	UserID       string    `json:"user_id" gorm:"type:uuid;not null;index"`
	Name         string    `json:"name" gorm:"not null"`
	Type         string    `json:"type"`              // windows, linux, macos, other
	OSType       string    `json:"os_type"`           // windows 10, windows 11, etc.
	Version      string    `json:"version"`
	IP           string    `json:"ip" gorm:"index"`
	MAC          string    `json:"mac" gorm:"uniqueIndex;not null"`
	Status       string    `json:"status" gorm:"default:'offline'"` // online, offline
	LastSeen     time.Time `json:"last_seen"`
	PublicKey    string    `json:"public_key"`       // 用于安全通信的公钥
	RegisteredAt time.Time `json:"registered_at"`
	UpdatedAt    time.Time `json:"updated_at"`
	User         User      `json:"-" gorm:"foreignKey:UserID"`
	Configs      []Config  `json:"configs,omitempty" gorm:"foreignKey:DeviceID"`
}

// TableName 指定表名
func (Device) TableName() string {
	return "devices"
}

// NewDevice 创建设备
func NewDevice(userID, name, deviceType, osType, version, mac string) *Device {
	now := time.Now()
	return &Device{
		ID:           uuid.New().String(),
		UserID:       userID,
		Name:         name,
		Type:         deviceType,
		OSType:       osType,
		Version:      version,
		MAC:          mac,
		Status:       "offline",
		LastSeen:     now,
		RegisteredAt: now,
		UpdatedAt:    now,
	}
}

// Config 配置模型
type Config struct {
	ID          string    `json:"id" gorm:"type:uuid;primary_key"`
	UserID      string    `json:"user_id" gorm:"type:uuid;not null;index"`
	DeviceID    string    `json:"device_id" gorm:"type:uuid;index"`
	Name        string    `json:"name" gorm:"not null"`
	Description string    `json:"description"`
	Type        string    `json:"type"` // network, security, general
	Content     string    `json:"content" gorm:"type:text"` // JSON 格式的配置内容
	Version     int       `json:"version" gorm:"default:1"`
	IsActive    bool      `json:"is_active" gorm:"default:true"`
	CreatedBy   string    `json:"created_by" gorm:"type:uuid"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
	PublishedAt *time.Time `json:"published_at"`
}

// TableName 指定表名
func (Config) TableName() string {
	return "configs"
}

// NewConfig 创建配置
func NewConfig(userID, deviceID, name, configType, content string) *Config {
	return &Config{
		ID:          uuid.New().String(),
		UserID:      userID,
		DeviceID:    deviceID,
		Name:        name,
		Type:        configType,
		Content:     content,
		Version:     1,
		IsActive:    true,
		CreatedBy:   userID,
		CreatedAt:   time.Now(),
		UpdatedAt:   time.Now(),
		Description: "",
	}
}

// ConnectionLog 连接日志模型
type ConnectionLog struct {
	ID             string    `json:"id" gorm:"type:uuid;primary_key"`
	UserID         string    `json:"user_id" gorm:"type:uuid;not null;index"`
	DeviceID       string    `json:"device_id" gorm:"type:uuid;not null;index"`
	ConnectionID   string    `json:"connection_id" gorm:"index"`
	Action         string    `json:"action"` // connect, disconnect, reconnect
	Status         string    `json:"status"` // success, failed
	ClientIP       string    `json:"client_ip"`
	UserAgent      string    `json:"user_agent"`
	ErrorMsg       string    `json:"error_msg" gorm:"type:text"`
	Duration       int64     `json:"duration"` // 持续时间（毫秒）
	ConnectionTime time.Time `json:"connection_time"`
	CreatedAt      time.Time `json:"created_at"`
}

// TableName 指定表名
func (ConnectionLog) TableName() string {
	return "connection_logs"
}

// NewConnectionLog 创建连接日志
func NewConnectionLog(userID, deviceID, connectionID, action, status, clientIP, userAgent string) *ConnectionLog {
	return &ConnectionLog{
		ID:             uuid.New().String(),
		UserID:         userID,
		DeviceID:       deviceID,
		ConnectionID:   connectionID,
		Action:         action,
		Status:         status,
		ClientIP:       clientIP,
		UserAgent:      userAgent,
		ConnectionTime: time.Now(),
		CreatedAt:      time.Now(),
	}
}

// NetworkConfig 网络配置结构（JSON 内容）
type NetworkConfig struct {
	NetworkName   string   `json:"network_name"`
	Subnet        string   `json:"subnet"`
	Gateway       string   `json:"gateway"`
	DNSServers    []string `json:"dns_servers"`
	MTU           int      `json:"mtu"`
	TunnelProtocol string  `json:"tunnel_protocol"`
	Encryption    string   `json:"encryption"`
}
