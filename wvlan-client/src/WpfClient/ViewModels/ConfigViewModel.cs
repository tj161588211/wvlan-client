using System.Collections.ObjectModel;
using System.Windows.Input;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Microsoft.Win32;
using WVLAN.Client.Models;

namespace WVLAN.Client.ViewModels
{
    /// <summary>
    /// 配置管理 ViewModel
    /// </summary>
    public partial class ConfigViewModel : ObservableObject
    {
        private readonly Services.ConfigService _configService;
        private readonly Services.LoggerService _loggerService;

        [ObservableProperty]
        private ObservableCollection<NetworkProfile> _profiles = new();

        [ObservableProperty]
        private NetworkProfile? _selectedProfile;

        [ObservableProperty]
        private bool _isEditing;

        [ObservableProperty]
        private NetworkProfile? _editingProfile;

        public ICommand ImportCommand { get; }
        public ICommand CreateCommand { get; }
        public ICommand EditCommand { get; }
        public ICommand SaveCommand { get; }
        public ICommand CancelEditCommand { get; }
        public ICommand DeleteCommand { get; }
        public ICommand SetActiveCommand { get; }
        public ICommand GenerateKeysCommand { get; }

        public ConfigViewModel(
            Services.ConfigService configService,
            Services.LoggerService loggerService)
        {
            _configService = configService;
            _loggerService = loggerService;

            ImportCommand = new AsyncRelayCommand(ExecuteImportAsync);
            CreateCommand = new AsyncRelayCommand(ExecuteCreateAsync);
            EditCommand = new RelayCommand(ExecuteEdit);
            SaveCommand = new AsyncRelayCommand(ExecuteSaveAsync);
            CancelEditCommand = new RelayCommand(ExecuteCancelEdit);
            DeleteCommand = new AsyncRelayCommand(ExecuteDeleteAsync);
            SetActiveCommand = new RelayCommand(ExecuteSetActive);
            GenerateKeysCommand = new RelayCommand(ExecuteGenerateKeys);
        }

        public void LoadProfiles()
        {
            Profiles.Clear();
            var profiles = _configService.GetAllProfiles();
            foreach (var profile in profiles)
            {
                Profiles.Add(profile);
            }
            _loggerService.Information($"Loaded {Profiles.Count} profiles");
        }

        private async Task ExecuteImportAsync()
        {
            var dialog = new OpenFileDialog
            {
                Filter = "WireGuard Config|*.conf|All Files|*.*",
                Title = "Import WireGuard Configuration"
            };

            if (dialog.ShowDialog() == true)
            {
                try
                {
                    var result = _configService.ParseConfigFile(dialog.FileName);
                    
                    if (result.Success && result.Profile != null)
                    {
                        // 检查是否已存在
                        if (Profiles.Any(p => p.ProfileName == result.Profile.ProfileName))
                        {
                            result.Profile.ProfileName += $"_{DateTime.Now:yyyyMMdd_HHmmss}";
                        }

                        var saveResult = _configService.SaveProfile(result.Profile);
                        
                        if (saveResult.IsSuccess)
                        {
                            Profiles.Add(result.Profile);
                            _loggerService.Information($"Profile imported: {result.Profile.ProfileName}");
                        }
                        else
                        {
                            _loggerService.Error($"Failed to save profile: {saveResult.ErrorMessage}");
                        }
                    }
                    else
                    {
                        _loggerService.Error($"Parse failed: {result.ErrorMessage}");
                        await MessageBoxHelper.ShowError("Import Failed", result.ErrorMessage ?? "Unknown error");
                    }
                }
                catch (Exception ex)
                {
                    _loggerService.Error(ex, "Import error");
                    await MessageBoxHelper.ShowError("Import Error", ex.Message);
                }
            }
        }

