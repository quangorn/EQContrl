using System.Windows;
using WrapperLibrary;

namespace AstroMountConfigurator
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private void ConnectButton_Click(object sender, RoutedEventArgs e)
        {
            var res = Connector.Connect();
            this.StatusText.Text = $"Connect result: {res}";
            if (res == Status.OK)
            {
                this.ConnectButton.IsEnabled = false;
                this.DisconnectButton.IsEnabled = true;
                this.WriteButton.IsEnabled = true;
                var config = Connector.GetConfig();
                this.StatusText.Text = $"Read complete: {config}";
            }
        }

        private void DisconnectButton_Click(object sender, RoutedEventArgs e)
        {
            var res = Connector.Disconnect();
            this.StatusText.Text = $"Disconnect result: {res}";
            if (res == Status.OK)
            {
                this.ConnectButton.IsEnabled = true;
                this.DisconnectButton.IsEnabled = false;
                this.WriteButton.IsEnabled = false;
            }
        }

        private void WriteButton_Click(object sender, RoutedEventArgs e)
        {
            var config = new Config();
            config.AxisConfigs[0].MaxFreq = 1000;
            config.AxisConfigs[0].MaxSpeed = 100;
            config.AxisConfigs[0].Microsteps = 10;
            config.AxisConfigs[1].MaxFreq = 2000;
            config.AxisConfigs[1].MaxSpeed = 200;
            config.AxisConfigs[1].Microsteps = 20;
            var res = Connector.WriteConfig(config);
            this.StatusText.Text = $"Write complete: {res}";
        }
    }
}
