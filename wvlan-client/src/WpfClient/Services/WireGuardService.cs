using System.Timers;
using Serilog;
using WVLAN.Client.Models;
using Timer = System.Timers.Timer;

namespace WVLAN.Client.Services
{
    /// <summary>
    /// WireGuard 服务 - 管理 WireGuard 接口的连接和状态
    /// </summary>
    public class WireGuardService : IDisposable
    {
        private readonly ILogger _logger;
        private readonly ConfigService _configService;
        private readonly object _lock = new();
        
        // 模拟桥接层对象（实际使用时替换为 C++/CLI Bridge）
        private NativeBridge.WireGuardBridge? _bridge;
        
        private Timer? _statusPollTimer;
        private ConnectionInfo _currentConnection = new();
        private bool _isDisposed = false;
        private const int STATUS_POLL_INTERVAL_MS = 2000;

        public event Action<ConnectionInfo>? OnConnectionChanged;
        public event Action<NodeStatus>? OnPeerStatusChanged;

        public ConnectionInfo CurrentConnection 
        { 
            get 
            {
                lock (_lock)
                {
                    return _currentConnection;
                }
            }
            private set
            {
                lock (_lock)
                {
                    _currentConnection = value;
                }
                OnConnectionChanged?.Invoke(value);
            }
        }

        public WireGuardService(ILogger logger, ConfigService configService)
        {
            _logger = logger;
            _configService = configService;
            
            Initialize();
        }

        private void Initialize()
        {
            try
            {
                // 尝试初始化桥接层
                if (NativeBridge.NetworkOperations.IsRunAsAdmin())
                {
                    _logger.Information("Running with administrator privileges");
                    
                    // 检查驱动是否已安装
                    if (NativeBridge.WireGuardBridge.IsDriverInstalled())
                    {
                        _logger.Information("WireGuard driver is installed");
                        _bridge = new NativeBridge.WireGuardBridge();
                    }
                    else
                    {
                        _logger.Warning("WireGuard driver not found. Use for UI demo only.");
                    }
                }
                else
                {
                    _logger.Warning("Not running as admin. WireGuard operations will be limited.");
                }

                // 启动状态轮询
                StartStatusPolling();
                
                _logger.Information("WireGuardService initialized");
            }
            catch (Exception ex)
            {
                _logger.Error(ex, "Failed to initialize WireGuardService");
            }
        }

        private void StartStatusPolling()
        {
            _statusPollTimer = new Timer(STATUS_POLL_INTERVAL_MS);
            _statusPollTimer.Elapsed += async (sender, args) => await PollStatusAsync();
            _statusPollTimer.AutoReset = true;
            _statusPollTimer.Enabled = true;
        }

        private async Task PollStatusAsync()
        {
            if (_bridge == null || _currentConnection.InterfaceName == null)
                return;

            try
            {
                await Task.Run(() =>
                {
                    var state = _bridge.GetInterfaceState();
                    var newState = (InterfaceState)state;
                    
                    if (newState != CurrentConnection.State)
                    {
                        CurrentConnection = new ConnectionInfo
                        {
                            State = newState,
                            InterfaceName = CurrentConnection.InterfaceName,
                            LocalAddress = CurrentConnection.LocalAddress
                        };
                        
                        _logger.Information("Interface state changed: {State}", newState);
                    }

                    // 更新统计信息
                    if (newState == InterfaceState.Up && _bridge != null)
                    {
                        var stats = _bridge.GetInterfaceStats();
                        var peers = _bridge.GetPeersStatus();
                        
                        var nodeStatuses = new List<NodeStatus>();
                        foreach (var peer in peers)
                        {
                            nodeStatuses.Add(new NodeStatus
                            {
                                PublicKey = peer.PublicKey ?? string.Empty,
                                Endpoint = peer.Endpoint,
                                ReceivedBytes = peer.ReceivedBytes,
                                TransmittedBytes = peer.TransmittedBytes,
                                LastHandshake = peer.LastHandshake != DateTime.MinValue ? peer.LastHandshake : null,
                                PersistentKeepalive = peer.PersistentKeepalive,
                                IsActive = peer.LastHandshake != DateTime.MinValue
                            });
                        }

                        CurrentConnection = new ConnectionInfo
                        {
                            State = newState,
                            InterfaceName = CurrentConnection.InterfaceName,
                            LocalAddress = CurrentConnection.LocalAddress,
                            Peers = nodeStatuses,
                            Stats = new InterfaceStats
                            {
                                TotalReceivedBytes = stats.TotalReceivedBytes,
                                TotalTransmittedBytes = stats.TotalTransmittedBytes,
                                ActivePeers = stats.ActivePeers,
                                LastActivity = stats.LastActivity != DateTime.MinValue ? stats.LastActivity : null
                            }
                        };
                    }
                });
            }
            catch (Exception ex)
            {
                _logger.Error(ex, "Error polling status");
            }
        }

