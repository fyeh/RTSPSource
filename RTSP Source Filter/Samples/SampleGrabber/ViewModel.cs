
using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Globalization;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows.Forms;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using DirectShowLib;
using HelpLib;
using Microsoft.Win32;
using SampleGrabber.Controls;

namespace SampleGrabber
{
    #region Video guids
    [ComVisible(false)]
    internal class MediaTypes
    {
        public static readonly Guid Video = new Guid(0x73646976, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
        public static readonly Guid Interleaved = new Guid(0x73766169, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
        public static readonly Guid Audio = new Guid(0x73647561, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
        public static readonly Guid Text = new Guid(0x73747874, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
        public static readonly Guid Stream = new Guid(0xE436EB83, 0x524F, 0x11CE, 0x9F, 0x53, 0x00, 0x20, 0xAF, 0x0B, 0xA7, 0x70);
    }

    // ReSharper disable InconsistentNaming
    [ComVisible(false)]
    internal class MediaSubTypes
    {
        public static readonly Guid UYVY = new Guid(0x55595659, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
        public static readonly Guid YUYV = new Guid(0x56595559, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
        public static readonly Guid IYUV = new Guid(0x56555949, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
        public static readonly Guid DVSD = new Guid(0x44535644, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
        public static readonly Guid RGB1 = new Guid(0xE436EB78, 0x524F, 0x11CE, 0x9F, 0x53, 0x00, 0x20, 0xAF, 0x0B, 0xA7, 0x70);
        public static readonly Guid RGB4 = new Guid(0xE436EB79, 0x524F, 0x11CE, 0x9F, 0x53, 0x00, 0x20, 0xAF, 0x0B, 0xA7, 0x70);
        public static readonly Guid RGB8 = new Guid(0xE436EB7A, 0x524F, 0x11CE, 0x9F, 0x53, 0x00, 0x20, 0xAF, 0x0B, 0xA7, 0x70);
        public static readonly Guid RGB565 = new Guid(0xE436EB7B, 0x524F, 0x11CE, 0x9F, 0x53, 0x00, 0x20, 0xAF, 0x0B, 0xA7, 0x70);
        public static readonly Guid RGB555 = new Guid(0xE436EB7C, 0x524F, 0x11CE, 0x9F, 0x53, 0x00, 0x20, 0xAF, 0x0B, 0xA7, 0x70);
        public static readonly Guid RGB24 = new Guid(0xE436Eb7D, 0x524F, 0x11CE, 0x9F, 0x53, 0x00, 0x20, 0xAF, 0x0B, 0xA7, 0x70);
        public static readonly Guid RGB32 = new Guid(0xE436EB7E, 0x524F, 0x11CE, 0x9F, 0x53, 0x00, 0x20, 0xAF, 0x0B, 0xA7, 0x70);
        public static readonly Guid Avi = new Guid(0xE436EB88, 0x524F, 0x11CE, 0x9F, 0x53, 0x00, 0x20, 0xAF, 0x0B, 0xA7, 0x70);
        public static readonly Guid Asf = new Guid(0x3DB80F90, 0x9412, 0x11D1, 0xAD, 0xED, 0x00, 0x00, 0xF8, 0x75, 0x4B, 0x99);
    }
    // ReSharper restore InconsistentNaming
    #endregion

    /// <summary>
    /// The viewmodel for the SampleGrabber application.
    /// ViewModel is a singleton object that encapsulates the business
    /// logic for the samplegrabber application.  We made ViewModel a singleton
    /// becuase samplegrabber itself is essentially a singleton application
    /// allowing only one RTSPsource url at a time.  Furthermore, it is
    /// cleaner to use a singleton that pass object reference all around.
    /// </summary>
    public class ViewModel : INotifyPropertyChanged, IDisposable
    {
        #region members
        private static readonly object LockObj = new object();
        private static ViewModel _instance;
        private static SampleGrabberCallback _capGrabber;
        private IGraphBuilder _graph;
        private ISampleGrabber _grabber;
        private string _url;
        DsROTEntry rot = null;

        #endregion

        /// <summary>
        /// Constructor
        /// </summary>
        private ViewModel()
        {
            var app = new System.Windows.Application();
            AutoRestart = false;
            StartTime = DateTime.Now.ToString("g");
            Log = new ObservableCollection<string>();
            CaptureImageCommand = new RelayCommand(DoCaptureImage, CanCaptureImage);
            PlayCommand = new RelayCommand(DoPlay, CanPlay);
            PauseCommand = new RelayCommand(DoPause, CanPause);
            RestartCommand = new RelayCommand(DoRestart, CanRestart);
            StopCommand = new RelayCommand(DoStop, CanStop);
            DefaultSettings = new ObservableCollection<DefaultSetting>();
        }

        /// <summary>
        /// Singleton interface
        /// </summary>
        /// <returns></returns>
        public static ViewModel Get()
        {
            lock (LockObj)
            {
                if (_instance == null)
                {
                    _instance = new ViewModel();
                }
            }
            return _instance;
        }

        /// <summary>
        /// Main entry point, kicks off a
        /// background thread to construct and run the graph
        /// Start should only be called once per session
        /// </summary>
        /// <param name="url">RTSPsource url</param>
        public void Start(string url)
        {
            var t = new Thread(StartAsync);
            t.Start(url);
        }

        /// <summary>
        /// The background thread entry point
        /// this is where the graph is created and run from
        /// </summary>
        /// <param name="arg"></param>
        private void StartAsync(object arg)
        {
            try
            {
                Trace("Starting ...");
                _url = arg.ToString();
                Trace(_url);

                Trace("Checking Status");
                CheckSystemStatus();

                Trace("Create Sample Grabber Callback");
                _capGrabber = new SampleGrabberCallback();

                Trace("Building Graph");
                _graph = (IGraphBuilder)new FilterGraph();

                
                BuildGraph(Graph, _url);

                InitializeSampleGrabber();

                //debug
                rot = new DsROTEntry(Graph);

                Trace("Running Graph");
                int hr = MediaControl.Run();
                CheckHr(hr, "Can't run the graph");
                RunGraph();
            }
            catch (Exception ex)
            {
                ExceptionHelper.HandleException(ex);
                Trace("ERROR: Creating Graph");
            }
        }

        /// <summary>
        /// Actualy run the graph
        /// </summary>
        private void RunGraph()
        {
            try
            {
                Running = true;
                while (Running)
                {
                    Thread.Sleep(500);
                    EventCode ev;
                    IntPtr p1, p2;
                    Application.DoEvents();
                    while (MediaEvent.GetEvent(out ev, out p1, out p2, 0) == 0)
                    {
                        if (ev == EventCode.Complete || ev == EventCode.UserAbort || ev == EventCode.ErrorAbort)
                        {
                            if (ev == EventCode.ErrorAbort)
                            {
                                string msg = DecodeHrResult((uint)p1.ToInt64());
                                Program.LogError("ERROR: HRESULT={0:X} {1}", p1, msg);
                                Trace("ERROR: HRESULT={0:X} {1}", p1, msg);
                            }
                            MediaControl.Stop();
                            Running = false;
                            Trace("Done!");
                            Program.LogSuccess("Done!");
                        }
                        MediaEvent.FreeEventParams(ev, p1, p2);
                    }
                }
                Graph.Abort();
                Trace("Graph Closed");
            }
            catch (Exception ex)
            {
                ExceptionHelper.HandleException(ex);
                Trace("ERROR: Running Graph");
            }
        }

        /// <summary>
        /// Setup the smaple grabber with the width and height
        /// of the expect image stream
        /// </summary>
        private void InitializeSampleGrabber()
        {
            var mediaType = new AMMediaType { majorType = MediaTypes.Video, subType = MediaSubTypes.RGB32 };
            _grabber.GetConnectedMediaType(mediaType);
            if (mediaType.formatType == FormatType.VideoInfo && mediaType.formatPtr != IntPtr.Zero)
            {
                var header = (VideoInfoHeader)Marshal.PtrToStructure(mediaType.formatPtr, typeof(VideoInfoHeader));

                if (header != null && header.BmiHeader != null)
                {
                    FrameWidth = header.BmiHeader.Width;
                    FrameHeight = header.BmiHeader.Height;
                    // Get the pixel count
                    var pcount = (uint)(FrameWidth * FrameHeight * PixelFormats.Bgr32.BitsPerPixel / 8);
                    _capGrabber.Initialize(pcount, FrameWidth, FrameHeight);
                }
            }
        }

        /// <summary>
        /// Helper function to send output to the UI trace log
        /// </summary>
        /// <param name="fmt"></param>
        /// <param name="args"></param>
        private void Trace(string fmt, params object[] args)
        {
            System.Windows.Application.Current.Dispatcher.BeginInvoke(new MethodInvoker(() =>
                Log.Add(string.Format(fmt, args)
            )));
            RaisePropertyChanged("Log");
        }

        #region Commands
        #region Capture Image
        /// <summary>
        /// Capture image command
        /// </summary>
        public RelayCommand CaptureImageCommand { get; set; }

        /// <summary>
        /// Determines whether the CaptureImage command can be executed
        /// </summary>
        /// <param name="sender">Sender</param>
        private bool CanCaptureImage(object sender)
        {
            // Check if there is a valid webcam
            return Running && _capGrabber.Bitmap != null;
        }

        /// <summary>
        /// Invoked when the CaptureImage command is executed
        /// </summary>
        /// <param name="sender">Sender</param>
        private void DoCaptureImage(object sender)
        {
            if (_capGrabber.Bitmap != null)
            {
                var bitmap = _capGrabber.Bitmap.Clone();
                if (bitmap != null)
                {
                    CapturedImage = bitmap.Clone();
                }
            }
        }

        #endregion
        #region Play
        /// <summary>
        /// Play video command
        /// </summary>
        public RelayCommand PlayCommand { get; set; }

        /// <summary>
        /// Determines whether the play command can be executed
        /// </summary>
        /// <param name="sender">Sender</param>
        private bool CanPlay(object sender)
        {
            // Check if there is a valid webcam
            return Running && _capGrabber.Bitmap != null;
        }

        /// <summary>
        /// Invoked when the Play command is executed
        /// </summary>
        /// <param name="sender">Sender</param>
        private void DoPlay(object sender)
        {
            MediaControl.Run();
        }

        #endregion
        #region Pause
        /// <summary>
        /// Play video command
        /// </summary>
        public RelayCommand PauseCommand { get; set; }

        /// <summary>
        /// Determines whether the Pause command can be executed
        /// </summary>
        /// <param name="sender">Sender</param>
        private bool CanPause(object sender)
        {
            // Check if there is a valid webcam
            return Running && _capGrabber.Bitmap != null;
        }

        /// <summary>
        /// Invoked when the Pause command is executed
        /// </summary>
        /// <param name="sender">Sender</param>
        private void DoPause(object sender)
        {
            MediaControl.Pause();
        }

        #endregion
        #region Stop
        /// <summary>
        /// Stop video command
        /// </summary>
        public RelayCommand StopCommand { get; set; }

        /// <summary>
        /// Determines whether the Stop command can be executed
        /// </summary>
        /// <param name="sender">Sender</param>
        private bool CanStop(object sender)
        {
            // Check if there is a valid webcam
            return Running && _capGrabber.Bitmap != null;
        }

        /// <summary>
        /// Invoked when the Pause command is executed
        /// </summary>
        /// <param name="sender">Sender</param>
        private void DoStop(object sender)
        {
            MediaControl.Stop();
            Graph.Abort();
        }

        #endregion
        #region Restart
        /// <summary>
        /// Play video command
        /// </summary>
        public RelayCommand RestartCommand { get; set; }

        /// <summary>
        /// Determines whether the Pause command can be executed
        /// </summary>
        /// <param name="sender">Sender</param>
        private bool CanRestart(object sender)
        {
            // Check if there is a valid webcam
            return Running && _capGrabber.Bitmap != null;
        }

        /// <summary>
        /// Invoked when the Pause command is executed
        /// </summary>
        /// <param name="sender">Sender</param>
        private void DoRestart(object sender)
        {
            AutoRestart = true;
            MediaControl.Stop();
            Graph.Abort();
        }

        #endregion
        #endregion

        #region Properties
        /// <summary>
        /// The actual direct show graph
        /// </summary>
        public IGraphBuilder Graph { get { return _graph; } }
        /// <summary>
        /// The media control interface for the graph
        /// </summary>
        public IMediaControl MediaControl { get { return (IMediaControl)Graph; } }
        /// <summary>
        /// The media event interface for the graph
        /// </summary>
        public IMediaEvent MediaEvent { get { return (IMediaEvent)Graph; } }

        #region Graph Status Properties
        /// <summary>
        /// Is the graph running
        /// </summary>
        public bool Running
        {
            get { return _running; }
            set
            {
                _running = value; RaisePropertyChanged("Running");
                System.Windows.Application.Current.Dispatcher.BeginInvoke(
                    new MethodInvoker(UpdateCommands));
            }
        }
        private bool _running;

        /// <summary>
        /// Was an error detected constructing the graph
        /// </summary>
        public bool Fault
        {
            get { return _fault; }
            set { _fault = value; RaisePropertyChanged("Fault"); }
        }
        private bool _fault;

        /// <summary>
        /// The RTSP filter has been created
        /// </summary>
        public bool Filter
        {
            get { return _filter; }
            set { _filter = value; RaisePropertyChanged("Filter"); }
        }
        private bool _filter;


        /// <summary>
        /// The color converter has been created
        /// </summary>
        public bool Color
        {
            get { return _color; }
            set { _color = value; RaisePropertyChanged("Color"); }
        }
        private bool _color;

        /// <summary>
        /// The sample grabber has been created
        /// </summary>
        public bool Sample
        {
            get { return _sample; }
            set { _sample = value; RaisePropertyChanged("Sample"); }
        }
        private bool _sample;

        /// <summary>
        /// The video renderer has been created
        /// </summary>
        public bool Render
        {
            get { return _render; }
            set { _render = value; RaisePropertyChanged("Render"); }
        }
        private bool _render;
        #endregion
        #region System Status Properties
        /// <summary>
        /// Is the filter properly installed
        /// </summary>
        public bool FilterStatus
        {
            get { return _filterStatus; }
            set { _filterStatus = value; RaisePropertyChanged("FilterStatus"); }
        }
        private bool _filterStatus;

        /// <summary>
        /// The required YUV codec is installed
        /// </summary>
        public bool Codec
        {
            get { return _codec; }
            set { _codec = value; RaisePropertyChanged("Codec"); }
        }
        private bool _codec;

        /// <summary>
        /// The correct version of directx is installed
        /// </summary>
        public bool DirectX
        {
            get { return _directX; }
            set { _directX = value; RaisePropertyChanged("DirectX"); }
        }
        private bool _directX;

        /// <summary>
        /// The RTSP API is properly installed
        /// </summary>
        public bool RTSPAPI
        {
            get { return _RTSPAPI; }
            set { _RTSPAPI = value; RaisePropertyChanged("RTSPAPI"); }
        }
        private bool _RTSPAPI;

        /// <summary>
        /// Get the Pelco api logging setting
        /// </summary>
        public string FilterLogging
        {
            get { return _filterLogging; }
            set
            {
                _filterLogging = value;
                RaisePropertyChanged("FilterLogging");
            }
        }
        private string _filterLogging;

        /// <summary>
        /// The name and location of the pelco log file
        /// </summary>
        public string LogFile
        {
            get { return _logFile; }
            set
            {
                _logFile = value;
                RaisePropertyChanged("LogFile");
                RaisePropertyChanged("LogOn");
            }
        }
        private string _logFile;
        #endregion
        /// <summary>
        /// The time the graph was started
        /// </summary>
        public string StartTime
        {
            get { return _startTime; }
            set { _startTime = value; RaisePropertyChanged("StartTime"); }
        }
        private string _startTime;

        /// <summary>
        /// The time the first frame was received set by sample grabber
        /// <seealso cref="SampleGrabberCallback"/>
        /// </summary>
        public string FirstFrame
        {
            get { return _firstFrame; }
            set { _firstFrame = value; RaisePropertyChanged("FirstFrame"); }
        }
        private string _firstFrame;

        /// <summary>
        /// The time the first frame was received set by sample grabber
        /// <seealso cref="SampleGrabberCallback"/>
        /// </summary>
        public long BufferSize
        {
            get { return _bufferSize; }
            set { _bufferSize = value; RaisePropertyChanged("BufferSize"); }
        }
        private long _bufferSize;

        /// <summary>
        /// The width of images in the video stream
        /// </summary>
        public int FrameWidth
        {
            get { return _frameWidth; }
            set { _frameWidth = value; RaisePropertyChanged("FrameWidth"); }
        }
        private int _frameWidth;

        /// <summary>
        /// The height of images in the video stream
        /// </summary>
        public int FrameHeight
        {
            get { return _frameHeight; }
            set { _frameHeight = value; RaisePropertyChanged("FrameHeight"); }
        }
        private int _frameHeight;

        /// <summary>
        /// The the rate as which frame grabber is getting called
        /// <seealso cref="SampleGrabberCallback"/>
        /// </summary>
        public double CurrentFrameRate
        {
            get { return Math.Round(_currentFrameRate, 2); }
            set { _currentFrameRate = value; RaisePropertyChanged("CurrentFrameRate"); }
        }
        private double _currentFrameRate;

        /// <summary>
        /// True if we have detected lost video frames in the current stream
        /// </summary>
        public bool LostVideo
        {
            get { return _lostVideo; }
            set
            {
                if (value == _lostVideo) return;
                _lostVideo = value;
                RaisePropertyChanged("LostVideo");
            }
        }
        private bool _lostVideo;

        /// <summary>
        /// The number of lost video frames detected since the last 
        /// 'real' frame
        /// </summary>
        public int LostVideoFrameCount
        {
            get { return _lostVideoFrameCount; }
            set
            {
                if (value == _lostVideoFrameCount) return;
                _lostVideoFrameCount = value;
                RaisePropertyChanged("LostVideoFrameCount");
            }
        }
        private int _lostVideoFrameCount;

        /// <summary>
        /// Our log messages
        /// </summary>
        public ObservableCollection<string> Log { get; set; }

        /// <summary>
        /// An image captured from the video stream
        /// <seealso cref="SampleGrabberCallback"/>
        /// </summary>
        public BitmapSource CapturedImage
        {
            get { return _capturedImage; }
            set
            {
                _capturedImage = value;
                ImageDate = DateTime.Now.ToString("g");
                RaisePropertyChanged("CapturedImage");
            }
        }
        private BitmapSource _capturedImage;

        /// <summary>
        /// The time the image was captured
        /// </summary>
        public string ImageDate
        {
            get { return _imageDate; }
            set { _imageDate = value; RaisePropertyChanged("ImageDate"); }
        }
        private string _imageDate;

        #region default settings
        /// <summary>
        /// The RTSP logging is turned on
        /// </summary>
        public bool LogOn { get { return File.Exists(LogFile); } }

        /// <summary>
        /// Default setting
        /// Use the primary or secondary stream as the default
        /// </summary>
        public int DefaultStream
        {
            get { return _defaultStream; }
            set { _defaultStream = value; RaisePropertyChanged("DefaultStream"); }
        }
        private int _defaultStream;

        /// <summary>
        /// Default setting
        /// The default framerate
        /// </summary>
        public int DefaultFramerate
        {
            get { return _defaultFramerate; }
            set { _defaultFramerate = value; RaisePropertyChanged("DefaultFramerate"); }
        }
        private int _defaultFramerate;

        /// <summary>
        /// Default setting
        /// The maximum supported framerate
        /// </summary>
        public int MaxFramerate
        {
            get { return _maxFramerate; }
            set { _maxFramerate = value; RaisePropertyChanged("MaxFramerate"); }
        }
        private int _maxFramerate;

        /// <summary>
        /// Default setting
        /// The number of seconds to wait before retrying the connection
        /// </summary>
        public int RetryFrameCount
        {
            get { return _retryFrameCount; }
            set { _retryFrameCount = value; RaisePropertyChanged("RetryFrameCount"); }
        }
        private int _retryFrameCount;

        /// <summary>
        /// Default setting
        /// The number of seconds to wait before the system decides it is no long
        /// receiving frames from the camera
        /// </summary>
        public int LostFrameCount
        {
            get { return _lostFrameCount; }
            set { _lostFrameCount = value; RaisePropertyChanged("LostFrameCount"); }
        }
        private int _lostFrameCount;

        /// <summary>
        /// Default setting
        /// The network mask for the network helper
        /// </summary>
        public string NetworkMask
        {
            get { return _networkMask; }
            set { _networkMask = value; RaisePropertyChanged("NetworkMask"); }
        }
        private string _networkMask;

        /// <summary>
        /// Default setting
        /// The seed address of the network 
        /// </summary>
        public string NetworkSeed
        {
            get { return _networkSeed; }
            set { _networkSeed = value; RaisePropertyChanged("NetworkSeed"); }
        }
        private string _networkSeed;
        #endregion

        /// <summary>
        /// List of default settings
        /// </summary>
        public ObservableCollection<DefaultSetting> DefaultSettings { get; set; }

        public bool AutoRestart { get; set; }

        #endregion

        #region GraphStuff
        /// <summary>
        /// Where the magic happens
        /// Build a graph with all the necessary filters, including a sample
        /// grabber to render video from an RTSP Video Source.
        /// </summary>
        /// <param name="pGraph">The actual graph</param>
        /// <param name="srcFile1">The URL of the RTSP stream</param>
        private void BuildGraph(IGraphBuilder pGraph, string srcFile1)
        {
            //reset our properties
            Filter = false;
            Sample = false;
            Render = false;
            Running = false;
            FirstFrame = string.Empty;

            //graph builder
            var pBuilder = (ICaptureGraphBuilder2)new CaptureGraphBuilder2();
            int hr = pBuilder.SetFiltergraph(pGraph);
            CheckHr(hr, "Can't SetFiltergraph");

            //add RTSP Filter
            var pRTSPFilter2 = CreateSourceFilter(pGraph, srcFile1);
            Filter = true;

            //add Colorspace conveter
            //*FYJ var pColorSpaceConverter = CreateColorSpace(pGraph);
            //*FYJ var pColorSpaceConverter2 = CreateColorSpace(pGraph);

            //add SampleGrabber
            var pSampleGrabber = CreateSampleGrabber(pGraph);

            //add Video Renderer
            var pVideoRenderer = CreateVideoRenderer(pGraph);
            
            //connect RTSP Filter and color space converter
            //*FYJ hr = pGraph.ConnectDirect(GetPin(pRTSPFilter2, "Out"), GetPin(pColorSpaceConverter, "Input"), null);
            //*FYJ CheckHr(hr, "Can't connect RTSP Filter and Color space converter");
            //*FYJ Color = true;

            //connect color space converter and sample grabber
            //*FYJ hr = pGraph.ConnectDirect(GetPin(pColorSpaceConverter, "XForm Out"), GetPin(pSampleGrabber, "Input"), null);
            //*FYJ CheckHr(hr, "Can't connect RTSP Filter and Color Space Converter and Sample Grabber");

            //?? Do we really need a second color space converter??
            //*FYJ hr = pGraph.ConnectDirect(GetPin(pSampleGrabber, "Output"), GetPin(pColorSpaceConverter2, "Input"), null);
            //*FYJ CheckHr(hr, "Can't connect RTSP Filter and Color Space Converter and Sample Grabber and Color converter 2");
            //*FYJ Sample = true;

            //add a renderer
            //*FYJ hr = pGraph.ConnectDirect(GetPin(pColorSpaceConverter2, "XForm Out"), GetPin(pVideoRenderer, "VMR Input0"), null);
            //*FYJ CheckHr(hr, "Can't connect RTSP Filter and Color Space Converter and Sample Grabber and Color converter and video render");
            //*FYJ Render = true;

            pBuilder.RenderStream(null, null, pRTSPFilter2, pSampleGrabber, pVideoRenderer);

            _grabber = pSampleGrabber as ISampleGrabber;
            Trace("Graph Complete");
        }

        /// <summary>
        /// Create the end line sample grabber.  Ths is the one that
        /// captures images from the stream
        /// </summary>
        /// <param name="pGraph"></param>
        /// <returns></returns>
        private IBaseFilter CreateSampleGrabber(IGraphBuilder pGraph)
        {
            var clsidSampleGrabber = new Guid("{C1F400A0-3F08-11D3-9F0B-006008039E37}"); //qedit.dll
            var pSampleGrabber3 = (IBaseFilter)Activator.CreateInstance(Type.GetTypeFromCLSID(clsidSampleGrabber));
            int hr = pGraph.AddFilter(pSampleGrabber3, "SampleGrabber");
            CheckHr(hr, "Can't add SampleGrabber to graph");
            var pSampleGrabber3Pmt = new AMMediaType
            {
                majorType = MediaType.Video,
                subType = MediaSubType.RGB32,
                formatType = FormatType.VideoInfo,
                fixedSizeSamples = true,
                formatSize = 88,
                sampleSize = 522240,
                temporalCompression = false
            };
            var pSampleGrabber3Format = new VideoInfoHeader
            {
                SrcRect = new DsRect(),
                TargetRect = new DsRect(),
                BitRate = 94003294,
                AvgTimePerFrame = 333333,
                BmiHeader =
                    new BitmapInfoHeader
                    {
                        Size = 40,
                        Width = 480,
                        Height = 272,
                        Planes = 1,
                        BitCount = 32,
                        ImageSize = 522240
                    }
            };
            pSampleGrabber3Pmt.formatPtr = Marshal.AllocCoTaskMem(Marshal.SizeOf(pSampleGrabber3Format));
            Marshal.StructureToPtr(pSampleGrabber3Format, pSampleGrabber3Pmt.formatPtr, false);
            hr = ((ISampleGrabber)pSampleGrabber3).SetMediaType(pSampleGrabber3Pmt);
            DsUtils.FreeAMMediaType(pSampleGrabber3Pmt);
            CheckHr(hr, "Can't set media type to sample grabber");

            var grabber = pSampleGrabber3 as ISampleGrabber;
            grabber.SetCallback(_capGrabber, 1);
            return pSampleGrabber3;
        }
        /// <summary>
        /// Creates a video render filter to display video in our sample
        /// </summary>
        /// <param name="pGraph"></param>
        /// <returns></returns>
        private IBaseFilter CreateVideoRenderer(IGraphBuilder pGraph)
        {
            var clsidVideoRenderer = new Guid("{B87BEB7B-8D29-423F-AE4D-6582C10175AC}"); //quartz.dll
            var pVideoRenderer2 = (IBaseFilter)Activator.CreateInstance(Type.GetTypeFromCLSID(clsidVideoRenderer));
            int hr = pGraph.AddFilter(pVideoRenderer2, "Video Renderer");
            CheckHr(hr, "Can't add Video Renderer to graph");
            return pVideoRenderer2;
        }
        /// <summary>
        /// The required color space converter
        /// </summary>
        /// <param name="pGraph"></param>
        /// <returns></returns>
        private IBaseFilter CreateColorSpace(IGraphBuilder pGraph)
        {
            var pColorSpaceConverter3 = (IBaseFilter)new Colour();
            int hr = pGraph.AddFilter(pColorSpaceConverter3, "Color Space Converter");
            CheckHr(hr, "Can't add Color Space Converter to graph");
            return pColorSpaceConverter3;
        }
        /// <summary>
        /// Create our RTSP source filter and load the 
        /// RTSP source url
        /// </summary>
        /// <param name="pGraph">The graph the filter will live in</param>
        /// <param name="url">The URL to load into the filter</param>
        /// <returns></returns>
        private IBaseFilter CreateSourceFilter(IGraphBuilder pGraph, string url)
        {
            //var clsidRTSPFilter = new Guid("{B3F5D418-CDB1-441C-9D6D-2063D5538962}"); //RTSPSource.ax
            //var pRTSPFilter2 = (IBaseFilter)Activator.CreateInstance(Type.GetTypeFromCLSID(clsidRTSPFilter));
            //int hr = pGraph.AddFilter(pRTSPFilter2, "RTSP Filter");
            //CheckHr(hr, "Can't add RTSP Filter to graph");
            ////set source filename
            //var pRTSPFilter2Src = pRTSPFilter2 as IFileSourceFilter;
            //if (pRTSPFilter2Src == null)
            //    CheckHr(unchecked((int)0x80004002), "Can't get IFileSourceFilter");
            
            //if (pRTSPFilter2Src != null) 
            //    hr = pRTSPFilter2Src.Load(url, null);
            
            //CheckHr(hr, "Can't load file");
            //Filter = true;
            IBaseFilter pSourceFilter2;

            int hr = pGraph.AddSourceFilter(url, "SourceFilter", out pSourceFilter2);
            CheckHr(hr, "Can't add source Filter to graph for url="+url);
            return pSourceFilter2;
        }

        /// <summary>
        /// Helper function to get a pin from a filter
        /// </summary>
        /// <param name="filter"></param>
        /// <param name="pinname"></param>
        /// <returns></returns>
        private IPin GetPin(IBaseFilter filter, string pinname)
        {
            IEnumPins epins;
            int hr = filter.EnumPins(out epins);
            CheckHr(hr, "Can't enumerate pins");
            IntPtr fetched = Marshal.AllocCoTaskMem(4);
            var pins = new IPin[1];
            while (epins.Next(1, pins, fetched) == 0)
            {
                PinInfo pinfo;
                pins[0].QueryPinInfo(out pinfo);
                bool found = (pinfo.name == pinname);
                DsUtils.FreePinInfo(pinfo);
                if (found)
                    return pins[0];
            }
            CheckHr(-1, "Pin not found");
            return null;
        }

        /// <summary>
        /// helper function to deal with hresults
        /// </summary>
        /// <returns></returns>
        private string DecodeHrResult(uint hr)
        {
            string ret;
            switch (hr)
            {
                case 0x8004020D:
                    ret = "Video Buffer does not match image size";
                    break;
                case 0x8004022F:
                    ret = "Invalid RTSPsource url";
                    break;
                default:
                    ret = string.Empty;
                    break;
            }
            return ret;
        }

        /// <summary>
        /// Helper function for HResults
        /// </summary>
        /// <param name="hr"></param>
        /// <param name="msg"></param>
        private void CheckHr(int hr, string msg)
        {
            if (hr >= 0) return;
            Trace("ERROR:  {0} (HR=0x{1:x})", msg, hr);
            Trace(DecodeHrResult((uint)hr));
            Console.WriteLine(msg);
            Fault = true;
            DsError.ThrowExceptionForHR(hr);
        }
        #endregion

        #region System Status
        /// <summary>
        /// Check all the system status 
        /// </summary>
        private void CheckSystemStatus()
        {
            CheckFilter();
            CheckDirectX();
            GetDefaultSettings();
        }

        /// <summary>
        /// Check the windows system directory to make sure direct x is installed
        /// </summary>
        private void CheckDirectX()
        {
            try
            {
                var count=Directory.GetFiles(@"C:\Windows\System32\", "D3DX*.DLL");
                if(count.Length>0)
                    Trace("Found DX version: "+count[0]);
                DirectX = count.Length > 0;
            }
            catch (Exception ex)
            {
                ExceptionHelper.HandleException(ex);
                Trace("ERROR:  Getting DirectX information.");
            }
        }

        /// <summary>
        /// Check the registry 
        /// </summary>
        private void CheckFilter()
        {
            try
            {
                using (var key = Registry.ClassesRoot.OpenSubKey(@"CLSID\{B3F5D418-CDB1-441C-9D6D-2063D5538962}\InprocServer32"))
                {
                    if (key != null)
                    {
                        Trace("Found filter at {0}",key.GetValue("").ToString());
                        FilterStatus = File.Exists(key.GetValue("").ToString());
                    }

                }
            }
            catch (Exception ex)
            {
                ExceptionHelper.HandleException(ex);
                Trace("ERROR:  Getting Filter information.");
            }
        }

        /// <summary>
        /// Get default filter settings from the registry
        /// </summary>
        private void GetDefaultSettings()
        {
            try
            {
                using (var key = Registry.CurrentUser.OpenSubKey("Software\\Microsoft\\MediaPlayer\\Player\\Schemes\\RTSPSource"))
                {
                    if (key != null)
                    {
                        //DefaultStream = Convert.ToInt16(GetValue(key, "DefaultStream", 2));
                        //System.Windows.Application.Current.Dispatcher.BeginInvoke(new MethodInvoker(() =>
                        //AddSetting("DefaultStream", DefaultStream.ToString(), "If the stream is not specified in the query string then use this value as the stream number.")));

                        DefaultFramerate = Convert.ToInt16(GetValue(key, "DefaultFramerate", 12));
                        System.Windows.Application.Current.Dispatcher.BeginInvoke(new MethodInvoker(() =>
                        AddSetting("DefaultFramerate", DefaultFramerate.ToString(CultureInfo.InvariantCulture), "If no Framerate is specified in the query string then use this value as the framerate.")));

                        MaxFramerate = Convert.ToInt16(GetValue(key, "MaxFramerate", 30));
                        System.Windows.Application.Current.Dispatcher.BeginInvoke(new MethodInvoker(() =>
                        AddSetting("MaxFramerate", MaxFramerate.ToString(CultureInfo.InvariantCulture), "The upper limit of supported framerates per the query string.")));

                        RetryFrameCount = Convert.ToInt16(GetValue(key, "RetryFrameCount", 120));
                        System.Windows.Application.Current.Dispatcher.BeginInvoke(new MethodInvoker(() =>
                        AddSetting("RetryFrameCount", RetryFrameCount.ToString(CultureInfo.InvariantCulture), "The number seconds that can be empty before the system attempts to reload the video pipeline.  ")));

                        LostFrameCount = Convert.ToInt16(GetValue(key, "LostFrameCount", 10));
                        System.Windows.Application.Current.Dispatcher.BeginInvoke(new MethodInvoker(() =>
                        AddSetting("LostFrameCount", LostFrameCount.ToString(CultureInfo.InvariantCulture), "The number seconds to wait before we take action for a lost frame problem. Often when video is lost the same frame will be sent over  and over again.  This value represents how many times we get the same frame before we fall into the lost video buffering mode. A value to low could cause performance problems if the video can self recover a value to high will cause lengthy delays.")));
                    }

                    RaisePropertyChanged("DefaultSettings");
                }
            }
            catch (Exception)
            {
                Trace("ERROR:  Getting Defaultsettings information.");
            }
        }
        /// <summary>
        /// helper function to add a setting to our list
        /// </summary>
        /// <param name="name"></param>
        /// <param name="value"></param>
        /// <param name="tooltip"></param>
        private void AddSetting(string name, string value, string tooltip)
        {
            DefaultSettings.Add(new DefaultSetting(name, value, tooltip));
        }
        /// <summary>
        /// Helper function to get a setting from the registry
        /// </summary>
        /// <param name="key"></param>
        /// <param name="setting"></param>
        /// <param name="defaultValue"></param>
        /// <returns></returns>
        private int GetValue(RegistryKey key, string setting, int defaultValue)
        {
            if (key.GetValue(setting) == null)
                return defaultValue;
            return Convert.ToInt16(key.GetValue(setting));
        }

        private void UpdateCommands()
        {
            PlayCommand.OnCanExecuteChanged();
            PauseCommand.OnCanExecuteChanged();
            StopCommand.OnCanExecuteChanged();
            RestartCommand.OnCanExecuteChanged();
            CaptureImageCommand.OnCanExecuteChanged();
        }
        #endregion

        #region INotifyPropertyChange
        private void RaisePropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;

            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(propertyName));
            }
        }
        public event PropertyChangedEventHandler PropertyChanged;
        #endregion

        /// <summary>
        /// Clean up stuff
        /// </summary>
        public void Dispose()
        {
            if (MediaControl != null)
                MediaControl.Stop();
            //debug
            if (rot != null)
            {
                rot.Dispose();
                rot = null;
            }
        }
    }
}
