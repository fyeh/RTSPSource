// IBM Confidential
//
// OCO Source Material
//
// 5725H94
//
// (C) Copyright IBM Corp. 2005,2006
//
// The source code for this program is not published or otherwise divested
// of its trade secrets, irrespective of what has been deposited with the
// U. S. Copyright Office.
//
// US Government Users Restricted Rights - Use, duplication or
// disclosure restricted by GSA ADP Schedule Contract with
// IBM Corp.

using System;
using System.Diagnostics;
using System.IO;
using System.IO.Pipes;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

namespace SampleGrabber
{
        internal sealed class ConsoleRedirector : IDisposable
        {
            private static readonly Log log = LogManager.getLog(typeof(ConsoleRedirector));
            private static ConsoleRedirector _instance;

            public static void attach(ILogListener listener)
            {
                Debug.Assert(null == _instance);
                _instance = new ConsoleRedirector(listener);
            }

            public static void detatch()
            {
                _instance.Dispose();
                _instance = null;
            }

            public static bool isAttached
            {
                get
                {
                    return null != _instance;
                }
            }

            //----------------------------------------------------------------------

            private const int PERIOD = 500;
            private const int BUFFER_SIZE = 4096;
            private volatile bool _isDisposed;
            private readonly ILogListener _logListener;
            private readonly IntPtr _stdout;
            private readonly IntPtr _stderr;
            private readonly Mutex _sync;
            private readonly Timer _timer;
            private readonly char[] _buffer;
            private readonly AnonymousPipeServerStream _outServer;
            private readonly AnonymousPipeServerStream _errServer;
            private readonly TextReader _outClient;
            private readonly TextReader _errClient;

            // ReSharper disable UseObjectOrCollectionInitializer
            private ConsoleRedirector(ILogListener logListener)
            {
                log.info("Redirecting stdout and stderr to log");

                bool ret;
                AnonymousPipeClientStream client;

                _logListener = logListener;
                _stdout = GetStdHandle(STD_OUTPUT_HANDLE);
                _stderr = GetStdHandle(STD_ERROR_HANDLE);
                _sync = new Mutex();
                _buffer = new char[BUFFER_SIZE];

                _outServer = new AnonymousPipeServerStream(PipeDirection.Out);
                client = new AnonymousPipeClientStream(PipeDirection.In, _outServer.ClientSafePipeHandle);
                //client.ReadTimeout = 0;
                Debug.Assert(_outServer.IsConnected);
                _outClient = new StreamReader(client, Encoding.Default);
                ret = SetStdHandle(STD_OUTPUT_HANDLE, _outServer.SafePipeHandle.DangerousGetHandle());
                Debug.Assert(ret);

                _errServer = new AnonymousPipeServerStream(PipeDirection.Out);
                client = new AnonymousPipeClientStream(PipeDirection.In, _errServer.ClientSafePipeHandle);
                //client.ReadTimeout = 0;
                Debug.Assert(_errServer.IsConnected);
                _errClient = new StreamReader(client, Encoding.Default);
                ret = SetStdHandle(STD_ERROR_HANDLE, _errServer.SafePipeHandle.DangerousGetHandle());
                Debug.Assert(ret);

                Thread outThread = new Thread(doRead);
                Thread errThread = new Thread(doRead);
                outThread.Name = "stdout logger";
                errThread.Name = "stderr logger";
                outThread.Start(_outClient);
                errThread.Start(_errClient);
                _timer = new Timer(flush, null, PERIOD, PERIOD);
            }
            // ReSharper restore UseObjectOrCollectionInitializer

            private void flush(object state)
            {
                _outServer.Flush();
                _errServer.Flush();
            }

            private void doRead(object clientObj)
            {
                TextReader client = (TextReader)clientObj;
                try
                {
                    while (true)
                    {
                        int read = client.Read(_buffer, 0, BUFFER_SIZE);
                        if (read > 0)
                            _logListener.writeChars(_buffer, 0, read);
                    }
                }
                catch (ObjectDisposedException)
                {
                    // Pipe was closed... terminate
                }
            }

            public void Dispose()
            {
                Dispose(true);
            }

            ~ConsoleRedirector()
            {
                Dispose(false);
            }

            // ReSharper disable InconsistentNaming
            // ReSharper disable UnusedParameter.Local
            // ReSharper disable EmptyGeneralCatchClause
            private void Dispose(bool disposing)
            {
                if (!_isDisposed)
                {
                    lock (_sync)
                    {
                        if (!_isDisposed)
                        {
                            _isDisposed = true;
                            _timer.Change(Timeout.Infinite, Timeout.Infinite);
                            _timer.Dispose();
                            flush(null);

                            try { SetStdHandle(STD_OUTPUT_HANDLE, _stdout); }
                            catch (Exception) { }
                            _outClient.Dispose();
                            _outServer.Dispose();

                            try { SetStdHandle(STD_ERROR_HANDLE, _stderr); }
                            catch (Exception) { }
                            _errClient.Dispose();
                            _errServer.Dispose();
                        }
                    }
                }
            }
            // ReSharper restore EmptyGeneralCatchClause
            // ReSharper restore UnusedParameter.Local
            // ReSharper restore InconsistentNaming

            // ReSharper disable InconsistentNaming
            private const int STD_OUTPUT_HANDLE = -11;
            private const int STD_ERROR_HANDLE = -12;

            [DllImport("kernel32.dll")]
            [return: MarshalAs(UnmanagedType.Bool)]
            private static extern bool SetStdHandle(int nStdHandle, IntPtr hHandle);

            [DllImport("kernel32.dll")]
            private static extern IntPtr GetStdHandle(int nStdHandle);
            // ReSharper restore InconsistentNaming
        }
    }
}
