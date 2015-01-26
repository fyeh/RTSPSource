RTSPSource
==========

Description
===========

	A DirectShow Source Filter that uses RTSP to get video streams
 
References & Attribution
========================

	This project contains snapshots of several other open source libraries or projects 
	that are included to ensure that the correct versions of these external libraries
	are used:
	
	LiveProxy - https://github.com/dhorth/LiveProxy
  
    ffMpeg - http://ffmpeg.zeranoe.com/builds/
      Codecs used by the Filter
      
    live555 - http://www.live555.com/liveMedia/
      Support for RTSP
  
    Directshowlib - http://directshownet.sourceforge.net/
        .NET access to DirectShow Used by Samples
		
    Log4net - http://logging.apache.org/log4net/
        Logging libraries Used by Samples
		
	This project is based on the vsm7dsf project at https://github.com/fyeh/vsm7dsf but has been modified to work with Generic RTSP sources

Microsoft pre-req's
===================

  Microsoft Visual C++ 2008 Service Pack 1 Redistributable Package MFC Security Update
  
  Microsoft Visual C++ 2010 Service Pack 1 Redistributable Package MFC Security Update
  
  Visual C++ Redistributable for Visual Studio 2012 Update 4
  
  .NET 4

  
Project Contents
================

  PushSource - The Actual DirectShow Filter.

  HelperLib - C# managed library.  Used to integrate the managed Cisco VSM SDK with the unmanaged direct show filter.
  
  SampleGrabber - C# Application useful for testing the DSF.

Building
========

  Build LiveProxy first then build RTSPSource. Do this for whichever configuration (Debug or Release) you are building for. 
  IE build Liveproxy for Debug then RTSPSource  for Debug. When you are ready to release, build LiveProxy for Release then RTSPSource for Release.
  
Using
=====

  After a successful build, the RTSPSource.ax filter will be registered on the system. Any program that uses DirectShow
  to render a URL in the specified format will load the filter.
  
  URL should be in the format:
  
  rtspsource://<path and parameters>&Width=<Width of Stream>&Height=<Height of Stream>
  
  Note that Width and Height are required and must match the width and height of the stream specified.
  
  EG: is you would normally used an RTSP URL of "rtsp://192.168.1.1/stream1" to play an RTSP stream from the video source, 
  the URL to invoke this source filter would be "rtspsource://192.168.1.1/stream1&width=704&height=480".
  
