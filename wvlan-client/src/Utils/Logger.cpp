/**
 * @file Logger.cpp
 * @brief 日志工具类实现
 */

#include "Logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace WVLAN {
namespace Core {

static Logger* g_instance = nullptr;

Logger& Logger::Instance() {
    if (g_instance == nullptr) {
        g_instance = new Logger();
    }
    return *g_instance;
}

Logger::Logger() : _logLevel(LogLevel::Info), _initialized(false) {
}

Logger::~Logger() {
    if (_fileStream.is_open()) {
        _fileStream.close();
    }
    
    if (g_instance == this) {
        g_instance = nullptr;
    }
}

void Logger::SetLogLevel(LogLevel level) {
    _logLevel = level;
}

void Logger::SetLogFile(const std::string& path) {
    if (_fileStream.is_open()) {
        _fileStream.close();
    }
    
    _logFile = path;
    _fileStream.open(path, std::ios::app);
    _initialized = _fileStream.is_open();
}

std::string Logger::GetTimestamp() {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string Logger::LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Critical: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

void Logger::Log(LogLevel level, const std::string& message) {
    if (level < _logLevel) {
        return;
    }

    std::lock_guard<std::mutex> lock(_mutex);
    
    std::string timestamp = GetTimestamp();
    std::string levelStr = LogLevelToString(level);
    
    std::string logLine = "[" + timestamp + "] [" + levelStr + "] " + message + "\n";
    
    // 输出到控制台
    std::cout << logLine;
    
    // 输出到文件
    if (_initialized && _fileStream.is_open()) {
        _fileStream << logLine;
        _fileStream.flush();
    }
}

void Logger::Debug(const std::string& message) {
    Log(LogLevel::Debug, message);
}

void Logger::Info(const std::string& message) {
    Log(LogLevel::Info, message);
}

void Logger::Warning(const std::string& message) {
    Log(LogLevel::Warning, message);
}

void Logger::Error(const std::string& message) {
    Log(LogLevel::Error, message);
}

void Logger::Critical(const std::string& message) {
    Log(LogLevel::Critical, message);
}

} // namespace Core
} // namespace WVLAN
