namespace WVLAN.Client.Models
{
    /// <summary>
    /// 操作状态枚举
    /// </summary>
    public enum OperationStatus
    {
        Success = 0,
        InvalidParameter = -1,
        AccessDenied = -2,
        NetworkError = -3,
        Timeout = -4,
        NotFound = -5,
        AlreadyExists = -6,
        InternalError = -99
    }

    /// <summary>
    /// 接口状态枚举
    /// </summary>
    public enum InterfaceState
    {
        Down = 0,
        Up = 1,
        Connecting = 2,
        Error = 3
    }

    /// <summary>
    /// 网络类型枚举
    /// </summary>
    public enum NetworkType
    {
        Unknown = 0,
        LocalNetwork = 1,
        PublicNetwork = 2,
        PrivateNetwork = 3
    }

    /// <summary>
    /// 操作结果
    /// </summary>
    public class OperationResult
    {
        public OperationStatus Status { get; set; }
        public string? ErrorMessage { get; set; }
        public bool IsSuccess => Status == OperationStatus.Success;

        public static OperationResult SuccessResult() => new() { Status = OperationStatus.Success };
        public static OperationResult Failure(OperationStatus status, string? message = null) => 
            new() { Status = status, ErrorMessage = message };
    }
}
