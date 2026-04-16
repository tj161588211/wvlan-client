namespace WVLAN.Client.Models
{
    /// <summary>
    /// 节点状态信息
    /// </summary>
    public class NodeStatus
    {
        public string PublicKey { get; set; } = string.Empty;
        public string? DisplayName { get; set; }
        public string? Endpoint { get; set; }
        public string AllowedIPs { get; set; } = string.Empty;
        public long ReceivedBytes { get; set; }
        public long TransmittedBytes { get; set; }
        public DateTime? LastHandshake { get; set; }
        public int PersistentKeepalive { get; set; }
        public bool IsActive { get; set; }
        public TimeSpan? TimeSinceLastHandshake => LastHandshake.HasValue 
            ? DateTime.Now - LastHandshake.Value 
            : null;
    }

    /// <summary>
    /// 接口统计信息
    /// </summary>
    public class InterfaceStats
    {
        public long TotalReceivedBytes { get; set; }
        public long TotalTransmittedBytes { get; set; }
        public int ActivePeers { get; set; }
        public DateTime? LastActivity { get; set; }
        public double UploadSpeed { get; set; } // bytes per second
        public double DownloadSpeed { get; set; } // bytes per second
    }

    /// <summary>
    /// 连接状态信息
    /// </summary>
    public class ConnectionInfo
    {
        public InterfaceState State { get; set; }
        public string? InterfaceName { get; set; }
        public string? LocalAddress { get; set; }
        public List<NodeStatus> Peers { get; set; } = new();
        public InterfaceStats Stats { get; set; } = new();
        public string? ErrorMessage { get; set; }
        public bool IsConnected => State == InterfaceState.Up;
        public bool IsConnecting => State == InterfaceState.Connecting;
        public bool HasError => State == InterfaceState.Error;
    }

    /// <summary>
    /// 网络接口信息
    /// </summary>
    public class NetworkInterfaceInfo
    {
        public string Name { get; set; } = string.Empty;
        public string Description { get; set; } = string.Empty;
        public string? IPAddress { get; set; }
        public string? MACAddress { get; set; }
        public bool IsWireGuard { get; set; }
        public NetworkType NetworkType { get; set; }
    }
}
