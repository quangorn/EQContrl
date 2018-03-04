using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

namespace AstroMountConfigurator
{
    struct Point
    {
        public int time;
        public int motorStep;
        public double periodicError;
    }

    class PeriodicErrorCorrectionLoader
    {
        private Point[] points;
        private string inputFileName;
        public List<short> result { get; } = new List<short>();
        public uint multiplier { set; get; }

        public PeriodicErrorCorrectionLoader(string inputFileName)
        {
            this.inputFileName = inputFileName;
        }

        public void Load()
        {
            StringBuilder sb = new StringBuilder();
            using (var file = System.IO.File.OpenText(inputFileName))
            {
                List<Point> list = new List<Point>();
                Regex regex = new Regex("^(\\d+) (\\d+) (\\S+)$");
                while (!file.EndOfStream)
                {
                    String line = file.ReadLine();
                    Match m = regex.Match(line);
                    if (m.Success)
                    {
                        Point point;
                        point.time = Convert.ToInt32(m.Groups[1].Value);
                        point.motorStep = Convert.ToInt32(m.Groups[2].Value);
                        point.periodicError = Convert.ToDouble(m.Groups[3].Value);
                        list.Add(point);
                    }
                }
                points = list.ToArray();
            }

            double maxDelta = 0;
            for (int i = 0; i < points.Count(); i++)
            {
                double absDelta = Math.Abs(GetPoint(i - 1).periodicError - GetPoint(i).periodicError);
                if (absDelta > maxDelta)
                    maxDelta = absDelta;
            }
            multiplier = (uint)Math.Floor(short.MaxValue / maxDelta);
            for (int i = 0; i < points.Count(); i++)
            {
                double delta = GetPoint(i - 1).periodicError - GetPoint(i).periodicError;
                result.Add((short)Math.Round(delta * multiplier));
            }
        }

        private Point GetPoint(int index)
        {
            if (index < 0)
            {
                return points[index + points.Count()];
            }
            else if (index >= points.Count())
            {
                return points[index - points.Count()];
            }
            return points[index];
        }
    }
}
