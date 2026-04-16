using System.Collections.ObjectModel;
using System.Windows.Input;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using WVLAN.Client.Models;

namespace WVLAN.Client.ViewModels
{
    /// <summary>
    /// 导航项
    /// </summary>
    public class NavigationItem
    {
        public string Icon { get; set; } = string.Empty;
        public string DisplayName { get; set; } = string.Empty;
        public object Content { get; set; } = null!;
        public bool IsSelected { get; set; }
    }

    /// <summary>
    /// 主窗口 ViewModel
    /// </summary>
    public partial class MainWindowViewModel : ObservableObject, IDisposable
    {
        private readonly Services.WireGuardService _wireGuardService;
        private readonly Services.ConfigService _configService;
        private readonly Services.LoggerService _loggerService;
        
        private bool _isDisposed = false;

        [ObservableProperty]
        private ObservableCollection<NavigationItem> _navigationItems = new();

        [ObservableProperty]
        private NavigationItem? _selectedNavigation;

        [ObservableProperty]
        private string _connectionStatusText = "Disconnected";

        [ObservableProperty]
        private SolidColorBrush _connectionStatusColor = new(Colors.Gray);

        [ObservableProperty]
        private bool _isConnecting;

        [ObservableProperty]
        private bool _isConnected;

        public ICommand ConnectCommand { get; }
        public ICommand DisconnectCommand { get; }
        public ICommand ImportConfigCommand { get; }
        public ICommand CreateProfileCommand { get; }

        public MainWindowViewModel(
            Services.WireGuardService wireGuardService,
            Services.ConfigService configService,
            Services.LoggerService loggerService)
        {
            _wireGuardService = wireGuardService;
            _configService = configService;
            _loggerService = loggerService;

            InitializeNavigation();
            SetupCommands();
            SubscribeToEvents();
        }

        private void InitializeNavigation()
        {
            NavigationItems.Add(new NavigationItem
            {
                Icon = "📊",
                DisplayName = "Dashboard",
                Content = new Views.DashboardView()
            });

            NavigationItems.Add(new NavigationItem
            {
                Icon = "📝",
                DisplayName = "Profiles",
                Content = new Views.ProfilesView()
            });

            NavigationItems.Add(new NavigationItem
            {
                Icon = "🔗",
                DisplayName = "Nodes",
                Content = new Views.NodesView()
            });

            NavigationItems.Add(new NavigationItem
            {
                Icon = "📋",
                DisplayName = "Logs",
                Content = new Views.LogsView()
            });

            NavigationItems.Add(new NavigationItem
            {
                Icon = "⚙️",
                DisplayName = "Settings",
                Content = new Views.SettingsView()
            });

            // 默认选择 Dashboard
            SelectedNavigation = NavigationItems[0];
            
            // 绑定到 ViewModel
            ((DashboardView)NavigationItems[0].Content).DataContext = this;
            ((ProfilesView)NavigationItems[1].Content).DataContext = this;
            ((NodesView)NavigationItems[2].Content).DataContext = this;
            ((LogsView)NavigationItems[3].Content).DataContext = this;
            ((SettingsView)NavigationItems[4].Content).DataContext = this;
        }

        private void SetupCommands()
        {
            ConnectCommand = new AsyncRelayCommand(ExecuteConnectAsync, CanExecuteConnect);
            DisconnectCommand = new AsyncRelayCommand(ExecuteDisconnectAsync, CanExecuteDisconnect);
            ImportConfigCommand = new AsyncRelayCommand(ExecuteImportConfigAsync);
            CreateProfileCommand = new AsyncRelayCommand(ExecuteCreateProfileAsync);
        }

        private void SubscribeToEvents()
        {
            _wireGuardService.OnConnectionChanged += OnConnectionChanged;
            _loggerService.OnLogEntry += OnLogEntry;
        }

        private void OnConnectionChanged(ConnectionInfo info)
        {
            DispatcherHelper.Invoke(() =>
            {
                IsConnected = info.IsConnected;
                IsConnecting = info.IsConnecting;
                
                switch (info.State)
                {
                    case InterfaceState.Up:
                        ConnectionStatusText = "Connected";
                        ConnectionStatusColor = new SolidColorBrush(Colors.Green);
                        break;
                    case InterfaceState.Connecting:
                        ConnectionStatusText = "Connecting...";
                        ConnectionStatusColor = new SolidColorBrush(Colors.Blue);
                        break;
                    case InterfaceState.Error:
                        ConnectionStatusText = "Error";
                        ConnectionStatusColor = new SolidColorBrush(Colors.Red);
                        break;
                    default:
                        ConnectionStatusText = "Disconnected";
                        ConnectionStatusColor = new SolidColorBrush(Colors.Gray);
                        break;
                }

                CommandManager.InvalidateRequerySuggested();
            });
        }

        private void OnLogEntry(LogEntry entry)
        {
            // 日志已由 LogsView 直接订阅服务
        }

        [RelayCommand]
        private async Task ExecuteConnectAsync()
        {
            var profile = GetActiveProfile();
            if (profile == null)
            {
                _loggerService.Warning("No active profile to connect");
                return;
            }

            try
            {
                IsConnecting = true;
                var result = await _wireGuardService.Connect(profile);
                
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
            return !IsConnecting && !IsConnected && GetActiveProfile() != null;
        }

        [RelayCommand]
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

        private NetworkProfile? GetActiveProfile()
        {
            var profiles = _configService.GetAllProfiles();
            return profiles.FirstOrDefault(p => p.IsActive) ?? profiles.FirstOrDefault();
        }

        public void LoadData()
        {
            // 加载初始数据
            var profiles = _configService.GetAllProfiles();
            _loggerService.Information($"Loaded {profiles.Count} profiles");
        }

        private void ExecuteImportConfigAsync()
        {
            // 由 ProfilesView 处理
        }

        private void ExecuteCreateProfileAsync()
        {
            // 由 ProfilesView 处理
        }

        public void Dispose()
        {
            if (_isDisposed) return;
            _isDisposed = true;

            _wireGuardService.OnConnectionChanged -= OnConnectionChanged;
            _loggerService.OnLogEntry -= OnLogEntry;
        }
    }

    /// <summary>
    /// WPF Dispatcher 辅助类
    /// </summary>
    public static class DispatcherHelper
    {
        public static void Invoke(Action action)
        {
            Application.Current?.Dispatcher.Invoke(action);
        }

        public static async Task InvokeAsync(Func<Task> action)
        {
            if (Application.Current?.Dispatcher.CheckAccess() == true)
            {
                await action();
            }
            else
            {
                await Application.Current?.Dispatcher.InvokeAsync(async () => await action())!;
            }
        }
    }
}
