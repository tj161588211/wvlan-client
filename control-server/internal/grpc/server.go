package grpc

import (
	"fmt"
	"net"

	"go.uber.org/zap"
	"google.golang.org/grpc"
	"google.golang.org/grpc/reflection"

	"github.com/virtual-lan/control-server/internal/config"
	pb "github.com/virtual-lan/control-server/proto"
)

// Server gRPC 服务器
type Server struct {
	pb.UnimplementedDeviceServiceServer
	pb.UnimplementedConfigServiceServer
	pb.UnimplementedConnectionServiceServer

	cfg *config.Config
}

// NewServer 创建 gRPC 服务器
func NewServer(cfg *config.Config) *Server {
	return &Server{
		cfg: cfg,
	}
}

// Start 启动 gRPC 服务器
func (s *Server) Start() error {
	lis, err := net.Listen("tcp", s.cfg.GRPCPortAddr())
	if err != nil {
		return fmt.Errorf("无法监听 gRPC 端口：%w", err)
	}

	// 创建 gRPC 服务器
	grpcServer := grpc.NewServer(
		grpc.MaxSendMsgSize(s.cfg.gRPC.MaxSendMsgSize),
		grpc.MaxRecvMsgSize(s.cfg.gRPC.MaxRecvMsgSize),
	)

	// 注册服务
	pb.RegisterDeviceServiceServer(grpcServer, s)
	pb.RegisterConfigServiceServer(grpcServer, s)
	pb.RegisterConnectionServiceServer(grpcServer, s)

	// 启用反射支持（用于 gRPC 工具）
	reflection.Register(grpcServer)

	zap.L().Info("gRPC 服务器启动", zap.String("address", lis.Addr().String()))

	// 启动服务器
	if err := grpcServer.Serve(lis); err != nil {
		return fmt.Errorf("gRPC 服务器启动失败：%w", err)
	}

	return nil
}

// Stop 停止 gRPC 服务器
func (s *Server) Stop() {
	zap.L().Info("停止 gRPC 服务器")
	// 优雅关闭
	// grpcServer.GracefulStop()
}
