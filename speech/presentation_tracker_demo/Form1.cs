using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using presentation_tracker;
using System.Threading;

namespace presentation_tracker_demo
{
    public partial class Form1 : Form
    {
        Tracker tracker;
        Thread workerThread;
        public Form1()
        {
            tracker = new Tracker();
            InitializeComponent();
            tracker.Results += onResult;
            workerThread = new Thread(start);
        }

        private void start()
        {
            string topicsPath = textBox2.Text + @"\" + textBox3.Text+".xml";
            tracker.runsharp(topicsPath, textBox1.Text);
        }

        public void AppendTextBox(string text, string probs)
        {
            if (InvokeRequired)
            {
                this.Invoke(new Action<string,string>(AppendTextBox), new object[] { text, probs });
                return;
            }
            richTextBox1.Text += text + "\n";
            richTextBox1.Refresh();

            richTextBox2.Text = probs;
            richTextBox2.Refresh();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            button1.Enabled = false;
            workerThread.Start();
        }

        void onResult(object sender, Tracker.ResultEventArgs e)
        {
            AppendTextBox(e.text,e.probs);
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            tracker.stopSharp();
            if (workerThread.IsAlive)
                workerThread.Join();

        }

        
    }
}
