/**
 * @file Logger.h
 * @brief 日志工具类 - 日志记录和输出
 * 
 * @version 1.0.0
 * @date 2026-04-15
 */

#ifndef WVLAN_LOGGER_H
#define WVLAN_LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <ctime>

namespace WVLAN {
namespace Core {

enum class LogLevel {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3,
    Critical = 4
};

class Logger {
public:
    static Logger& Instance();

    void SetLogLevel(LogLevel level);
    void SetLogFile(const std::string& path);
    
    void Debug(const std::string& message);
    void Info(const std::string& message);
    void Warning(const std::string& message);
    void Error(const std::string& message);
    void Critical(const std::string& message);

private:
    Logger();
    ~Logger();

    void Log(LogLevel level, const std::string& message);
    std::string GetTimestamp();
    std::string LogLevelToString(LogLevel level);

    LogLevel _logLevel;
    std::string _logFile;
    std::ofstream _fileStream;
    std::mutex _mutex;
    bool _initialized;
};

} // namespace Core
} // namespace WVLAN

#endif // WVLAN_LOGGER_H
