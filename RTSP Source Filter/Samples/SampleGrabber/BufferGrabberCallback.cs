

using System;
using System.Runtime.InteropServices;
using DirectShowLib;

namespace SampleGrabber
{
    /// <summary>
    /// The callback class interface for the sample grabber
    /// The buffer grabber is inserted right after the RTSPsource
    /// filter and gets the raw YUV buffer. 
    /// The purpose of the buffer grabber is to check the frame
    /// buffer for the lost video tag.
    /// </summary>
    public class BufferGrabberCallback : ISampleGrabberCB
    {
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
        /// Called for each frame. 
        /// Check the buffer for the presence of the lost
        /// video tag, if found set the view model flags and count
        /// </summary>
        /// <param name="sampleTime">Time of sample</param>
        /// <param name="pBuffer">Buffer containing image</param>
        /// <param name="bufferLen">size of buffer</param>
        /// <returns></returns>
        public int BufferCB(double sampleTime, IntPtr pBuffer, int bufferLen)
        {
            if (Marshal.ReadByte(pBuffer, 0) == 0x21
                && Marshal.ReadByte(pBuffer, 1) == 0x44
                && Marshal.ReadByte(pBuffer, 2) == 0x4d
                && Marshal.ReadByte(pBuffer, 3) == 0x48
                && Marshal.ReadByte(pBuffer, 4) == 0x21)
            {
                ViewModel.Get().LostVideoFrameCount++;
                ViewModel.Get().LostVideo = true;
            }
            else
            {
                ViewModel.Get().LostVideo = false;
                ViewModel.Get().LostVideoFrameCount = 0;
            }
            return 0;
        }
    }
}
