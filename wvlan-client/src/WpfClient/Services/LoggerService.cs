using Serilog;
using WVLAN.Client.Models;

namespace WVLAN.Client.Services
{
    /// <summary>
    /// 日志服务 - 提供统一的日志记录接口
    /// </summary>
    public class LoggerService : IDisposable
    {
        private readonly ILogger _logger;
        private readonly List<LogEntry> _logEntries = new();
        private readonly object _lock = new();
        private const int MaxLogEntries = 10000;

        public event Action<LogEntry>? OnLogEntry;

        public IReadOnlyList<LogEntry> LogEntries 
        { 
            get 
            {
                lock (_lock)
                {
                    return _logEntries.AsReadOnly();
                }
            }
        }

        public LoggerService(ILogger logger)
        {
            _logger = logger;
        }

        public void Verbose(string message, string? category = null)
        {
            WriteLog(LogLevel.Verbose, message, category);
        }

        public void Debug(string message, string? category = null)
        {
            WriteLog(LogLevel.Debug, message, category);
        }

        public void Information(string message, string? category = null)
        {
            WriteLog(LogLevel.Information, message, category);
        }

        public void Warning(string message, string? category = null)
        {
            WriteLog(LogLevel.Warning, message, category);
        }

        public void Error(string message, string? category = null)
        {
            WriteLog(LogLevel.Error, message, category);
        }

        public void Error(Exception ex, string message, string? category = null)
        {
            WriteLog(LogLevel.Error, $"{message} - {ex.Message}", category, ex.ToString());
        }

        public void Fatal(string message, string? category = null)
        {
            WriteLog(LogLevel.Fatal, message, category);
        }

        public void Fatal(Exception ex, string message, string? category = null)
        {
            WriteLog(LogLevel.Fatal, $"{message} - {ex.Message}", category, ex.ToString());
        }

        private void WriteLog(LogLevel level, string message, string? category = null, string? exception = null)
        {
            var entry = new LogEntry
            {
                Timestamp = DateTime.Now,
                Level = level,
                Message = message,
                Category = category,
                Exception = exception
            };

            // 添加到内存列表
            lock (_lock)
            {
                _logEntries.Add(entry);
                if (_logEntries.Count > MaxLogEntries)
                {
                    _logEntries.RemoveAt(0);
                }
            }

            // 触发事件
            OnLogEntry?.Invoke(entry);

            // 写入底层日志
            switch (level)
            {
                case LogLevel.Verbose:
                case LogLevel.Debug:
                    _logger.Debug("{Category} {Message}", category ?? "General", message);
                    break;
                case LogLevel.Information:
                    _logger.Information("{Category} {Message}", category ?? "General", message);
                    break;
                case LogLevel.Warning:
                    _logger.Warning("{Category} {Message}", category ?? "General", message);
                    break;
                case LogLevel.Error:
                    _logger.Error("{Category} {Message}", category ?? "General", message);
                    break;
                case LogLevel.Fatal:
                    _logger.Fatal("{Category} {Message}", category ?? "General", message);
                    break;
            }
        }

        public void ClearLogs()
        {
            lock (_lock)
            {
                _logEntries.Clear();
            }
        }

        public List<LogEntry> GetFilteredLogs(LogFilter filter)
        {
            lock (_lock)
            {
                var filtered = _logEntries.AsEnumerable();

                // 按级别过滤
                if (!filter.ShowVerbose)
                    filtered = filtered.Where(x => x.Level != LogLevel.Verbose);
                if (!filter.ShowDebug)
                    filtered = filtered.Where(x => x.Level != LogLevel.Debug);
                if (!filter.ShowInformation)
                    filtered = filtered.Where(x => x.Level != LogLevel.Information);
                if (!filter.ShowWarning)
                    filtered = filtered.Where(x => x.Level != LogLevel.Warning);
                if (!filter.ShowError)
                    filtered = filtered.Where(x => x.Level != LogLevel.Error);
                if (!filter.ShowFatal)
                    filtered = filtered.Where(x => x.Level != LogLevel.Fatal);

                // 按时间过滤
                if (filter.StartTime.HasValue)
                    filtered = filtered.Where(x => x.Timestamp >= filter.StartTime.Value);
                if (filter.EndTime.HasValue)
                    filtered = filtered.Where(x => x.Timestamp <= filter.EndTime.Value);

                // 按文本搜索
                if (!string.IsNullOrWhiteSpace(filter.SearchText))
                {
                    var search = filter.SearchText.ToLower();
                    filtered = filtered.Where(x => 
                        x.Message.ToLower().Contains(search) ||
                        (x.Category?.ToLower().Contains(search) ?? false) ||
                        (x.Exception?.ToLower().Contains(search) ?? false));
                }

                return filtered.ToList();
            }
        }

        public void Dispose()
        {
            OnLogEntry = null;
        }
    }
}
