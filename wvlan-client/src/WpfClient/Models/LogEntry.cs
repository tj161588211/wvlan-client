namespace WVLAN.Client.Models
{
    /// <summary>
    /// 日志级别
    /// </summary>
    public enum LogLevel
    {
        Verbose = 0,
        Debug = 1,
        Information = 2,
        Warning = 3,
        Error = 4,
        Fatal = 5
    }

    /// <summary>
    /// 日志条目
    /// </summary>
    public class LogEntry
    {
        public DateTime Timestamp { get; set; }
        public LogLevel Level { get; set; }
        public string Message { get; set; } = string.Empty;
        public string? Exception { get; set; }
        public string? Category { get; set; }

        public string LevelString => Level switch
        {
            LogLevel.Verbose => "VRB",
            LogLevel.Debug => "DBG",
            LogLevel.Information => "INF",
            LogLevel.Warning => "WRN",
            LogLevel.Error => "ERR",
            LogLevel.Fatal => "FTL",
            _ => "UNK"
        };

        public override string ToString() => 
            $"[{Timestamp:HH:mm:ss.fff}] [{LevelString}] {Message}";
    }

    /// <summary>
    /// 日志过滤条件
    /// </summary>
    public class LogFilter
    {
        public bool ShowVerbose { get; set; } = true;
        public bool ShowDebug { get; set; } = true;
        public bool ShowInformation { get; set; } = true;
        public bool ShowWarning { get; set; } = true;
        public bool ShowError { get; set; } = true;
        public bool ShowFatal { get; set; } = true;
        public string? SearchText { get; set; }
        public DateTime? StartTime { get; set; }
        public DateTime? EndTime { get; set; }
    }
}
