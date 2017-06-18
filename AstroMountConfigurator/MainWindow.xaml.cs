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
            var config = Connector.GetConfig();
            this.StatusText.Text = $"Write complete: {config}";
        }
    }
}
