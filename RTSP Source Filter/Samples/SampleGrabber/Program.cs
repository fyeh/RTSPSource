/** @page samplegrabberpage SampleGrabber
 * The purpose of this program is to demonstrate how to use the RTSPSource filter
 * in a C# application.  SampleGrabber creates a DirectShow Graph using the RTSPFilter
 * as the source, then connecting it to  ColorSpace filter and a sample grabber.  The
 * sample grabber callback method BufferCB is called for each frame.  
 * 
 * The application consists of three components:
 *  1. Console window - A running trace log of the application
 *  2. Video window - A video viewer from the source requested in the RTSPsource URL
 *  3. Windows user interface - Provides valuable information regarding the status of your machine and the RTSPsource url that is rendering
 * 
 * This code is meant for demonstration purposes only, and should not be considered ready
 * for production.
 * 
 * Don't forget to add reference to DirectShowLib in your project.
*/

using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows.Forms;

namespace SampleGrabber
{
    /// <summary>
    /// Main application
    /// </summary>
    internal class Program
    {
        #region unmanaged
        // Declare the SetConsoleCtrlHandler function
        // as external and receiving a delegate.

        [DllImport("Kernel32")]
        private static extern bool SetConsoleCtrlHandler(HandlerRoutine handler, bool add);

        // A delegate type to be used as the handler routine
        // for SetConsoleCtrlHandler.
        private delegate bool HandlerRoutine(CtrlType ctrlType);

        // An enumerated type for the control messages
        // sent to the handler routine.
        public enum CtrlType
        {
            CtrlCEvent = 0,
            CtrlBreakEvent,
            CtrlCloseEvent,
            CtrlLogoffEvent = 5,
            CtrlShutdownEvent
        }

        #endregion

        private static EventHandler _processEvent;
        static HandlerRoutine _handler;
        static VideoStatus _dlg;
        private static ConsoleColor _startColor;
        /// <summary>
        /// Program Entry point.  Expects a command line argument of an 
        /// EndureSource URL.  If missing will prompt the user with a 
        /// windows dialog input box
        /// </summary>
        /// <param name="args">RTSP Source URL</param>
        [STAThread]
        private static void Main(string[] args)
        {
            _startColor = Console.ForegroundColor;
            try
            {
                Application.EnableVisualStyles();
                Application.SetCompatibleTextRenderingDefault(false);

                _handler = ConsoleCtrlCheck;
                _processEvent = Program_Exited;

                SetConsoleCtrlHandler(_handler, true);
                Process.GetCurrentProcess().Exited += _processEvent;
                string url = args.Length != 1 ? InputBox.GetUrl() : args[0];
                LogSuccess(url);
                if (string.IsNullOrEmpty(url))
                {
                    LogError("No RTSPsource url specified");
                    return;
                }
                bool restart = true;
                while (restart)
                {
                    using (_dlg = new VideoStatus())
                    {
                        _dlg.LoadUrl(url);
                        Application.Run(_dlg);
                    }
                    Thread.Sleep(5000);
                    restart = ViewModel.Get().AutoRestart;
                }
            }
            catch (Exception ex)
            {
                LogError("Error: " + ex.Message);
                LogError("-------------------------------");
                LogError("Program Terminating");
            }
            finally
            {
                Console.ForegroundColor = _startColor;
            }
            if (_dlg != null && !_dlg.IsDisposed)
                _dlg.Dispose();

            LogError("Program ended");
        }

        /// <summary>
        /// Catch the shutdown requests and exit gracefully
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        static void Program_Exited(object sender, EventArgs e)
        {
            if (_dlg != null && !_dlg.IsDisposed)
            {
                _dlg.BeginInvoke(new MethodInvoker(_dlg.Close));
            }
            LogError("Program ended");
        }

        /// <summary>
        /// Catch the shutdown requests and exit gracefully
        /// </summary>
        /// <param name="ctrltype"></param>
        /// <returns></returns>
        private static bool ConsoleCtrlCheck(CtrlType ctrltype)
        {
            if (_dlg != null && !_dlg.IsDisposed)
            {
                _dlg.BeginInvoke(new MethodInvoker(_dlg.Close));
            }else
            {
                Trace.TraceWarning("Dialog disposed, force exit");
                Application.Exit();
            }
            LogError("Program ended");
            return true;
        }

        /// <summary>
        /// Show a message on the command window in red
        /// </summary>
        /// <param name="line"></param>
        /// <param name="args"></param>
        public static void LogError(string line, params object[] args)
        {
            ConsoleColor current = Console.ForegroundColor;
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine(String.Format(line, args));
            Console.ForegroundColor = current;
        }

        /// <summary>
        /// Show a message in the command window in green
        /// </summary>
        /// <param name="line"></param>
        /// <param name="args"></param>
        public static void LogSuccess(string line, params object[] args)
        {
            ConsoleColor current = Console.ForegroundColor;
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine(String.Format(line, args));
            Console.ForegroundColor = current;
        }

    }
}