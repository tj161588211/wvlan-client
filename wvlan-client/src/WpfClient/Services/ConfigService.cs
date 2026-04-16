using System.Text;
using System.Text.Json;
using Serilog;
using WVLAN.Client.Models;

namespace WVLAN.Client.Services
{
    /// <summary>
    /// 配置服务 - 管理 WireGuard 配置文件的解析、保存和加载
    /// </summary>
    public class ConfigService
    {
        private readonly ILogger _logger;
        private readonly string _profilesPath;
        private readonly string _settingsPath;
        private readonly Dictionary<string, NetworkProfile> _loadedProfiles = new();

        public ConfigService(ILogger logger)
        {
            _logger = logger;
            
            var appDataPath = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
                "WVLAN");
            
            _profilesPath = Path.Combine(appDataPath, "profiles");
            _settingsPath = Path.Combine(appDataPath, "settings.json");
            
            Directory.CreateDirectory(_profilesPath);
            
            _logger.Information("ConfigService initialized. Profiles path: {Path}", _profilesPath);
        }

        /// <summary>
        /// 从 WireGuard 配置文件文本解析
        /// </summary>
        public ConfigParseResult ParseConfigText(string configText)
        {
            var result = new ConfigParseResult();
            
            try
            {
                var profile = new NetworkProfile
                {
                    ProfileName = $"Profile_{DateTime.Now:yyyyMMdd_HHmmss}",
                    CreatedAt = DateTime.Now,
                    UpdatedAt = DateTime.Now
                };

                string? currentSection = null;
                
                foreach (var line in configText.Split('\n'))
                {
                    var trimmed = line.Trim();
                    if (string.IsNullOrEmpty(trimmed) || trimmed.StartsWith('#'))
                        continue;

                    var parts = trimmed.Split('=', 2);
                    if (parts.Length != 2)
                        continue;

                    var key = parts[0].Trim().ToLower();
                    var value = parts[1].Trim();

                    switch (currentSection)
                    {
                        case "interface":
                            ParseInterfaceSection(profile.Interface, key, value);
                            break;
                        case "peer":
                            var peer = new PeerConfig();
                            ParsePeerSection(peer, key, value);
                            if (!string.IsNullOrEmpty(peer.PublicKey))
                            {
                                profile.Peers.Add(peer);
                            }
                            break;
                        default:
                            if (key == "[interface]")
                                currentSection = "interface";
                            else if (key == "[peer]")
                            {
                                currentSection = "peer";
                                // 创建新的 peer 对象，但不添加直到有 PublicKey
                            }
                            break;
                    }
                }

                // 验证配置
                if (string.IsNullOrEmpty(profile.Interface.PrivateKey))
                {
                    result.ErrorMessage = "Missing interface private key";
                    return result;
                }

                if (string.IsNullOrEmpty(profile.Interface.Address))
                {
                    result.ErrorMessage = "Missing interface address";
                    return result;
                }

                result.Success = true;
                result.Profile = profile;
                _logger.Information("Config parsed successfully: {ProfileName}", profile.ProfileName);
            }
            catch (Exception ex)
            {
                _logger.Error(ex, "Failed to parse config");
                result.ErrorMessage = $"Parse error: {ex.Message}";
            }

            return result;
        }

        /// <summary>
        /// 从文件解析配置
        /// </summary>
        public ConfigParseResult ParseConfigFile(string filePath)
        {
            try
            {
                var content = File.ReadAllText(filePath);
                var result = ParseConfigText(content);
                
                if (result.Success && result.Profile != null)
                {
                    // 从文件名获取 profile name
                    result.Profile.ProfileName = Path.GetFileNameWithoutExtension(filePath);
                }
                
                return result;
            }
            catch (Exception ex)
            {
                _logger.Error(ex, "Failed to read config file: {FilePath}", filePath);
                return new ConfigParseResult
                {
                    Success = false,
                    ErrorMessage = $"Failed to read file: {ex.Message}"
                };
            }
        }

        /// <summary>
        /// 保存配置到本地存储
        /// </summary>
        public OperationResult SaveProfile(NetworkProfile profile)
        {
            try
            {
                profile.UpdatedAt = DateTime.Now;
                
                var fileName = $"{SanitizeFileName(profile.ProfileName)}.json";
                var filePath = Path.Combine(_profilesPath, fileName);
                
                var json = JsonSerializer.Serialize(profile, new JsonSerializerOptions
                {
                    WriteIndented = true
                });
                
                File.WriteAllText(filePath, json);
                _loadedProfiles[profile.ProfileName] = profile;
                
                _logger.Information("Profile saved: {ProfileName} -> {FilePath}", profile.ProfileName, filePath);
                return OperationResult.SuccessResult();
            }
            catch (Exception ex)
            {
                _logger.Error(ex, "Failed to save profile: {ProfileName}", profile.ProfileName);
                return OperationResult.Failure(OperationStatus.InternalError, ex.Message);
            }
        }

        /// <summary>
        /// 从本地存储加载配置
        /// </summary>
        public NetworkProfile? LoadProfile(string profileName)
        {
            if (_loadedProfiles.TryGetValue(profileName, out var cached))
            {
                return cached;
            }

            try
            {
                var fileName = $"{SanitizeFileName(profileName)}.json";
                var filePath = Path.Combine(_profilesPath, fileName);
                
                if (!File.Exists(filePath))
                {
                    _logger.Warning("Profile not found: {ProfileName}", profileName);
                    return null;
                }
                
                var json = File.ReadAllText(filePath);
                var profile = JsonSerializer.Deserialize<NetworkProfile>(json);
                
                if (profile != null)
                {
                    _loadedProfiles[profileName] = profile;
                    _logger.Information("Profile loaded: {ProfileName}", profileName);
                }
                
                return profile;
            }
            catch (Exception ex)
            {
                _logger.Error(ex, "Failed to load profile: {ProfileName}", profileName);
                return null;
            }
        }

