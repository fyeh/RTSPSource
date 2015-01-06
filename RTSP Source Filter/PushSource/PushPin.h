
 #pragma once
#include "stdafx.h"
#include "PushGuids.h"
#include "TransportUrl.h"
#include "live.h"

// Filter name strings
#define g_wszPushRTSP L"RTSP Filter"


enum VideoState {
	  NoVideo,
	  Lost,
	  Reloading,
	  Started,
	  Playing,
   };
/**
 * This is the output pin, essentially the base class does all the
 * work we just handle the FillBuffer request by passing back to the 
 * mediabridge
 */
class CPushPinRTSP : public CSourceStream
{
	friend class CPushSourceRTSP;

private:
	CstreamMedia*				m_streamMedia;
	int							m_videoWidth, m_videoHeight;
	DWORD						m_fourCC;
	GUID						m_mediaSubType;
	int							m_bitCount;
	REFERENCE_TIME				m_framerate;
	REFERENCE_TIME				m_rtStart;
	std::string					m_strSessionId; //session id for video
	
	//More interesting variables
	/**
	use the low bandwidth stream
	*/
	bool m_useStream2;
	/**
	flag to tells us is we have at least gotten one frame in the new session
	*/
	bool						m_firstFrame;  

	/**
	The DS ref time of the last frame (no relation to frame time)
	*/
	REFERENCE_TIME				m_lastFrame;

	/**
	counter used to track the number of frame request that could not
	be satisified, when this value get high enough we do a reload attempt
	*/
	long						m_bufferCount; 
	
	/**
	The number of seconds to wait before we attempt to reload video
	The seconds are converted to frames and checked when we either
	have not recieved a frame or keep getting the same frame over and over
	Registry Setting: ReloadLostVideoTime
	\sa Settings::ReloadLostVideoTime()
	*/
	long							m_reloadLostVideoTime;
	
	/**
	The number seconds that the filter will wait before attempting
	to reload the video pipeline if no video is found.  This value 
	is used when we detect the camera is online but we are not recieving
	video, typically this means switching from multicast to unicast
	\sa Settings::RetryConnectionTime()
	*/
	long							m_retryConnectionTime;


	/**
	After a reload we can see a burst of frames come through.  the retryFrameBufferTime is used 
	to damper that load.  We wait the number of seconds in specified before we start checking for
	a reload.
	Registry Setting: ReloadFrameBufferSeconds
	\sa Settings::ReloadFrameBufferSeconds()
	*/
	long						m_reloadFrameBufferTime;

	/**
	This counter is used to track the number of frames received from the pipeline 
	that have the same timestamp, when the LostFrameCount is exceeded we fall into
	the reload loop
	*/
	long						m_lostFrameBufferCount; 

	/**
	To determine if the pipeline is just resending us the same frame
	over and over again, we need to check the frame time of the last
	frame against the current frame time.  The m_lostframecount and 
	m_lostframebuffercount are used to damper false positives
	*/
	std::string					m_lastFrameTime;

	/**
	enum of the current video state
	*/
	VideoState					m_currentVideoState;

protected:
	HRESULT OnThreadCreate(void);
	HRESULT OnThreadDestroy(void);
	HRESULT OnThreadStartPlay(void);

	CCritSec m_cSharedState;

public:
	DECLARE_IUNKNOWN;
	virtual HRESULT Stop(void);
	CPushPinRTSP(HRESULT *phr, CSource *pFilter);
	~CPushPinRTSP();

	//HRESULT CheckMediaType(const CMediaType *pMediaType);
	HRESULT GetMediaType(CMediaType *pMediaType);
	HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest);
	HRESULT FillBuffer(IMediaSample *pSample);
	LPVOID get_Filter(){return (LPVOID)(this->m_pFilter);}

private:
	bool ReloadVideo(TransportUrl * url);
	bool ProcessVideo(IMediaSample *pSample);
	int QueryVideo(TransportUrl * url);
};
