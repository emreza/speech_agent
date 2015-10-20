using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Xml;

namespace presentation_tracker
{
    public class Tracker
    {
        [DllImport("kw_spotter.dll")]
        public static extern int run(listenCallback callback, string kpath);
        [DllImport("kw_spotter.dll")]
        public static extern void stop();

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void listenCallback(IntPtr utter_hyp);

        string topicsPath = @"italy4.xml";
        string kwsPath = @"tracker.kws";

        const int maxNTopics = 10;
        const int maxNkws = 20;
        int nTopics = 0;
        int[] totalScores = new int[maxNTopics];
        int[] scores = new int[maxNTopics];
        bool[] topicDone = new bool[maxNTopics];
        bool[] kwUsed = new bool[maxNkws * maxNTopics];
        int[] kwScores = new int[maxNkws * maxNTopics];
        int[] topicIndices = new int[maxNkws * maxNTopics];
        double maxScore = 0;
        int maxIndex = -1;
        double scoreThreshold = 0.5;

        Dictionary<string, int> kwsDictionary = new Dictionary<string, int>();

        void initTopics()
        {
            int topicNum = 0;
            int kwIndex = 0;
            int totalScore = 0;
            int score = 0;
            int wordThresh = 0;

            XmlTextReader reader = new XmlTextReader(topicsPath);
            using (System.IO.StreamWriter kwsFile = new System.IO.StreamWriter(kwsPath))
            {
                while (reader.Read())
                {
                    switch (reader.NodeType)
                    {
                        case XmlNodeType.Element: // The node is an element.
                            switch (reader.Name)
                            {
                                case "keyword":
                                    reader.MoveToNextAttribute();
                                    score = Int16.Parse(reader.Value);
                                    break;
                                case "text":
                                    reader.MoveToNextAttribute();
                                    wordThresh = Int16.Parse(reader.Value);
                                    break;
                            }
                            break;
                        case XmlNodeType.Text: // keyword text
                            string kw = reader.Value.Trim().ToLowerInvariant();
                            if (!kwsDictionary.ContainsKey(kw))
                            {
                                kwsDictionary.Add(kw, kwIndex);
                                kwsFile.WriteLine(kw + " /1e-" + wordThresh + "/");
                            }
                            break;
                        case XmlNodeType.EndElement:
                            switch (reader.Name)
                            {
                                case "keyword":
                                    kwUsed[kwIndex] = false;
                                    kwScores[kwIndex] = score;
                                    topicIndices[kwIndex] = topicNum;
                                    kwIndex++;
                                    totalScore+= score;
                                    break;
                                case "topic":
                                    totalScores[topicNum] = totalScore;
                                    scores[topicNum] = 0;
                                    topicDone[topicNum] = false;
                                    topicNum++;
                                    totalScore = 0;
                                    break;
                            }
                            break;
                    }

                }
                nTopics = topicNum;
            }
        }

        public void runsharp(string tpath, string kpath)
        {
            this.topicsPath = tpath;
            this.kwsPath = kpath;
            initTopics();
            listenCallback callback = new listenCallback(kwsCallback);
            run(callback, kpath);
        }

        public void stopSharp()
        {
            stop();
        }

        void kwsCallback(IntPtr utter_hyp)
        {
            String result = Marshal.PtrToStringAnsi(utter_hyp);
            int kwIndex = kwsDictionary[result];
            int topicInd = topicIndices[kwIndex];
            if (!topicDone[topicInd] && !kwUsed[kwIndex])
            {
                kwUsed[kwIndex] = true;
                scores[topicInd] += kwScores[kwIndex];
                double prob = (float)scores[topicInd] / totalScores[topicInd];
                if (prob > maxScore)
                {
                    maxScore = prob;
                    maxIndex = topicInd;
                }
            }

            string probs = "";
            for (int topicNumber = 0; topicNumber < nTopics; topicNumber++)
            {
                int p = 100 * scores[topicNumber] / totalScores[topicNumber];
                probs += "Topic " + topicNumber.ToString() + " probability: " + p.ToString() + "%\n";
            }

            ResultEventArgs args = new ResultEventArgs();
            args.text = result;
            args.probs = probs;
            OnResults(args);
            if (maxScore > scoreThreshold)
            {
                args.text = maxIndex.ToString() + " " + maxScore.ToString();
                topicDone[maxIndex] = true;
                OnResults(args);
                maxScore = 0;
            }
        }

        protected virtual void OnResults(ResultEventArgs e1)
        {
            EventHandler<ResultEventArgs> handler = Results;
            if (handler != null)
            {
                handler(this, e1);
            }
        }

        public event EventHandler<ResultEventArgs> Results;

        public class ResultEventArgs : EventArgs
        {
            public string text { get; set; }
            public string probs { get; set; }
        }
    }
}
