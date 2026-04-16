package handlers

import (
	"bytes"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"testing"

	"github.com/gin-gonic/gin"
	"github.com/stretchr/testify/assert"
)

func setupTestRouter() *gin.Engine {
	gin.SetMode(gin.TestMode)
	return gin.New()
}

func TestRegisterRequest_Validate(t *testing.T) {
	tests := []struct {
		name    string
		request RegisterRequest
		wantErr bool
	}{
		{
			name: "valid request",
			request: RegisterRequest{
				Username: "testuser",
				Email:    "test@example.com",
				Password: "password123",
			},
			wantErr: false,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			err := tt.request.Validate()
			if (err != nil) != tt.wantErr {
				t.Errorf("Validate() error = %v, wantErr %v", err, tt.wantErr)
			}
		})
	}
}

func TestLoginRequest_Validate(t *testing.T) {
	tests := []struct {
		name    string
		request LoginRequest
		wantErr bool
	}{
		{
			name: "valid request",
			request: LoginRequest{
				Username: "testuser",
				Password: "password123",
			},
			wantErr: false,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			err := tt.request.Validate()
			if (err != nil) != tt.wantErr {
				t.Errorf("Validate() error = %v, wantErr %v", err, tt.wantErr)
			}
		})
	}
}

func TestUserHandler_Register(t *testing.T) {
	router := setupTestRouter()
	handler := NewUserHandler()
	
	router.POST("/api/v1/users/register", handler.Register)

	tests := []struct {
		name           string
		body           RegisterRequest
		expectedStatus int
	}{
		{
			name: "valid registration",
			body: RegisterRequest{
				Username: "testuser",
				Email:    "test@example.com",
				Password: "password123",
			},
			expectedStatus: http.StatusCreated,
		},
		{
			name: "invalid email",
			body: RegisterRequest{
				Username: "testuser",
				Email:    "invalid-email",
				Password: "password123",
			},
			expectedStatus: http.StatusBadRequest,
		},
		{
			name: "weak password",
			body: RegisterRequest{
				Username: "testuser",
				Email:    "test@example.com",
				Password: "123",
			},
			expectedStatus: http.StatusBadRequest,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			bodyBytes, _ := json.Marshal(tt.body)
			req, _ := http.NewRequest("POST", "/api/v1/users/register", bytes.NewBuffer(bodyBytes))
			req.Header.Set("Content-Type", "application/json")
			
			w := httptest.NewRecorder()
			router.ServeHTTP(w, req)

			assert.Equal(t, tt.expectedStatus, w.Code)
		})
	}
}

func TestUserHandler_Login(t *testing.T) {
	router := setupTestRouter()
	handler := NewUserHandler()
	
	router.POST("/api/v1/users/login", handler.Login)

	tests := []struct {
		name           string
		body           LoginRequest
		expectedStatus int
	}{
		{
			name: "valid login",
			body: LoginRequest{
				Username: "testuser",
				Password: "password123",
			},
			expectedStatus: http.StatusOK,
		},
		{
			name: "missing username",
			body: LoginRequest{
				Password: "password123",
			},
			expectedStatus: http.StatusBadRequest,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			bodyBytes, _ := json.Marshal(tt.body)
			req, _ := http.NewRequest("POST", "/api/v1/users/login", bytes.NewBuffer(bodyBytes))
			req.Header.Set("Content-Type", "application/json")
			
			w := httptest.NewRecorder()
			router.ServeHTTP(w, req)

			assert.Equal(t, tt.expectedStatus, w.Code)
		})
	}
}

func TestDeviceHandler_Register(t *testing.T) {
	router := setupTestRouter()
	handler := NewDeviceHandler()
	
	router.POST("/api/v1/devices", handler.Register)

	tests := []struct {
		name           string
		body           RegisterDeviceRequest
		expectedStatus int
	}{
		{
			name: "valid registration",
			body: RegisterDeviceRequest{
				Name:   "Test Device",
				Type:   "windows",
				OSType: "Windows 11",
				Version: "22H2",
				MAC:    "00:1A:2B:3C:4D:5E",
			},
			expectedStatus: http.StatusCreated,
		},
		{
			name: "missing required fields",
			body: RegisterDeviceRequest{
				Name: "Test Device",
			},
			expectedStatus: http.StatusBadRequest,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			bodyBytes, _ := json.Marshal(tt.body)
			req, _ := http.NewRequest("POST", "/api/v1/devices", bytes.NewBuffer(bodyBytes))
			req.Header.Set("Content-Type", "application/json")
			req.Header.Set("Authorization", "Bearer mock-token")
			
			w := httptest.NewRecorder()
			router.ServeHTTP(w, req)

			assert.Equal(t, tt.expectedStatus, w.Code)
		})
	}
}

func TestConfigHandler_Create(t *testing.T) {
	router := setupTestRouter()
	handler := NewConfigHandler()
	
	router.POST("/api/v1/configs", handler.Create)

	tests := []struct {
		name           string
		body           CreateConfigRequest
		expectedStatus int
	}{
		{
			name: "valid creation",
			body: CreateConfigRequest{
				DeviceID: "device-123",
				Name:     "Network Config",
				Type:     "network",
				Content:  `{"subnet": "10.0.0.0/24"}`,
			},
			expectedStatus: http.StatusCreated,
		},
		{
			name: "invalid type",
			body: CreateConfigRequest{
				DeviceID: "device-123",
				Name:     "Network Config",
				Type:     "invalid",
				Content:  `{"subnet": "10.0.0.0/24"}`,
			},
			expectedStatus: http.StatusBadRequest,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			bodyBytes, _ := json.Marshal(tt.body)
			req, _ := http.NewRequest("POST", "/api/v1/configs", bytes.NewBuffer(bodyBytes))
			req.Header.Set("Content-Type", "application/json")
			req.Header.Set("Authorization", "Bearer mock-token")
			
			w := httptest.NewRecorder()
			router.ServeHTTP(w, req)

			assert.Equal(t, tt.expectedStatus, w.Code)
		})
	}
}
