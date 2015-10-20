using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace keywordSpotterDemo
{
    class Program
    {
        [DllImport("kw_spotter.dll")]
        public static extern int run(listenCallback callback, string kpath);
        [DllImport("kw_spotter.dll")]
        public static extern void stop();
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void listenCallback(IntPtr utter_hyp);

        //string topicsPath = @"C:\Users\Reza\Documents\GitHub\speech_agent\speech\Release\italy4.xml";
        static string kwsPath = @"C:\Users\Reza\Documents\GitHub\speech_agent\speech\Release\tracker.kws";

        static void Main(string[] args)
        {
            listenCallback callback = new listenCallback(kwsCallback);
            run(callback, kwsPath);
        }

        static void kwsCallback(IntPtr utter_hyp)
        {
            //String result = Marshal.PtrToStringAnsi(utter_hyp);
        }
    }
}
