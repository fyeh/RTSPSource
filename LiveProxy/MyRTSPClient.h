#pragma once
#include "stdafx.h"
#include "StreamClientState.h"
#include "MediaSink.hh"
#include "live.h"
#include "MediaQueue.h"
#include "H264VideoSink.h";
#include "MP4VVideoSink.h";


/**
Our implementation of an RTSPClient

This class is used to manage the filter video sink /sa MyVideoSink
and implement the rtsp interface to receive video frames see both
/sa CStreammedia and /sa CPushPinCisco
*/
class MyRTSPClient: public RTSPClient {
public:
	static MyRTSPClient* createNew(UsageEnvironment& env, char const* rtspURL, char const* format);
	virtual ~MyRTSPClient();

protected:
	MyRTSPClient(UsageEnvironment& env, char const* rtspURL, char const* format, int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);

public:
	StreamTrack* get_StreamTrack(){return tk;}
	void set_StreamTrack(StreamTrack* tk){this->tk=tk;}

	MyVideoSink * get_sink(){return m_sink;}
	
public:
	StreamClientState scs;
	MyVideoSink * m_sink;
	StreamTrack * tk;
};