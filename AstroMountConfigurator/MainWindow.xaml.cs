using System.Windows;
using WrapperLibrary;
using System.ComponentModel;

namespace AstroMountConfigurator
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window,  INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        private Config _config = new Config();

        public Config Config
        {
            get
            {
                return _config;
            }
            set
            {
                _config = value;
            }
        }

        public MainWindow()
        {
            InitializeComponent();
            this.DataContext = this;
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
                res = Connector.ReadConfig(_config);
                OnPropertyChanged(new PropertyChangedEventArgs("Config"));
                //this.StatusText.Text = $"Read complete: {config}";
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

                _config = new Config();
                OnPropertyChanged(new PropertyChangedEventArgs("Config"));
            }
        }

        private void WriteButton_Click(object sender, RoutedEventArgs e)
        {
            //var config = new Config();
            //config.AxisConfigs[0].MaxFreq = 1000;
            //config.AxisConfigs[0].MaxSpeed = 100;
            //config.AxisConfigs[0].Microsteps = 10;
            //config.AxisConfigs[1].MaxFreq = 2000;
            //config.AxisConfigs[1].MaxSpeed = 200;
            //config.AxisConfigs[1].Microsteps = 20;
            var res = Connector.WriteConfig(_config);
            this.StatusText.Text = $"Write complete: {res}";
        }

        private void OnPropertyChanged(PropertyChangedEventArgs e)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, e);
            }
        }
    }
}