        private async Task ExecuteCreateAsync()
        {
            // 创建新的空白配置
            var newProfile = new NetworkProfile
            {
                ProfileName = $"New_Profile_{DateTime.Now:HHmmss}",
                Interface = new InterfaceConfig
                {
                    ListenPort = 51820,
                    DNS = "8.8.8.8,8.8.4.4"
                },
                Peers = new List<PeerConfig>(),
                CreatedAt = DateTime.Now,
                UpdatedAt = DateTime.Now
            };

            ExecuteEdit(newProfile);
        }

        private void ExecuteEdit(NetworkProfile? profile = null)
        {
            if (profile == null && SelectedProfile == null)
                return;

            EditingProfile = profile ?? SelectedProfile!.Copy();
            IsEditing = true;
        }

        private async Task ExecuteSaveAsync()
        {
            if (EditingProfile == null)
                return;

            try
            {
                var result = _configService.SaveProfile(EditingProfile);
                
                if (result.IsSuccess)
                {
                    // 更新或添加到列表
                    var existing = Profiles.FirstOrDefault(p => p.ProfileName == EditingProfile.ProfileName);
                    if (existing != null)
                    {
                        var index = Profiles.IndexOf(existing);
                        Profiles[index] = EditingProfile;
                    }
                    else
                    {
                        Profiles.Add(EditingProfile);
                    }

                    IsEditing = false;
                    EditingProfile = null;
                    _loggerService.Information($"Profile saved: {EditingProfile.ProfileName}");
                }
                else
                {
                    await MessageBoxHelper.ShowError("Save Failed", result.ErrorMessage ?? "Unknown error");
                }
            }
            catch (Exception ex)
            {
                _loggerService.Error(ex, "Save error");
                await MessageBoxHelper.ShowError("Save Error", ex.Message);
            }
        }

        private void ExecuteCancelEdit()
        {
            IsEditing = false;
            EditingProfile = null;
        }

        private async Task ExecuteDeleteAsync()
        {
            if (SelectedProfile == null)
                return;

            var confirm = await MessageBoxHelper.ShowConfirm(
                "Delete Profile",
                $"Are you sure you want to delete \"{SelectedProfile.ProfileName}\"?");

            if (!confirm)
                return;

            try
            {
                var result = _configService.DeleteProfile(SelectedProfile.ProfileName);
                
                if (result.IsSuccess)
                {
                    Profiles.Remove(SelectedProfile);
                    SelectedProfile = null;
                    _loggerService.Information($"Profile deleted: {SelectedProfile.ProfileName}");
                }
                else
                {
                    await MessageBoxHelper.ShowError("Delete Failed", result.ErrorMessage ?? "Unknown error");
                }
            }
            catch (Exception ex)
            {
                _loggerService.Error(ex, "Delete error");
                await MessageBoxHelper.ShowError("Delete Error", ex.Message);
            }
        }

        private void ExecuteSetActive()
        {
            if (SelectedProfile == null)
                return;

            foreach (var profile in Profiles)
            {
                profile.IsActive = (profile == SelectedProfile);
            }

            _loggerService.Information($"Active profile set to: {SelectedProfile.ProfileName}");
        }

        private void ExecuteGenerateKeys()
        {
            if (EditingProfile?.Interface == null)
                return;

            var (privateKey, publicKey) = Bridge.BridgeWrapper.GenerateKeyPair();
            
            EditingProfile.Interface.PrivateKey = privateKey;
            EditingProfile.Interface.PublicKey = publicKey;
            
            _loggerService.Information("Key pair generated");
        }

        public static async Task<bool> ShowError(string title, string message)
        {
            await DispatcherHelper.InvokeAsync(() =>
            {
                System.Windows.MessageBox.Show(message, title, 
                    System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Error);
            });
            return false;
        }

        public static async Task<bool> ShowConfirm(string title, string message)
        {
            bool result = false;
            await DispatcherHelper.InvokeAsync(() =>
            {
                var boxResult = System.Windows.MessageBox.Show(message, title,
                    System.Windows.MessageBoxButton.YesNo, System.Windows.MessageBoxImage.Question);
                result = boxResult == System.Windows.MessageBoxResult.Yes;
            });
            return result;
        }
    }
}
