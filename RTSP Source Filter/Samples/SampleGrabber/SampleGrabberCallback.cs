
using System;
using System.Runtime.InteropServices;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using DirectShowLib;

namespace SampleGrabber
{
    /// <summary>
    /// This is the callback for the sample grabber filter
    /// that is installed at the end of the graph, after the
    /// color conversion.  Frames delivered to this sample
    /// grabber are already in an RGB format
    /// </summary>
    public class SampleGrabberCallback : ISampleGrabberCB
    {
        private IntPtr _map = IntPtr.Zero;
        private IntPtr _section = IntPtr.Zero;
        private int _width;
        private int _height;
        private bool firstFrame;
        private DateTime _startTime;
        private int _frameCount = 0;

        #region pInvoke Stuff
        /// <summary>
        /// The CreateFileMapping function creates or opens a named or unnamed file mapping object for the specified file
        /// </summary>
        /// <param name="hFile"></param>
        /// <param name="lpFileMappingAttributes"></param>
        /// <param name="flProtect"></param>
        /// <param name="dwMaximumSizeHigh"></param>
        /// <param name="dwMaximumSizeLow"></param>
        /// <param name="lpName"></param>
        /// <returns></returns>
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr CreateFileMapping(IntPtr hFile, IntPtr lpFileMappingAttributes, uint flProtect,
                                                       uint dwMaximumSizeHigh, uint dwMaximumSizeLow, string lpName);

        /// <summary>
        /// Maps a view of a file mapping into the address space of a calling process.
        /// </summary>
        /// <param name="hFileMappingObject"></param>
        /// <param name="dwDesiredAccess"></param>
        /// <param name="dwFileOffsetHigh"></param>
        /// <param name="dwFileOffsetLow"></param>
        /// <param name="dwNumberOfBytesToMap"></param>
        /// <returns></returns>
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr MapViewOfFile(IntPtr hFileMappingObject, uint dwDesiredAccess,
                                                   uint dwFileOffsetHigh, uint dwFileOffsetLow,
                                                   uint dwNumberOfBytesToMap);


        /// <summary>
        /// Copies a block of memory from one location to another.
        /// </summary>
        /// <param name="destination"></param>
        /// <param name="source"></param>
        /// <param name="length"></param>
        [DllImport("Kernel32.dll", EntryPoint = "RtlMoveMemory")]
        private static extern void CopyMemory(IntPtr destination, IntPtr source, int length);
        #endregion

        /// <summary>
        /// Bitmap of the current frame, continually updated
        /// </summary>
        public BitmapSource Bitmap { get; set; }

        /// <summary>
        /// Create our local frame buffer, this is the
        /// memory we use when we copy the incoming frame
        /// </summary>
        /// <param name="pcount"></param>
        /// <param name="width"></param>
        /// <param name="height"></param>
        public void Initialize(uint pcount, int width, int height)
        {
            try
            {
                _width = width;
                _height = height;

                // Create a file mapping
                _section = CreateFileMapping(new IntPtr(-1), IntPtr.Zero, 0x04, 0, pcount, null);
                _map = MapViewOfFile(_section, 0xF001F, 0, 0, pcount);

                // Get the bitmap
                System.Windows.Application.Current.Dispatcher.Invoke(new Action(AllocateBitmap));
            }
            catch (Exception ex)
            {
            }
        }

        #region ISampleGrabberCB
        /// <summary>
        /// receives a pointer to the media sample.
        /// </summary>
        /// <param name="sampleTime">Starting time of the sample, in seconds.</param>
        /// <param name="pSample">Pointer to the IMediaSample interface of the sample.</param>
        /// <returns></returns>
        public int SampleCB(double sampleTime, IMediaSample pSample)
        {
            return 0;
        }

        /// <summary>
        /// Called for each frame
        /// </summary>
        /// <param name="sampleTime">Time of sample</param>
        /// <param name="pBuffer">Buffer containing image</param>
        /// <param name="bufferLen">size of buffer</param>
        /// <returns></returns>
        public int BufferCB(double sampleTime, IntPtr pBuffer, int bufferLen)
        {
            _frameCount++;

            //stuff todo when we start up
            if (!firstFrame)
            {
                firstFrame = true;
                var vm = ViewModel.Get();
                _startTime = DateTime.Now;
                vm.FirstFrame = DateTime.Now.ToString("g");
                vm.BufferSize = bufferLen;
            }

            //copy the frame to our local memory
            if (_map != IntPtr.Zero)
                CopyMemory(_map, pBuffer, bufferLen);

            // Calculate framerate
            if ((DateTime.Now - _startTime).TotalMilliseconds >= 2000)
            {
                ViewModel.Get().CurrentFrameRate = _frameCount/((DateTime.Now - _startTime).TotalMilliseconds/1000);
                _frameCount = 0;
                _startTime = DateTime.Now;
            }
            return 0;
        }
        #endregion

        /// <summary>
        /// Create a bitmap
        /// </summary>
        private void AllocateBitmap()
        {
            try
            {
                Bitmap = Imaging.CreateBitmapSourceFromMemorySection
                             (_section, _width, _height,
                              PixelFormats.Bgr32, _width * PixelFormats.Bgr32.BitsPerPixel / 8, 0)
                         as InteropBitmap;
                ViewModel.Get().CaptureImageCommand.OnCanExecuteChanged();
            }
            catch (Exception ex)
            {
            }
        }

    }
}
