namespace WVLAN.Client.Models
{
    /// <summary>
    /// 接口配置信息
    /// </summary>
    public class InterfaceConfig
    {
        public string PrivateKey { get; set; } = string.Empty;
        public string PublicKey { get; set; } = string.Empty;
        public string Address { get; set; } = string.Empty;
        public string? DNS { get; set; } = "8.8.8.8,8.8.4.4";
        public int ListenPort { get; set; } = 51820;
    }

    /// <summary>
    /// 对等节点配置
    /// </summary>
    public class PeerConfig
    {
        public string PublicKey { get; set; } = string.Empty;
        public string? Endpoint { get; set; }
        public string AllowedIPs { get; set; } = "0.0.0.0/0";
        public int PersistentKeepalive { get; set; } = 25;
        public string? DisplayName { get; set; }
    }

    /// <summary>
    /// 完整网络配置文件
    /// </summary>
    public class NetworkProfile
    {
        public string ProfileName { get; set; } = string.Empty;
        public InterfaceConfig Interface { get; set; } = new();
        public List<PeerConfig> Peers { get; set; } = new();
        public DateTime CreatedAt { get; set; } = DateTime.Now;
        public DateTime UpdatedAt { get; set; } = DateTime.Now;
        public string? Description { get; set; }
        public bool IsActive { get; set; }
    }

    /// <summary>
    /// 配置解析结果
    /// </summary>
    public class ConfigParseResult
    {
        public bool Success { get; set; }
        public string? ErrorMessage { get; set; }
        public NetworkProfile? Profile { get; set; }
    }
}
