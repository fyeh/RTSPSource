#pragma once
#include <MediaSink.hh>
#include <H264VideoRTPSink.hh>
#include <JPEGVideoRTPSink.hh>
#include <liveMedia.hh>
#include <iostream>
#include <deque>
#include "live.h"
#include "MediaQueue.h"
#include "VideoDecoder.h"

#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 100000


/**
Our implementation of a MediaSink.
The media sink recevices the video frames from the rtsp stream
then passes them to our /sa CVideoDecoder for processing.  This class
is also the keeper of our decoded frame queue /sa CMediaQueue
*/
class MyVideoSink : public MediaSink {

// static methods
public:
	static MyVideoSink * createNew(UsageEnvironment& env);

protected:
    //static void afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes, timeval presentationTime, unsigned durationInMicroseconds);

public:
	//StreamTrack* get_StreamTrack(){return m_tk;}
	//void set_StreamTrack(StreamTrack* tk){m_tk=tk;}
	CMediaQueue* get_FrameQueue(){return m_ready==1? m_frameQueue:NULL;}
	virtual ~MyVideoSink();


protected:
	MyVideoSink(UsageEnvironment& env);
	//void afterGettingFrame1(unsigned frameSize, struct timeval presentationTime);
	void AddData(uint8_t* aData, int aSize);
    //Boolean	continuePlaying();

       
//Members
protected:
	//CVideoDecoder *		m_decoder;
	unsigned char *		m_buffer;
	unsigned int		m_bufferSize;
	//MediaSubsession&	m_fSubsession;
	CRITICAL_SECTION	m_criticalSection;
	//char *				m_fStreamId;
	//char *				m_format;
	CMediaQueue*		m_frameQueue;
	int					m_fPos;
	int					m_ready;
};