        /// <summary>
        /// 使用配置文件连接
        /// </summary>
        public async Task<OperationResult> Connect(NetworkProfile profile)
        {
            if (_bridge == null)
            {
                return OperationResult.Failure(OperationStatus.InternalError, "Bridge not initialized");
            }

            try
            {
                _logger.Information("Connecting with profile: {ProfileName}", profile.ProfileName);
                
                CurrentConnection = new ConnectionInfo 
                { 
                    State = InterfaceState.Connecting,
                    InterfaceName = profile.ProfileName
                };

                await Task.Run(async () =>
                {
                    try
                    {
                        // 初始化接口
                        var initResult = _bridge.InitializeInterface(
                            profile.ProfileName,
                            profile.Interface.PrivateKey,
                            profile.Interface.ListenPort);
                        
                        if (!initResult.IsSuccess)
                        {
                            throw new Exception($"Failed to initialize interface: {initResult}");
                        }

                        // 设置接口地址
                        if (!string.IsNullOrEmpty(profile.Interface.Address))
                        {
                            var addrResult = _bridge.SetInterfaceAddress(profile.Interface.Address);
                            if (!addrResult.IsSuccess)
                            {
                                _logger.Warning("Failed to set interface address: {Result}", addrResult);
                            }
                        }

                        // 添加对等节点
                        foreach (var peer in profile.Peers)
                        {
                            if (!string.IsNullOrEmpty(peer.PublicKey) && !string.IsNullOrEmpty(peer.Endpoint))
                            {
                                var peerResult = _bridge.AddPeer(
                                    peer.PublicKey,
                                    peer.Endpoint,
                                    peer.AllowedIPs);
                                
                                if (!peerResult.IsSuccess)
                                {
                                    _logger.Warning("Failed to add peer: {PublicKey}", peer.PublicKey);
                                }
                            }
                        }

                        // 启动接口
                        var startResult = _bridge.StartInterface();
                        if (!startResult.IsSuccess)
                        {
                            throw new Exception($"Failed to start interface: {startResult}");
                        }

                        CurrentConnection = new ConnectionInfo
                        {
                            State = InterfaceState.Up,
                            InterfaceName = profile.ProfileName,
                            LocalAddress = profile.Interface.Address
                        };

                        _logger.Information("Connection established successfully");
                    }
                    catch (Exception ex)
                    {
                        _logger.Error(ex, "Connection failed");
                        CurrentConnection = new ConnectionInfo
                        {
                            State = InterfaceState.Error,
                            InterfaceName = profile.ProfileName,
                            ErrorMessage = ex.Message
                        };
                        throw;
                    }
                });

                return OperationResult.SuccessResult();
            }
            catch (Exception ex)
            {
                _logger.Error(ex, "Connect error");
                return OperationResult.Failure(OperationStatus.NetworkError, ex.Message);
            }
        }

        /// <summary>
        /// 断开连接
        /// </summary>
        public async Task<OperationResult> Disconnect()
        {
            if (_bridge == null)
            {
                return OperationResult.SuccessResult();
            }

            try
            {
                _logger.Information("Disconnecting...");
                
                await Task.Run(() =>
                {
                    var result = _bridge.StopInterface();
                    
                    CurrentConnection = new ConnectionInfo
                    {
                        State = InterfaceState.Down,
                        InterfaceName = CurrentConnection.InterfaceName
                    };
                    
                    _logger.Information("Disconnected");
                });

                return OperationResult.SuccessResult();
            }
            catch (Exception ex)
            {
                _logger.Error(ex, "Disconnect error");
                return OperationResult.Failure(OperationStatus.NetworkError, ex.Message);
            }
        }

        /// <summary>
        /// 关闭并删除接口
        /// </summary>
        public async Task<OperationResult> Shutdown()
        {
            if (_bridge == null)
            {
                return OperationResult.SuccessResult();
            }

            try
            {
                _logger.Information("Shutting down interface...");
                
                await Task.Run(() =>
                {
                    var result = _bridge.ShutdownInterface();
                    _logger.Information("Interface shutdown complete");
                });

                CurrentConnection = new ConnectionInfo { State = InterfaceState.Down };
                
                return OperationResult.SuccessResult();
            }
            catch (Exception ex)
            {
                _logger.Error(ex, "Shutdown error");
                return OperationResult.Failure(OperationStatus.NetworkError, ex.Message);
            }
        }

        /// <summary>
        /// 获取网络接口列表
        /// </summary>
        public List<NetworkInterfaceInfo> GetNetworkInterfaces()
        {
            if (_bridge == null) return new List<NetworkInterfaceInfo>();
            
            try
            {
                var ops = new NativeBridge.NetworkOperations();
                return ops.GetNetworkInterfaces().Select(i => new NetworkInterfaceInfo
                {
                    Name = i.Name,
                    Description = i.Description,
                    IPAddress = i.IPAddress,
                    MACAddress = i.MACAddress,
                    IsWireGuard = i.IsWireGuard,
                    NetworkType = (NetworkType)(int)i.NetworkType
                }).ToList();
            }
            catch (Exception ex)
            {
                _logger.Error(ex, "Failed to get network interfaces");
                return new List<NetworkInterfaceInfo>();
            }
        }

        public void Dispose()
        {
            if (_isDisposed) return;
            
            _isDisposed = true;
            
            try
            {
                _statusPollTimer?.Stop();
                _statusPollTimer?.Dispose();
                
                if (_bridge != null)
                {
                    _bridge.StopInterface();
                    _bridge.Dispose();
                }
                
                _logger.Information("WireGuardService disposed");
            }
            catch (Exception ex)
            {
                _logger.Error(ex, "Error during disposal");
            }
        }
    }
}
