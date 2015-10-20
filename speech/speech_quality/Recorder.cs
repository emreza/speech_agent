using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Timers;
using NAudio.Wave;
using System.Diagnostics;

namespace speech_quality
{
    public class Recorder
    {

        int nFiles = 0;
        bool recording = false;
        string outputPath;

        bool isMale;
        WaveFormat waveFormat;
        int fileDuration;
        WaveIn sourceStream;
        int bufferMilliseconds;
        const int fs = 16000;
        const int bits = 16;
        const int nChannels = 1;

        WaveFileWriter writer;

        public void run(bool male, int duration, string path)
        {
            fileDuration = duration;
            outputPath = path;
            bufferMilliseconds = fileDuration * 1000;
            isMale = male;
            waveFormat = new WaveFormat(fs, bits, nChannels);

            sourceStream = new WaveIn();
            sourceStream.BufferMilliseconds = bufferMilliseconds;
            sourceStream.WaveFormat = waveFormat;
            sourceStream.DataAvailable += sourceStreamDataAvailable;

            if (!recording)
            {
                sourceStream.StartRecording();
                recording = true;
            }

        }

        private void sourceStreamDataAvailable(object sender, WaveInEventArgs e)
        {
            string path = outputPath + "wavsam" + nFiles + ".wav";
            writer = new WaveFileWriter(path, waveFormat);
            writer.Write(e.Buffer, 0, e.Buffer.Length);
            writer.Flush();
            writer.Close();
            nFiles++;

            Process process = new Process();
            process.StartInfo.CreateNoWindow = true;
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.FileName = "praatcon.exe";
            String male;
            if (isMale) male = "yes"; else male = "no";
            process.StartInfo.Arguments = "extract_measures.praat " + path + " " + male;
           // process.StartInfo.RedirectStandardOutput = true;
             
            process.Start();
            
            process.WaitForExit();
            
            ResultEventArgs args = new ResultEventArgs();
      //      args.text = output;
            OnResults(args);
           // args.nWords = err.Length;
        }


        public void stop()
        {
            sourceStream.StopRecording();
            recording = false;
        }

        protected virtual void OnResults(ResultEventArgs e)
        {
            EventHandler<ResultEventArgs> handler = Results;
            if (handler != null)
            {
                handler(this, e);
            }
        }

        public event EventHandler<ResultEventArgs> Results;

        public class ResultEventArgs : EventArgs
        {
            public int nWords {get; set;}
            public string text { get; set; }
            double duration;
            double speakingRate, articulationRate;
            int nPauses, nFillers;
            double averagePause;
            double meanPitch, stdPitch, pitchRange;
            double meanIntensity;
            double[] fillerTimes;
        }
    }
}
