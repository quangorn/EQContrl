using System.Windows;
using WrapperLibrary;
using System.ComponentModel;
using System.Threading;
using System.IO;
using System;

namespace AstroMountConfigurator
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window, INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        private Config _config { get; set; } = new Config();

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

        private int _encoder_speed { get; set; } = 3;

        public int EncoderSpeed
        {
            get
            {
                return _encoder_speed;
            }
            set
            {
                _encoder_speed = value;
            }
        }

        private Thread _thread;

        public MainWindow()
        {
            InitializeComponent();
            this.DataContext = this;
        }

        private void ConnectButton_Click(object sender, RoutedEventArgs e)
        {
            var res = Connector.Connect();
            this.StatusText.Text = $"Connect result: {res};";
            if (res == Status.OK)
            {
                this.ConnectButton.IsEnabled = false;
                this.DisconnectButton.IsEnabled = true;
                this.WriteButton.IsEnabled = true;
                res = Connector.ReadConfig(_config);
                OnPropertyChanged(new PropertyChangedEventArgs("Config"));
                this.StatusText.Text = $"Read complete, dividers: {_config.AxisConfigs[0].MicrostepsDivider} {_config.AxisConfigs[1].MicrostepsDivider}";
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
            var res = Connector.WriteConfig(_config);
            this.StatusText.Text = $"Write complete: {res}";
        }

        private void EncoderStartButton_Click(object sender, RoutedEventArgs e)
        {
            var res = Connector.StartRA_Motor(EncoderSpeed);
            if (res == Status.OK)
            {
                this.EncoderStartButton.IsEnabled = false;
                this.EncoderStopButton.IsEnabled = true;
                _thread = new Thread(() =>
                {
                    Thread.CurrentThread.IsBackground = true;
                    try
                    {
                        DateTime date = DateTime.Now;
                        using (StreamWriter file = new StreamWriter(String.Format("test_{0:yyyy_MM_dd_HH_mm_ss}.txt", date)))
                        {
                            Thread.Sleep(2000);
                            date = DateTime.Now;
                            int x = 0, y = 0;
                            for (int i = 0; true; i++)
                            {
                                var status = Connector.GetEncoderValues(ref x, ref y);
                                if (status != Status.OK)
                                {
                                    //this.StatusText.Text = $"Get encoder values error: {status}";
                                    return;
                                }                                
                                file.WriteLine(String.Format("{0};{1};{2};", DateTime.Now.Subtract(date).Ticks, x, y));
                                Thread.Sleep(10);
                            }
                        }
                    } catch (ThreadInterruptedException)
                    {
                        return;
                    }
                });
                _thread.Start();
            }
            this.StatusText.Text = $"Start RA speed = {EncoderSpeed} complete: {res}";
        }

        private void EncoderStopButton_Click(object sender, RoutedEventArgs e)
        {
            var res = Connector.StopRA_Motor();
            if (res == Status.OK)
            {
                this.EncoderStartButton.IsEnabled = true;
                this.EncoderStopButton.IsEnabled = false;
                _thread.Interrupt();
                _thread.Join();
            }
            this.StatusText.Text = $"Stop RA complete: {res}";
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
