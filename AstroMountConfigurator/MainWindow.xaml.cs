using System.Windows;
using WrapperLibrary;
using System.ComponentModel;
using System.Threading;
using System.IO;
using System;
using Microsoft.Win32;

namespace AstroMountConfigurator
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window, INotifyPropertyChanged
    {
        private string encoderCalibrationFolder = Directory.CreateDirectory("EncoderCalibration").FullName;
        private string encoderTestFileName;

        public event PropertyChangedEventHandler PropertyChanged;

        private Config config { get; set; } = new Config();

        private EncoderCorrection correction;

        public Config Config
        {
            get
            {
                return config;
            }
            set
            {
                config = value;
            }
        }

        private int _encoder_speed { get; set; } = 5;

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
            var res = Connector.ReadConfig(config);
            if (res == Status.OK)
            {
                this.ConnectButton.IsEnabled = false;
                this.DisconnectButton.IsEnabled = true;
                this.WriteButton.IsEnabled = true;
                OnPropertyChanged(new PropertyChangedEventArgs("Config"));
                this.StatusText.Text = $"Read complete, dividers: {config.AxisConfigs[0].MicrostepsDivider} {config.AxisConfigs[1].MicrostepsDivider}";
            }
            else
            {
                this.StatusText.Text = $"Read failed: {res}";
            }
        }

        private void DisconnectButton_Click(object sender, RoutedEventArgs e)
        {
            var res = Connector.Disconnect();
            this.StatusText.Text = $"Disconnect result: {res}";
            this.ConnectButton.IsEnabled = true;
            this.DisconnectButton.IsEnabled = false;
            this.WriteButton.IsEnabled = false;

            config = new Config();
            OnPropertyChanged(new PropertyChangedEventArgs("Config"));
        }

        private void WriteButton_Click(object sender, RoutedEventArgs e)
        {
            var res = Connector.WriteConfig(config);
            this.StatusText.Text = $"Write complete: {res}";
        }

        private void EncoderStartButton_Click(object sender, RoutedEventArgs e)
        {
            var res = Connector.StartRA_Motor(EncoderSpeed);
            if (res != Status.OK)
            {
                this.StatusText.Text = $"Start RA error: {res}";
                return;
            }
            if (res == Status.OK)
            {
                _thread = new Thread(() =>
                {
                    Thread.CurrentThread.IsBackground = true;
                    try
                    {
                        DateTime date = DateTime.Now;
                        encoderTestFileName = String.Format("{0}\\test_{1:yyyy_MM_dd_HH_mm_ss}.txt", encoderCalibrationFolder, date);
                        using (StreamWriter file = new StreamWriter(encoderTestFileName))
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

                this.EncoderStartButton.IsEnabled = false;
                this.EncoderStopButton.IsEnabled = true;
                this.EncoderCalculateCorrectionButton.IsEnabled = false;
                this.EncoderFindCorrectionFileButton.IsEnabled = false;
                this.EncoderWriteCorrectionButton.IsEnabled = false;
                this.EncoderCorrectionFilePath.Text = null;
            }
            this.StatusText.Text = "RA motor started";
        }

        private void EncoderStopButton_Click(object sender, RoutedEventArgs e)
        {
            this.EncoderStartButton.IsEnabled = true;
            this.EncoderStopButton.IsEnabled = false;
            _thread.Interrupt();
            _thread.Join();
            var res = Connector.StopRA_Motor();
            this.EncoderCorrectionFilePath.Text = encoderTestFileName;
            this.EncoderFindCorrectionFileButton.IsEnabled = true;
            this.EncoderCalculateCorrectionButton.IsEnabled = true;
            this.StatusText.Text = $"Stop RA complete: {res}";
        }

        private void EncoderFindCorrectionFileButton_Click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            openFileDialog.Filter = "Text files (*.txt)|*.txt|All files (*.*)|*.*";
            openFileDialog.InitialDirectory = encoderCalibrationFolder;
            if (openFileDialog.ShowDialog() == true)
            {
                this.EncoderCorrectionFilePath.Text = openFileDialog.FileName;
                this.EncoderCalculateCorrectionButton.IsEnabled = true;
            }
        }

        private void EncoderCalculateCorrectionButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                var calc = new EncoderCorrectionCalculator(this.EncoderCorrectionFilePath.Text);
                var message = calc.Calculate();
                this.StatusText.Text = message;
                correction = calc.result;
                this.EncoderWriteCorrectionButton.IsEnabled = true;
            }
            catch (Exception exc)
            {
                this.StatusText.Text = $"Calculate correction error: {exc.Message}";
            }
        }

        private void EncoderWriteCorrectionButton_Click(object sender, RoutedEventArgs e)
        {
            var res = Connector.WriteEncoderCorrection(correction);
            if (res != Status.OK)
            {
                this.StatusText.Text = $"Write correction error: {res}";
                return;
            }
            var readCorrection = new EncoderCorrection();
            res = Connector.ReadEncoderCorrection(readCorrection);
            if (res != Status.OK)
            {
                this.StatusText.Text = $"Read correction error: {res}";
                return;
            }
            if (!correction.Equals(readCorrection))
            {
                this.StatusText.Text = "Verify correction error";
                return;
            }
            this.StatusText.Text = "Write and verify correction OK";
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
