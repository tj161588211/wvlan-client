using System.Windows;

namespace WVLAN.Client
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {
        private readonly Services.WireGuardService _wireGuardService;
        private readonly Services.ConfigService _configService;
        private readonly Services.LoggerService _loggerService;

        public MainWindow(
            Services.WireGuardService wireGuardService,
            Services.ConfigService configService,
            Services.LoggerService loggerService)
        {
            InitializeComponent();
            
            _wireGuardService = wireGuardService;
            _configService = configService;
            _loggerService = loggerService;

            // 设置 DataContext
            var viewModel = new ViewModels.MainWindowViewModel(
                wireGuardService, configService, loggerService);
            DataContext = viewModel;

            // 加载初始数据
            viewModel.LoadData();
        }

        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            base.OnClosing(e);
            
            // 通知 ViewModel 清理
            if (DataContext is ViewModels.MainWindowViewModel viewModel)
            {
                viewModel.Dispose();
            }
        }
    }
}
