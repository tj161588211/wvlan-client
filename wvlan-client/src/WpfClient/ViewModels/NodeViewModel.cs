using System.Collections.ObjectModel;
using System.Windows.Input;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using WVLAN.Client.Models;

namespace WVLAN.Client.ViewModels
{
    /// <summary>
    /// 节点状态 ViewModel
    /// </summary>
    public partial class NodesViewModel : ObservableObject
    {
        private readonly Services.WireGuardService _wireGuardService;
        private readonly Services.LoggerService _loggerService;

        [ObservableProperty]
        private ObservableCollection<NodeStatus> _nodes = new();

        [ObservableProperty]
        private NodeStatus? _selectedNode;

        [ObservableProperty]
        private InterfaceStats _stats = new();

        [ObservableProperty]
        private InterfaceState _connectionState;

        public NodesViewModel(
            Services.WireGuardService wireGuardService,
            Services.LoggerService loggerService)
        {
            _wireGuardService = wireGuardService;
            _loggerService = loggerService;

            _wireGuardService.OnConnectionChanged += OnConnectionChanged;
        }

        private void OnConnectionChanged(ConnectionInfo info)
        {
            DispatcherHelper.Invoke(() =>
            {
                ConnectionState = info.State;
                Stats = info.Stats;

                Nodes.Clear();
                foreach (var peer in info.Peers)
                {
                    Nodes.Add(peer);
                }
            });
        }

        public void Refresh()
        {
            var info = _wireGuardService.CurrentConnection;
            OnConnectionChanged(info);
        }
    }

    /// <summary>
    /// 仪表盘 ViewModel
    /// </summary>
    public partial class DashboardViewModel : ObservableObject
    {
        private readonly Services.WireGuardService _wireGuardService;
        private readonly Services.ConfigService _configService;
        private readonly Services.LoggerService _loggerService;

        [ObservableProperty]
        private InterfaceState _connectionState;

        [ObservableProperty]
        private string _connectionStatusText = "Disconnected";

        [ObservableProperty]
        private string _localAddress = "N/A";

        [ObservableProperty]
        private long _totalReceivedBytes;

        [ObservableProperty]
        private long _totalTransmittedBytes;

        [ObservableProperty]
        private int _activePeers;

        [ObservableProperty]
        private int _totalProfiles;

        [ObservableProperty]
        private bool _isConnecting;

        [ObservableProperty]
        private bool _isConnected;

        public ICommand ConnectCommand { get; }
        public ICommand DisconnectCommand { get; }
        public ICommand RefreshCommand { get; }

        public DashboardViewModel(
            Services.WireGuardService wireGuardService,
            Services.ConfigService configService,
            Services.LoggerService loggerService)
        {
            _wireGuardService = wireGuardService;
            _configService = configService;
            _loggerService = loggerService;

            ConnectCommand = new AsyncRelayCommand(ExecuteConnectAsync, CanExecuteConnect);
            DisconnectCommand = new AsyncRelayCommand(ExecuteDisconnectAsync, CanExecuteDisconnect);
            RefreshCommand = new RelayCommand(ExecuteRefresh);

            _wireGuardService.OnConnectionChanged += OnConnectionChanged;
        }

        private void OnConnectionChanged(ConnectionInfo info)
        {
            DispatcherHelper.Invoke(() =>
            {
                ConnectionState = info.State;
                IsConnected = info.IsConnected;
                IsConnecting = info.IsConnecting;
                
                ConnectionStatusText = info.State switch
                {
                    InterfaceState.Up => "Connected",
                    InterfaceState.Connecting => "Connecting...",
                    InterfaceState.Error => "Error",
                    _ => "Disconnected"
                };

                LocalAddress = info.LocalAddress ?? "N/A";
                TotalReceivedBytes = info.Stats.TotalReceivedBytes;
                TotalTransmittedBytes = info.Stats.TotalTransmittedBytes;
                ActivePeers = info.Stats.ActivePeers;

                CommandManager.InvalidateRequerySuggested();
            });
        }

        private async Task ExecuteConnectAsync()
        {
            var profiles = _configService.GetAllProfiles();
            var activeProfile = profiles.FirstOrDefault(p => p.IsActive) ?? profiles.FirstOrDefault();

            if (activeProfile == null)
            {
                _loggerService.Warning("No profile available to connect");
                return;
            }

            try
            {
                IsConnecting = true;
                var result = await _wireGuardService.Connect(activeProfile);
                
                if (!result.IsSuccess)
                {
                    _loggerService.Error($"Connection failed: {result.ErrorMessage}");
                }
            }
            finally
            {
                IsConnecting = false;
            }
        }

        private bool CanExecuteConnect()
        {
            return !IsConnecting && !IsConnected;
        }

        private async Task ExecuteDisconnectAsync()
        {
            try
            {
                IsConnecting = true;
                var result = await _wireGuardService.Disconnect();
                
                if (!result.IsSuccess)
                {
                    _loggerService.Error($"Disconnect failed: {result.ErrorMessage}");
                }
            }
            finally
            {
                IsConnecting = false;
            }
        }

        private bool CanExecuteDisconnect()
        {
            return IsConnected && !IsConnecting;
        }

        private void ExecuteRefresh()
        {
            var info = _wireGuardService.CurrentConnection;
            OnConnectionChanged(info);
            
            TotalProfiles = _configService.GetAllProfiles().Count;
        }

        public void LoadData()
        {
            TotalProfiles = _configService.GetAllProfiles().Count;
            ExecuteRefresh();
        }
    }
}