        /// <summary>
        /// 获取所有保存的配置列表
        /// </summary>
        public List<NetworkProfile> GetAllProfiles()
        {
            var profiles = new List<NetworkProfile>();
            
            try
            {
                foreach (var file in Directory.GetFiles(_profilesPath, "*.json"))
                {
                    try
                    {
                        var json = File.ReadAllText(file);
                        var profile = JsonSerializer.Deserialize<NetworkProfile>(json);
                        if (profile != null)
                        {
                            profiles.Add(profile);
                            _loadedProfiles[profile.ProfileName] = profile;
                        }
                    }
                    catch (Exception ex)
                    {
                        _logger.Warning(ex, "Failed to load profile from: {FilePath}", file);
                    }
                }
            }
            catch (Exception ex)
            {
                _logger.Error(ex, "Failed to enumerate profiles");
            }
            
            return profiles;
        }

        /// <summary>
        /// 删除配置
        /// </summary>
        public OperationResult DeleteProfile(string profileName)
        {
            try
            {
                var fileName = $"{SanitizeFileName(profileName)}.json";
                var filePath = Path.Combine(_profilesPath, fileName);
                
                if (File.Exists(filePath))
                {
                    File.Delete(filePath);
                }
                
                _loadedProfiles.Remove(profileName);
                
                _logger.Information("Profile deleted: {ProfileName}", profileName);
                return OperationResult.SuccessResult();
            }
            catch (Exception ex)
            {
                _logger.Error(ex, "Failed to delete profile: {ProfileName}", profileName);
                return OperationResult.Failure(OperationStatus.InternalError, ex.Message);
            }
        }

        /// <summary>
        /// 导出配置为标准 WireGuard 格式
        /// </summary>
        public string ExportToWireGuardFormat(NetworkProfile profile)
        {
            var sb = new StringBuilder();
            
            // Interface section
            sb.AppendLine("[Interface]");
            if (!string.IsNullOrEmpty(profile.Interface.PrivateKey))
                sb.AppendLine($"PrivateKey = {profile.Interface.PrivateKey}");
            if (!string.IsNullOrEmpty(profile.Interface.Address))
                sb.AppendLine($"Address = {profile.Interface.Address}");
            if (!string.IsNullOrEmpty(profile.Interface.DNS))
                sb.AppendLine($"DNS = {profile.Interface.DNS}");
            if (profile.Interface.ListenPort > 0)
                sb.AppendLine($"ListenPort = {profile.Interface.ListenPort}");
            
            sb.AppendLine();
            
            // Peer sections
            foreach (var peer in profile.Peers)
            {
                sb.AppendLine("[Peer]");
                if (!string.IsNullOrEmpty(peer.PublicKey))
                    sb.AppendLine($"PublicKey = {peer.PublicKey}");
                if (!string.IsNullOrEmpty(peer.Endpoint))
                    sb.AppendLine($"Endpoint = {peer.Endpoint}");
                if (!string.IsNullOrEmpty(peer.AllowedIPs))
                    sb.AppendLine($"AllowedIPs = {peer.AllowedIPs}");
                if (peer.PersistentKeepalive > 0)
                    sb.AppendLine($"PersistentKeepalive = {peer.PersistentKeepalive}");
                sb.AppendLine();
            }
            
            return sb.ToString();
        }

        /// <summary>
        /// 生成密钥对
        /// </summary>
        public (string privateKey, string publicKey) GenerateKeyPair()
        {
            // 使用 Cryptographic API 生成密钥
            using var rng = new System.Security.Cryptography.RNGCryptoServiceProvider();
            var privateKey = new byte[32];
            rng.GetBytes(privateKey);
            
            var privateKeyBase64 = Convert.ToBase64String(privateKey);
            
            // 注意：实际公钥需要从私钥计算，这里简化处理
            // 在实际应用中应该调用 WireGuard 的密钥生成函数
            var publicKeyBase64 = Convert.ToBase64String(SHA256.HashData(privateKey));
            
            return (privateKeyBase64, publicKeyBase64);
        }

        private void ParseInterfaceSection(InterfaceConfig config, string key, string value)
        {
            switch (key)
            {
                case "privatekey":
                    config.PrivateKey = value;
                    break;
                case "address":
                    config.Address = value;
                    break;
                case "dns":
                    config.DNS = value;
                    break;
                case "listenport":
                    if (int.TryParse(value, out var port))
                        config.ListenPort = port;
                    break;
            }
        }

        private void ParsePeerSection(PeerConfig config, string key, string value)
        {
            switch (key)
            {
                case "publickey":
                    config.PublicKey = value;
                    break;
                case "endpoint":
                    config.Endpoint = value;
                    break;
                case "allowedips":
                    config.AllowedIPs = value;
                    break;
                case "persistentkeepalive":
                    if (int.TryParse(value, out var keepalive))
                        config.PersistentKeepalive = keepalive;
                    break;
            }
        }

        private string SanitizeFileName(string name)
        {
            var invalidChars = Path.GetInvalidFileNameChars();
            return new string(name.Where(c => !invalidChars.Contains(c)).ToArray());
        }
    }
}
