using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using speech_quality;

namespace speech_quality_demo
{
    public partial class Form1 : Form
    {
        Recorder testRecorder;
        string path = @"C:\Users\Reza\Documents\recorded_samples\";
        int duration = 10;
        int time = 0;

        public Form1()
        {
            
            InitializeComponent();
            testRecorder = new Recorder();
            button2.Enabled = false;
            testRecorder.Results += onResult;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            duration = int.Parse(textBox1.Text);
            testRecorder.run(checkBox1.Checked, duration, path);
            button1.Enabled = false;
            button2.Enabled = true;
        }

        private void button2_Click(object sender, EventArgs e)
        {
            testRecorder.stop();
            button2.Enabled = false;
            button1.Enabled = true;
        }

        void onResult(object sender, Recorder.ResultEventArgs e)
        {
            time += duration;
            richTextBox1.Text += "Time = "+time+"s\n";
            richTextBox1.Text +=  System.IO.File.ReadAllText(path+"log.txt");
        }
    }
}
