# Makefile for Control Server

.PHONY: help build run test clean docker-up docker-down migrate swagger install deps

# 变量定义
APP_NAME := control-server
GO := go
GOOS := $(shell go env GOOS)
GOARCH := $(shell go env GOARCH)
VERSION := $(shell git describe --tags --always --dirty 2>/dev/null || echo "1.0.0")
BUILD_TIME := $(shell date -u '+%Y-%m-%d_%H:%M:%S')

# 默认目标
help: ## 显示帮助信息
	@echo "用法：make [目标]"
	@echo ""
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2}'

## 安装与依赖
deps: ## 下载依赖
	$(GO) mod download

install: ## 安装依赖
	$(GO) get -v -t ./...
	$(GO) mod tidy

## 构建
build: ## 编译应用
	$(GO) build -ldflags "-X main.Version=$(VERSION)" -o bin/$(APP_NAME) ./cmd/server

build-all: ## 跨平台编译
	CGO_ENABLED=0 GOOS=linux GOARCH=amd64 $(GO) build -ldflags "-X main.Version=$(VERSION)" -o bin/linux/$(APP_NAME) ./cmd/server
	CGO_ENABLED=0 GOOS=darwin GOARCH=amd64 $(GO) build -ldflags "-X main.Version=$(VERSION)" -o bin/darwin/$(APP_NAME) ./cmd/server
	CGO_ENABLED=0 GOOS=windows GOARCH=amd64 $(GO) build -ldflags "-X main.Version=$(VERSION)" -o bin/windows/$(APP_NAME).exe ./cmd/server

bin/$(APP_NAME): ## 创建二进制文件目录
	mkdir -p bin

## 运行
run: ## 运行开发服务器
	$(GO) run ./cmd/server/main.go

run-prod: ## 运行生产环境
	APP_ENV=production $(GO) run ./cmd/server/main.go

## 测试
test: ## 运行测试
	$(GO) test -v ./...

test-cover: ## 生成测试覆盖率报告
	$(GO) test -coverprofile=coverage.out ./...
	$(GO) tool cover -html=coverage.out -o coverage.html

test-race: ## 运行竞态检测
	$(GO) test -race ./...

## 代码质量
lint: ## 运行 linter (需要 golint)
	golangci-lint run

fmt: ## 格式化代码
	$(GO) fmt ./...

vet: ## 运行 go vet
	$(GO) vet ./...

## 数据库迁移
migrate-up: ## 执行向上迁移
	sql-migrate up -config=migrations/migrate.yml

migrate-down: ## 回滚迁移
	sql-migrate down -config=migrations/migrate.yml

migrate-new: ## 创建新迁移
	@read -p "请输入迁移名称：" name; sql-migrate new migrations/$$name

## Swagger 文档
swagger: ## 生成 Swagger 文档
	swag init --generalInfo cmd/server/main.go --output docs/api

## Docker
docker-build: ## 构建 Docker 镜像
	docker build -t $(APP_NAME):$(VERSION) .

docker-run: ## 使用 Docker 运行
	docker run -d -p 8080:8080 -p 9090:9090 --name $(APP_NAME) $(APP_NAME):$(VERSION)

docker-compose-up: ## 使用 Docker Compose 启动所有服务
	docker-compose up -d

docker-compose-down: ## 停止 Docker Compose 服务
	docker-compose down

docker-compose-logs: ## 查看 Docker Compose 日志
	docker-compose logs -f

## 清理
clean: ## 清理构建文件
	rm -rf bin/
	rm -f coverage.out coverage.html

## 实用工具
env-example: ## 生成环境变量示例
	cp .env.example .env

## 帮助信息
.DEFAULT_GOAL := help
