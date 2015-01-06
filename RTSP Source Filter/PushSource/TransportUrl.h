
#pragma once
#include "stdafx.h"

   enum  DatePart{Year=0,Month=4,Day=6,Seperator=8,Hour=9,Minute=11,Second=13};


/**
Parse the RTSPsource URL into our required parameters.

The first part of the URL is the address of the RTSP server. 
The port is optional, if specified the filter will use the port as part of the URL used to communicate with the RTSP server. If it is not specified the standard http ports will be used (80 & 443) depending on the ‘Secure’ 
The query string contains data that will be used to locate the desired video stream. It may contain name/value pairs separated by ampersands, for example ?camera=1.  The name identifier of property is case insensitive; Camera and camera are both valid.

\cond Examples
The following are examples of acceptable URI’s:
Typical URL
RTSPsource://192.168.1.1/stream1?Width=704&Height=480

Full URL

\endcond
*/
   class TransportUrl 
	{

	public:
        /**
        * Constructor which takes a url string and breaks it into
        * its properties for easy access in the code
        **/
        TransportUrl(LPCTSTR url);
		~TransportUrl(){}
        
        /**
        * The complete URL
        **/
		const char* get_Url(){return m_url.c_str();}

		/**
		the converted RTSP url
		*/
		const char* get_RtspUrl(){return m_rtspUrl.c_str();}

		/**
		we have loaded a valid url
		*/
		bool hasUrl(){return m_rtspUrl.length()>0;}

        /**
        * The scheme name is always rtspsource, this is not case sensitive.  The scheme name is
        *  available once the DirectShow filter is installed and registered.  This name is recorded
        *  in the windows registry at HKEY_CURRENT_USER-Software-Microsoft-MediaPlayer-Player-Schemes.
        **/
		const char* get_Scheme(){return m_scheme.c_str();}
        
        /**
        *  Use the Framerate parameter in the query string to specify
        *  the speed at which frames will be retrieved from the live
        *  video stream.  The framerate setting is the number of frame
        *  per second (ie 30 would be 30 frame per second or 1 frame 
        *  every 1/33333 seconds, and 2 would be one frame every ½ second).
        *  The Framerate setting here does not have to match the actually
        *  frame rate of the video stream, if for instance you specify 5
        *  for five frames a second but the stream is set for 10 FPS then
        *  you would skip every other frame.  The reverse is true if you
        *  set this setting to 10 and the video stream is configured for
        *  5FPS then you would get the same frame twice before getting a new frame.
        **/
		  int get_Framerate(){return m_framerate;}

        /**
		  * The number of consecutive time we fail to get a frame from the
		  * queue before we restart the video stream
        **/
		  int get_LostFrameCount(){return m_lostFrameCount;}

		/**
		  the max number of frames our queue can hold
		*/
		 int get_FrameQueueSize(){return m_frameQueueSize;}
		  
		 /**
		  width of a single video frame
		  */
		  int get_Width(){return m_width;}

		  /**
		  height of a single video frame
		  */
		  int get_Height(){return m_height;}
	private:
		void getValue(std::string query, LPCSTR field, std::string& returnValue, bool ignoreCase=true);
		int getInt(std::string query, LPCSTR field, bool ignoreCase=true);
		bool IsInt(std::string value);

	private:
		std::string m_url;
		std::string m_scheme;
		std::string m_rtspUrl;
		std::string m_path;

		int m_framerate;
		int m_frameQueueSize;
		int m_lostFrameCount;
		int m_width;
		int m_height;

	};
