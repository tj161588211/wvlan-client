package logger

import (
	"os"
	"time"

	"go.uber.org/zap"
	"go.uber.org/zap/zapcore"
)

// Config 日志配置
type Config struct {
	Level  string
	Format string
}

var logger *zap.Logger
var sugar *zap.SugaredLogger

// Init 初始化日志
func Init(cfg Config) error {
	level, err := parseLevel(cfg.Level)
	if err != nil {
		return err
	}

	encoderConfig := zapcore.EncoderConfig{
		TimeKey:        "time",
		LevelKey:       "level",
		NameKey:        "logger",
		CallerKey:      "caller",
		MessageKey:     "msg",
		StacktraceKey:  "stacktrace",
		LineEnding:     zapcore.DefaultLineEnding,
		EncodeLevel:    zapcore.LowercaseLevelEncoder,
		EncodeTime:     encodeTime,
		EncodeDuration: zapcore.SecondsDurationEncoder,
		EncodeCaller:   zapcore.ShortCallerEncoder,
	}

	var encoder zapcore.Encoder
	if cfg.Format == "json" {
		encoder = zapcore.NewJSONEncoder(encoderConfig)
	} else {
		encoder = zapcore.NewConsoleEncoder(encoderConfig)
	}

	core := zapcore.NewCore(
		encoder,
		zapcore.AddSync(os.Stdout),
		level,
	)

	logger = zap.New(core, zap.AddCaller(), zap.AddCallerSkip(1))
	sugar = logger.Sugar()

	return nil
}

// parseLevel 解析日志级别
func parseLevel(level string) (zapcore.Level, error) {
	var l zapcore.Level
	err := l.UnmarshalText([]byte(level))
	return l, err
}

// encodeTime 自定义时间格式
func encodeTime(t time.Time, enc zapcore.PrimitiveArrayEncoder) {
	enc.AppendString(t.Format("2006-01-02T15:04:05.000Z07:00"))
}

// Logger 获取日志器
func Logger() *zap.Logger {
	return logger
}

// Sugar 获取 sugared logger
func Sugar() *zap.SugaredLogger {
	return sugar
}

// Info 记录 info 级别日志
func Info(msg string, fields ...zap.Field) {
	logger.Info(msg, fields...)
}

// Infof 格式化记录 info 级别日志
func Infof(template string, args ...interface{}) {
	sugar.Infof(template, args...)
}

// Warn 记录 warn 级别日志
func Warn(msg string, fields ...zap.Field) {
	logger.Warn(msg, fields...)
}

// Warnf 格式化记录 warn 级别日志
func Warnf(template string, args ...interface{}) {
	sugar.Warnf(template, args...)
}

// Error 记录 error 级别日志
func Error(msg string, fields ...zap.Field) {
	logger.Error(msg, fields...)
}

// Errorf 格式化记录 error 级别日志
func Errorf(template string, args ...interface{}) {
	sugar.Errorf(template, args...)
}

// Debug 记录 debug 级别日志
func Debug(msg string, fields ...zap.Field) {
	logger.Debug(msg, fields...)
}

// Debugf 格式化记录 debug 级别日志
func Debugf(template string, args ...interface{}) {
	sugar.Debugf(template, args...)
}

// Fatal 记录 fatal 级别日志并退出
func Fatal(msg string, fields ...zap.Field) {
	logger.Fatal(msg, fields...)
}

// Fatalf 格式化记录 fatal 级别日志并退出
func Fatalf(template string, args ...interface{}) {
	sugar.Fatalf(template, args...)
}

// Sync 同步日志缓冲区
func Sync() error {
	return logger.Sync()
}
