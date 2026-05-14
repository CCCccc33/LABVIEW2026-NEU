using System;
using System.Diagnostics;
using System.Speech.Recognition;
using System.Windows.Forms;

namespace VoiceRecognitionDll
{
    public class VoiceRecognizer : IDisposable
    {
        private SpeechRecognitionEngine recognitionEngine;
        private bool isRunning;

        public string LastRecognizedText { get; private set; }

        public string LastResponseText { get; private set; }

        public void Init()
        {
            Stop();
            DisposeEngine();

            recognitionEngine = new SpeechRecognitionEngine();

            Choices choices = new Choices();
            choices.Add(new string[] { "你好", "下午好", "中午好", "晚上好", "分析", "音乐" });

            GrammarBuilder grammarBuilder = new GrammarBuilder(choices);
            Grammar grammar = new Grammar(grammarBuilder);

            recognitionEngine.LoadGrammar(grammar);
            recognitionEngine.SetInputToDefaultAudioDevice();
            recognitionEngine.SpeechRecognized += RecognitionEngineSpeechRecognized;
        }

        public void Start()
        {
            if (recognitionEngine == null)
            {
                Init();
            }

            if (isRunning)
            {
                return;
            }

            recognitionEngine.RecognizeAsync(RecognizeMode.Multiple);
            isRunning = true;
        }

        public void Stop()
        {
            if (recognitionEngine == null || !isRunning)
            {
                return;
            }

            recognitionEngine.RecognizeAsyncStop();
            isRunning = false;
        }

        public void Dispose()
        {
            Stop();
            DisposeEngine();
        }

        private void RecognitionEngineSpeechRecognized(object sender, SpeechRecognizedEventArgs e)
        {
            LastRecognizedText = e.Result.Text;

            switch (e.Result.Text)
            {
                case "你好":
                    ShowResponse("你好啊");
                    break;
                case "下午好":
                    ShowResponse("下午好呀");
                    break;
                case "中午好":
                    ShowResponse("中午好呀");
                    break;
                case "晚上好":
                    ShowResponse("晚上好呀");
                    break;
                case "分析":
                    ShowResponse("好的");
                    break;
                case "音乐":
                    ShowResponse("正在为您打开网易云");
                    OpenNetEaseCloudMusic();
                    break;
            }
        }

        private void ShowResponse(string responseText)
        {
            LastResponseText = responseText;
            MessageBox.Show(responseText);
        }

        private void OpenNetEaseCloudMusic()
        {
            try
            {
                Process.Start("cloudmusic://");
            }
            catch
            {
                // If NetEase Cloud Music is not registered on this computer, keep the speech response only.
            }
        }

        private void DisposeEngine()
        {
            if (recognitionEngine == null)
            {
                return;
            }

            recognitionEngine.SpeechRecognized -= RecognitionEngineSpeechRecognized;
            recognitionEngine.Dispose();
            recognitionEngine = null;
            isRunning = false;
        }
    }
}
