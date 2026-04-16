using System;
using System.Windows;
using Serilog;
using WVLAN.Client.Services;

namespace WVLAN.Client
{
    /// <summary>
    /// WPF 应用入口
    /// </summary>
    public partial class App : Application
    {
        private static ILogger? _logger;
        private static LoggerService? _loggerService;
        private static WireGuardService? _wireGuardService;
        private static ConfigService? _configService;

        public static ILogger Logger => _logger ?? throw new InvalidOperationException("Logger not initialized");
        public static LoggerService LoggerService => _loggerService ?? throw new InvalidOperationException("LoggerService not initialized");
        public static WireGuardService WireGuardService => _wireGuardService ?? throw new InvalidOperationException("WireGuardService not initialized");
        public static ConfigService ConfigService => _configService ?? throw new InvalidOperationException("ConfigService not initialized");

        private void Application_Startup(object sender, StartupEventArgs e)
        {
            try
            {
                // 初始化日志服务
                InitializeLogging();
                
                Logger.Information("===============================");
                Logger.Information("WVLAN Client Starting...");
                Logger.Information("Version: 1.0.0");
                Logger.Information("===============================");

                // 检查管理员权限
                if (!IsRunningAsAdmin())
                {
                    Logger.Warning("Application is not running with administrator privileges.");
                    MessageBox.Show(
                        "WVLAN Client requires administrator privileges to operate network interfaces.\n\n" +
                        "Please restart the application as Administrator.",
                        "Administrator Required",
                        MessageBoxButton.OK,
                        MessageBoxImage.Warning);
                    Shutdown(1);
                    return;
                }

                // 初始化服务
                _loggerService = new LoggerService(Logger);
                _configService = new ConfigService(Logger);
                _wireGuardService = new WireGuardService(Logger, _configService);

                Logger.Information("Services initialized successfully.");

                // 启动主窗口
                var mainWindow = new MainWindow(_wireGuardService, _configService, _loggerService);
                mainWindow.Show();
            }
            catch (Exception ex)
            {
                Logger.Fatal(ex, "Failed to start application");
                MessageBox.Show(
                    $"Fatal error during startup: {ex.Message}\n\n{ex.StackTrace}",
                    "Startup Error",
                    MessageBoxButton.OK,
                    MessageBoxImage.Error);
                Shutdown(1);
            }
        }

        private void Application_Exit(object sender, ExitEventArgs e)
        {
            try
            {
                Logger.Information("WVLAN Client shutting down...");
                
                // 清理资源
                _wireGuardService?.Dispose();
                _logger?.Information("Application exited normally.");
                _logger?.Close();
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Error during shutdown: {ex}");
            }
        }

        private void InitializeLogging()
        {
            string logPath = System.IO.Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
                "WVLAN", "logs", "client.log");

            System.IO.Directory.CreateDirectory(System.IO.Path.GetDirectoryName(logPath)!);

            _logger = new LoggerConfiguration()
                .MinimumLevel.Debug()
                .WriteTo.Console(outputTemplate: "[{Timestamp:HH:mm:ss} {Level:u3}] {Message:lj}{NewLine}{Exception}")
                .WriteTo.File(logPath, rollingInterval: RollingInterval.Day, 
                    outputTemplate: "{Timestamp:yyyy-MM-dd HH:mm:ss.fff zzz} [{Level:u3}] {Message:lj}{NewLine}{Exception}")
                .CreateLogger();
        }

        private bool IsRunningAsAdmin()
        {
            using var identity = System.Security.Principal.WindowsIdentity.GetCurrent();
            var principal = new System.Security.Principal.WindowsPrincipal(identity);
            return principal.IsInRole(System.Security.Principal.WindowsBuiltInRole.Administrator);
        }
    }
}
