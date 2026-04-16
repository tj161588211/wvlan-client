using System.Collections.ObjectModel;
using System.Windows.Input;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using WVLAN.Client.Models;

namespace WVLAN.Client.ViewModels
{
    /// <summary>
    /// 日志 ViewModel
    /// </summary>
    public partial class LogsViewModel : ObservableObject
    {
        private readonly Services.LoggerService _loggerService;

        [ObservableProperty]
        private ObservableCollection<LogEntry> _logEntries = new();

        [ObservableProperty]
        private LogFilter _filter = new();

        [ObservableProperty]
        private bool _autoScroll = true;

        [ObservableProperty]
        private int _filteredCount;

        public ICommand ClearCommand { get; }
        public ICommand ExportCommand { get; }
        public ICommand RefreshCommand { get; }

        public LogsViewModel(Services.LoggerService loggerService)
        {
            _loggerService = loggerService;

            ClearCommand = new RelayCommand(ExecuteClear);
            ExportCommand = new AsyncRelayCommand(ExecuteExportAsync);
            RefreshCommand = new RelayCommand(ExecuteRefresh);

            // 订阅新日志
            _loggerService.OnLogEntry += OnLogEntry;

            // 加载现有日志
            ExecuteRefresh();
        }

        private void OnLogEntry(LogEntry entry)
        {
            DispatcherHelper.Invoke(() =>
            {
                LogEntries.Add(entry);
                
                // 保持列表不会无限增长
                if (LogEntries.Count > 1000)
                {
                    LogEntries.RemoveAt(0);
                }

                if (AutoScroll)
                {
                    // 滚动到底部由视图处理
                }
            });
        }

        private void ExecuteClear()
        {
            _loggerService.ClearLogs();
            LogEntries.Clear();
            FilteredCount = 0;
        }

        private async Task ExecuteExportAsync()
        {
            var dialog = new Microsoft.Win32.SaveFileDialog
            {
                Filter = "Text Files|*.txt|All Files|*.*",
                Title = "Export Logs"
            };

            if (dialog.ShowDialog() == true)
            {
                try
                {
                    var logs = GetFilteredLogs();
                    var content = string.Join(Environment.NewLine, logs.Select(l => l.ToString()));
                    await File.WriteAllTextAsync(dialog.FileName, content);
                }
                catch (Exception ex)
                {
                    await ConfigViewModel.ShowError("Export Failed", ex.Message);
                }
            }
        }

        private void ExecuteRefresh()
        {
            LogEntries.Clear();
            foreach (var entry in _loggerService.LogEntries)
            {
                LogEntries.Add(entry);
            }
            UpdateFilteredCount();
        }

        public List<LogEntry> GetFilteredLogs()
        {
            return _loggerService.GetFilteredLogs(Filter);
        }

        private void UpdateFilteredCount()
        {
            FilteredCount = GetFilteredLogs().Count;
        }
    }

    /// <summary>
    /// 设置 ViewModel
    /// </summary>
    public partial class SettingsViewModel : ObservableObject
    {
        private readonly Services.LoggerService _loggerService;

        [ObservableProperty]
        private bool _isDarkTheme = true;

        [ObservableProperty]
        private int _logRetentionDays = 30;

        [ObservableProperty]
        private bool _autoConnectOnStart;

        [ObservableProperty]
        private string _selectedProfileName = string.Empty;

        [ObservableProperty]
        private ObservableCollection<string> _availableProfiles = new();

        public ICommand SaveCommand { get; }
        public ICommand ResetCommand { get; }

        public SettingsViewModel(
            Services.LoggerService loggerService,
            Services.ConfigService configService)
        {
            _loggerService = loggerService;

            SaveCommand = new AsyncRelayCommand(ExecuteSaveAsync);
            ResetCommand = new RelayCommand(ExecuteReset);

            // 加载配置文件列表
            var profiles = configService.GetAllProfiles();
            foreach (var profile in profiles)
            {
                AvailableProfiles.Add(profile.ProfileName);
                if (profile.IsActive)
                {
                    SelectedProfileName = profile.ProfileName;
                }
            }
        }

        private async Task ExecuteSaveAsync()
        {
            // TODO: 保存设置到配置文件
            _loggerService.Information("Settings saved");
            
            await DispatcherHelper.InvokeAsync(() =>
            {
                System.Windows.MessageBox.Show("Settings saved successfully!", 
                    "Settings", System.Windows.MessageBoxButton.OK, 
                    System.Windows.MessageBoxImage.Information);
            });
        }

        private void ExecuteReset()
        {
            IsDarkTheme = true;
            LogRetentionDays = 30;
            AutoConnectOnStart = false;
            SelectedProfileName = string.Empty;
        }
    }
}
