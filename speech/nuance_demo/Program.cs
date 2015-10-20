using System;
using System.IO;
using System.Net;
using System.Text;

 public class NuanceDemo
    {
        public static void Main()
        {
            // Create a request using a URL that can receive a post. 
            String baseurl = "https://dictation.nuancemobility.net/NMDPAsrCmdServlet/dictation";
            String appid = "NMDPTRIAL_asadi_ccs_neu_edu20151014101926";
            String appkey = "ca949b8bb9f6e3b23d1c7fe6bc0f4bc007326b07f46dbeee9a93acbf5a959f24293aafe23193956eaf14546b24a02b2bd29632c704f1b59e84615edef6fd021f";
            String id = "di1";
            String fullurl = baseurl + "?appId=" + appid + "&appKey=" + appkey + "&id=" + id;
            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(fullurl);
            // Set the Method property of the request to POST.
            request.Method = "POST";
            request.Headers["X-Dictation-NBestListSize"] = "10";
            request.Headers["X-Dictation-AudioSource"] = "SpeakerAndMicrophone";
           // request.Headers["Accept"] = "application/xml";
            request.ContentType = "audio/x-wav;codec=pcm;bit=16;rate=16000";
            String filepath = @"C:\Users\Reza\Documents\GitHub\speech_agent\samples\ha2.wav";
            
           byte[] byteArray = File.ReadAllBytes(filepath);
          
          //request.ContentLength = byteArray.Length;
         // request.ServicePoint.Expect100Continue = false;
            // Get the request stream.
          request.SendChunked = true;
           Stream dataStream = request.GetRequestStream();
            // Write the data to the request stream.
           dataStream.Write(byteArray, 0, byteArray.Length);
            // Close the Stream object.
           dataStream.Close();
            // Get the response.
           // request.ProtocolVersion = HttpVersion.Version11;
          //  request.KeepAlive = false;
            
          //  request.Timeout = -1;
            WebResponse response = request.GetResponse();
            // Display the status.
            Console.WriteLine(((HttpWebResponse)response).StatusDescription);
            // Get the stream containing content returned by the server.
            // Open the stream using a StreamReader for easy access.
            String responseString;
            using (StreamReader responseStream = new StreamReader(response.GetResponseStream(), Encoding.UTF8))
            {
                responseString = responseStream.ReadToEnd();
                Console.WriteLine(responseString);
                responseStream.Close();
            }

            response.Close();
        }
    }