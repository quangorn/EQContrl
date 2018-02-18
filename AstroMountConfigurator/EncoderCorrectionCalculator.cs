using System;
using System.Collections.Generic;
using System.Linq;
using WrapperLibrary;
using System.Text.RegularExpressions;
using System.Text;

namespace AstroMountConfigurator
{
    struct MeasurePoint
    {
        public long time;
        public short x;
        public short y;
        public double atanValue;
    }

    struct TimeInterval
    {
        public int startIndex;
        public int endIndex;
        public long startTime;
        public long duration;

        public TimeInterval(int startIndex, int endIndex, long startTime, long duration)
        {
            this.startIndex = startIndex;
            this.endIndex = endIndex;
            this.startTime = startTime;
            this.duration = duration;
        }
    }

    struct CorrectionInfo
    {
        public double sum;
        public int count;
    }

    class EncoderCorrectionCalculator
    {
        private string inputFileName;
        private int tableSize;
        private const int tableLevels = 0x10000;
        private MeasurePoint[] points;
        private List<TimeInterval> timeIntervals;
        private CorrectionInfo[] correctionTable;
        public EncoderCorrection result { get; }

        public EncoderCorrectionCalculator(string inputFileName)
        {
            this.inputFileName = inputFileName;
            result = new EncoderCorrection();
            timeIntervals = new List<TimeInterval>();
            tableSize = result.Data.Length;
            correctionTable = new CorrectionInfo[tableSize];
        }

        public string Calculate()
        {
            StringBuilder sb = new StringBuilder();
            using (var file = System.IO.File.OpenText(inputFileName))
            {
                List<MeasurePoint> list = new List<MeasurePoint>();
                Regex regex = new Regex("(\\d+);(-?\\d+);(-?\\d+);");
                while (!file.EndOfStream)
                {
                    String line = file.ReadLine();
                    Match m = regex.Match(line);
                    if (m.Success)
                    {
                        MeasurePoint point;
                        point.time = Convert.ToInt64(m.Groups[1].Value);
                        point.x = Convert.ToInt16(m.Groups[2].Value);
                        point.y = Convert.ToInt16(m.Groups[3].Value);
                        point.atanValue = 0;
                        list.Add(point);

                        if (point.x > result.MaxX)
                            result.MaxX = point.x;
                        else if (point.x < result.MinX)
                            result.MinX = point.x;
                        if (point.y > result.MaxY)
                            result.MaxY = point.y;
                        else if (point.y < result.MinY)
                            result.MinY = point.y;
                    }
                    else
                    {
                        //throw new Exception($"Unable to parse line: {line}");
                        break;
                    }
                }
                points = list.ToArray();
            }

            double xRange = (double)(result.MaxX - result.MinX) / 2;
            double xOffset = (double)(result.MaxX + result.MinX) / 2;
            double yRange = (double)(result.MaxY - result.MinY) / 2;
            double yOffset = (double)(result.MaxY + result.MinY) / 2;
            for (int i = 0; i < points.Count(); i++)
            {
                points[i].atanValue = Math.Atan2((points[i].y - yOffset) / yRange, (points[i].x - xOffset) / xRange);
            }

            bool intervalEnded = true;
            int startIndex = 0;
            int size = points.Count();
            for (int i = 0; i < size - 1; i++)
            {
                if (points[i].atanValue > 2.5 && points[i + 1].atanValue < -2.5) //find interval end
                {
                    if (intervalEnded)
                    {
                        intervalEnded = false;
                        AddInterval(startIndex, i);
                        startIndex = i + 1;
                    }
                    else
                    {
                        startIndex = i + 1;
                    }
                }
                if (!intervalEnded && Math.Abs(points[i].atanValue) < 2)
                {
                    intervalEnded = true;
                }
            }
            if (intervalEnded)
            {
                AddInterval(startIndex, size - 1);
            }

            if (timeIntervals.Count() < 3)
            {
                throw new Exception("No full period detected");
            }

            double meanDuration = 0;
            for (int i = 1; i < timeIntervals.Count() - 1; i++)
            {
                meanDuration += timeIntervals[i].duration;
            }
            meanDuration /= timeIntervals.Count() - 2;
            var ts = new TimeSpan((long)meanDuration);
            sb.Append($"Full periods: {timeIntervals.Count() - 2}, mean duration: {ts.TotalSeconds:0.00}s");

            for (int i = 0; i < timeIntervals.Count(); i++)
            {
                var interval = timeIntervals[i];
                double startTime = i == 0 ? (interval.duration - meanDuration) : interval.startTime;
                double intervalDuration = (i == 0 || i == timeIntervals.Count() - 1) ? meanDuration : interval.duration;
                double itemDuration = intervalDuration / tableSize;
                for (int j = interval.startIndex; j <= interval.endIndex; j++)
                {
                    var point = points[j];
                    int index = (int)((point.time - startTime) / itemDuration);
                    if (index >= tableSize)
                    {
                        index = tableSize - 1;
                    }
                    correctionTable[index].count++;
                    correctionTable[index].sum += point.atanValue;
                }
            }

            double[] curvePoints = new double[correctionTable.Count()];
            for (int i = 0; i < correctionTable.Count(); i++)
            {
                curvePoints[i] = correctionTable[i].sum / correctionTable[i].count;
            }

            ushort lastTableValue = 0;
            for (int i = 0; i < curvePoints.Count(); i++)
            {
                //Raw
                //double value = curvePoints[i];

                ////Bezier 3
                //double value = (GetPoint(curvePoints, i - 1) + GetPoint(curvePoints, i + 1)) / 4 + GetPoint(curvePoints, i) / 2;

                ////Bezier 5
                //double value = (GetPoint(curvePoints, i - 2) + GetPoint(curvePoints, i + 2)) / 16 +
                //    (GetPoint(curvePoints, i - 1) + GetPoint(curvePoints, i + 1)) / 4 +
                //    GetPoint(curvePoints, i) * 3 / 8;

                ////Bezier 7
                double value = (GetPoint(curvePoints, i - 3) + GetPoint(curvePoints, i + 3)) / 64 +
                    (GetPoint(curvePoints, i - 2) + GetPoint(curvePoints, i + 2)) * 3 / 32 +
                    (GetPoint(curvePoints, i - 1) + GetPoint(curvePoints, i + 1)) * 15 / 64 +
                    GetPoint(curvePoints, i) * 5 / 16;

                ushort tableValue = (ushort)((value + Math.PI) * tableLevels / (2 * Math.PI));
                if (tableValue > lastTableValue)
                {
                    lastTableValue = tableValue;
                }
                //else
                //{
                //    Console.WriteLine($"Current value less than last: {tableValue} < {lastTableValue}");
                //}
                result.Data[i] = lastTableValue;
            }

            return sb.ToString();
        }

        private void AddInterval(int startIndex, int endIndex)
        {
            timeIntervals.Add(new TimeInterval(startIndex, endIndex, points[startIndex].time, points[endIndex].time - points[startIndex].time));
            //Console.WriteLine($"New interval: from {startIndex} to {endIndex}, duration: {timeIntervals.Last().duration}");
        }

        private static double GetPoint(double[] points, int index)
        {
            if (index < 0)
            {
                return points[index + points.Count()] - 2 * Math.PI;
            }
            else if (index >= points.Count())
            {
                return points[index - points.Count()] + 2 * Math.PI;
            }
            return points[index];
        }
    }
}
