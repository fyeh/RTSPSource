
#include "stdafx.h"
#include "PushSource.h"
#include "PushPin.h"
#include "PushGuids.h"
#include "TransportUrl.h"
#include "Settings.h"
#include "live.h"
#include "trace.h"
#include <Dvdmedia.h>

#pragma managed
/**
 * This is the output pin construction for our source filter
 */
CPushPinRTSP::CPushPinRTSP(HRESULT *phr, CSource *pFilter)
        : CSourceStream(NAME("Push Source RTSP"), phr, pFilter, L"Out")
{
	TRACE_INFO(  "Pin constructor");
	//create and instance of the view object
	m_rtStart=0;
	m_bufferCount=0;
	m_lostFrameBufferCount=0;
	
	m_retryConnectionTime=0;
	m_reloadLostVideoTime=0;
	m_videoWidth=m_videoHeight=m_bitCount=0;
	m_reloadFrameBufferTime=0;
	m_currentVideoState=NoVideo;
	m_framerate=12;
	m_streamMedia=NULL;
}

/**
* Destructor, 
*/
CPushPinRTSP::~CPushPinRTSP()
{   
	//stop the pipe/viewer first
	Stop();

	TRACE_INFO( "Shutting down stream");
	if(m_streamMedia!=NULL)
	{
		m_streamMedia->rtspClientCloseStream();
		delete m_streamMedia;
	}
	m_streamMedia=NULL;
	TRACE_DEBUG(  "Pin destructor");
}

/**
* Returns the media type that we will be rendering 
* back to the base class.  This is where we actually
* load the video pipeline, we do it up front so that
* we have the correct image size for the buffer
*/
HRESULT CPushPinRTSP::GetMediaType(CMediaType *pMediaType)
{
	TRACE_DEBUG(  "GetMediaType");
	TransportUrl *url=((CPushSourceRTSP*)this->m_pFilter)->m_transportUrl;
	QueryVideo(url);

	CAutoLock cAutoLockShared(&m_cSharedState);
	VIDEOINFO *pvi = (VIDEOINFO *)pMediaType->AllocFormatBuffer(sizeof(VIDEOINFO));

	/* Would this ever really happen? */
    if (NULL == pvi) return S_OK;

	/* Zero out the memory */
	ZeroMemory(pvi, sizeof(VIDEOINFO));

	/* Plugin in our FOURCC */
	TRACE_INFO( "  biCompression = BI_RGB");
	pvi->bmiHeader.biCompression = BI_RGB;

	/* 24 bits per pixel */
    pvi->bmiHeader.biBitCount = 24;

	/* Header size */
    pvi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

	/* Video width */
    pvi->bmiHeader.biWidth =  m_videoWidth;

	/* Video height */
    pvi->bmiHeader.biHeight =  -m_videoHeight;

	/* Video Planes */
    pvi->bmiHeader.biPlanes = 1;

	/* The max size a video frame will be */
    pvi->bmiHeader.biSizeImage = GetBitmapSize(&pvi->bmiHeader);
	
	/* ?? */
    pvi->bmiHeader.biClrImportant = 0;

	/* This stuff is pretty much ignored in this situation */
	pvi->AvgTimePerFrame = UNITS/30;
	
	/* Set the Major-type of the media for DShow */
    pMediaType->SetType(&MEDIATYPE_Video);

	/* Set the structure type so DShow knows 
	 * how to read the structure */
    pMediaType->SetFormatType(&FORMAT_VideoInfo);

	/* If our codec uses temporal compression */
    pMediaType->SetTemporalCompression(FALSE);
		
	GUID subTypeGUID;
	subTypeGUID = MEDIASUBTYPE_RGB24;
	pMediaType->SetSubtype(&subTypeGUID);

	/* Set the max sample size */
    TRACE_DEBUG(  " biSizeImage=%d", pvi->bmiHeader.biSizeImage);
	 pMediaType->SetSampleSize(pvi->bmiHeader.biSizeImage);
	
	/* Get our video header to configure */
	VIDEOINFOHEADER *vihOut = (VIDEOINFOHEADER *)pMediaType->Format();
	
	/* 30 Fps - Not important */
	vihOut->AvgTimePerFrame = UNITS / 30;
	return S_OK;
}

/**
Setup our video envirnoment.
Using the transport URL setup the environment
this most likely means connecting to a VSM and getting
a security token for the camera.  Note this can take some time
to complete, we made to revist.  It could be possible to get
the token in a background thread as long as we have the width
and height in the url
*/
int CPushPinRTSP::QueryVideo(TransportUrl * url)
{
	int ret=-1;

	TRACE_INFO("Query video");
	if(!(m_currentVideoState==VideoState::NoVideo||m_currentVideoState==VideoState::Lost)) 
	{
		TRACE_INFO("Video already configured, exiting");
		return 0;
	}

	if(!url->hasUrl())
	{
		TRACE_INFO("Missing the URL");
		return ret;
	}

	try
	{
		TRACE_INFO("Try to open the RTSP video stream");
		if(m_streamMedia==NULL)
			m_streamMedia=new CstreamMedia();

		ret =  m_streamMedia->rtspClientOpenStream((const char *)url->get_RtspUrl());
		if (ret < 0)
		{
			TRACE_ERROR( "Unable to open rtsp video stream ret=%d", ret);
			return E_FAIL;	
		}
	}
	catch(...)
	{
		TRACE_CRITICAL( "QueryVideo Failed");
		m_currentVideoState=VideoState::NoVideo;
		throw false;
	}

	//attempt to get the media info from the stream
	//we know that in 7.2 this does not work, but we are
	//hoping that 7.5 will enable width and height
	MediaInfo videoMediaInfo;
	try{
		TRACE_INFO("Get Media Info");
		ret= m_streamMedia->rtspClinetGetMediaInfo(CODEC_TYPE_VIDEO, videoMediaInfo);
		if(ret < 0)
		{	
			TRACE_CRITICAL( "Unable to get media info from RTSP stream.  ret=%d (url=%s)", ret,url->get_Url());
			return VFW_S_NO_MORE_ITEMS;
		}
	}
	catch(...)
	{
		TRACE_CRITICAL( "QueryVideo Failed from ");
		m_currentVideoState=VideoState::NoVideo;
		throw false;
	}

	TRACE_INFO( "Format: %d",videoMediaInfo.i_format);
	TRACE_INFO( "Codec: %s",videoMediaInfo.codecName);
	if(videoMediaInfo.video.width>0)
	{
		TRACE_INFO( "Using video information directly from the stream");
		m_videoWidth = videoMediaInfo.video.width;
		m_videoHeight = videoMediaInfo.video.height;
		m_bitCount = videoMediaInfo.video.bitrate;
		if(videoMediaInfo.video.fps>0)
			m_framerate=(REFERENCE_TIME)(10000000/videoMediaInfo.video.fps);
	}else{
		TRACE_WARN( "No video info in stream, using defaults from url");
		m_videoWidth = url->get_Width();
		m_videoHeight = url->get_Height();
		//m_videoWidth = 352;
		//m_videoHeight = 240;
		m_bitCount = 1;
		if(url->get_Framerate()>0)
			m_framerate=(REFERENCE_TIME)(10000000/url->get_Framerate());
	}

	TRACE_INFO( "Width: %d",m_videoWidth);
	TRACE_INFO( "Height: %d",m_videoHeight);
	TRACE_INFO( "FPS: %d",10000000/m_framerate);
	TRACE_INFO( "Bitrate: %d",m_bitCount);
	m_currentVideoState=VideoState::Reloading;
		
	return ret;
}


/**
* DecideBufferSize
*
* This will always be called after the format has been sucessfully
* negotiated. So we have a look at m_mt to see what size image we agreed.
* Then we can ask for buffers of the correct size to contain them.
*/
HRESULT CPushPinRTSP::DecideBufferSize(IMemAllocator* pMemAlloc, ALLOCATOR_PROPERTIES* pProperties)
{
	/* Thread-saftey */
    TRACE_DEBUG(  "DecideBufferSize");
	CAutoLock cAutoLockShared(&m_cSharedState);

	HRESULT hr = S_OK;
	VIDEOINFO *pvi = (VIDEOINFO *)m_mt.Format();
	pProperties->cBuffers = 1;
	pProperties->cbBuffer =2* pvi->bmiHeader.biSizeImage;		
	
//	pProperties->cBuffers = 20;
//	pProperties->cbBuffer = 65535;
	
	/* Ask the allocator to reserve us some sample memory, NOTE the function
	 * can succeed (that is return NOERROR) but still not have allocated the
	 * memory that we requested, so we must check we got whatever we wanted */
	ALLOCATOR_PROPERTIES Actual;

	hr = pMemAlloc->SetProperties(pProperties, &Actual);
	if (FAILED(hr)) 
	{
		TRACE_ERROR( "DecideBufferSize pMemAlloc->SetProperties.  HRESULT = %#x, SIZE=%d\n", hr, pProperties->cbBuffer);
		return hr;
	}

	/* Is this allocator unsuitable? */ 
	if (Actual.cbBuffer < pProperties->cbBuffer) 
	{
		TRACE_ERROR( "DecideBufferSize to small %d < %d\n",Actual.cbBuffer, pProperties->cbBuffer);
		return E_FAIL;
	}

	TRACE_INFO( "DecideBufferSize SIZE=%d\n",  pProperties->cbBuffer);

	TransportUrl *url=((CPushSourceRTSP*)this->m_pFilter)->m_transportUrl;
	if(m_currentVideoState!=Playing)
		m_streamMedia->rtspClientPlayStream(url->get_RtspUrl());
	m_currentVideoState=Playing;
	return S_OK;
}



/**
* This is where we insert the DIB bits into the video stream.
* FillBuffer is called once for every sample in the stream.
* We then pass the buffer to the ProcessVideo to do the actual
* byte copy
*
* If the source checked out ok but fill buffer never gets 
* called then the problem is probably a bad media type.
* FillBuffer is called by the directshow thread.
*/
HRESULT CPushPinRTSP::FillBuffer(IMediaSample *pSample)
{
	bool syncPoint;
	HRESULT hr = S_OK;
    REFERENCE_TIME rtStop = 0, rtDuration = 0;
	bool rc=true;

	CheckPointer(pSample, E_POINTER);
	//CAutoLock cAutoLockShared(&CPushPinRTSP);

	try
	{
		// Copy the DIB bits over into our filter's output buffer.
		//This is where the magic happens, call the pipeline to fill our buffer
		rc = ProcessVideo(pSample);

		if (rc)
		{
			// Set the timestamps that will govern playback frame rate.
			REFERENCE_TIME rtStop  = m_rtStart + m_framerate;
			pSample->SetTime(&m_rtStart, &rtStop);
			pSample->SetDiscontinuity(FALSE);
			m_rtStart=rtStop;
			pSample->SetSyncPoint(TRUE);
		}else{
			hr=E_FAIL;
		}
	}
	catch(...)
	{
		TRACE_ERROR(  "--Exception---------------------");
		TRACE_ERROR(  "FillBuffer...");
		TRACE_ERROR(  "--------------------------------");
		
		hr=E_FAIL;//This will cause the filter to stop
	}

	return hr;
}

/**
Send our video buffer the live media provider who will copy
the next frame from the queue
*/
bool CPushPinRTSP::ProcessVideo(IMediaSample *pSample)
{
	bool rc=true;
	long cbData;
	BYTE *pData;


	// Access the sample's data buffer
	pSample->GetPointer(&pData);
	cbData = pSample->GetSize();
	long bufferSize=cbData;

	rc=m_streamMedia->GetFrame(pData, bufferSize);

	if(rc)
	{
		m_lostFrameBufferCount=0;
		m_currentVideoState=Playing;
	}else{
		//paint black video to indicate a lose
		int count=((CPushSourceRTSP*)this->m_pFilter)->m_transportUrl->get_LostFrameCount();
		if(m_lostFrameBufferCount>count)
		{
			if(!(m_currentVideoState==VideoState::Lost))
			{
				TRACE_INFO("Lost frame count (%d) over limit {%d). Paint Black Frame",m_lostFrameBufferCount, count );
				//HelpLib::TraceHelper::WriteInfo("Lost frame count over limit. Paint Black Frame and shutdown.");
				memset(pData,0, bufferSize);
				m_currentVideoState=VideoState::Lost;
				rc=true;
			} else{
				TRACE_INFO("Shutting Down");
				rc=false;
			}
		
			
		}else{
			m_lostFrameBufferCount++;
			rc = true;
			//if(m_currentVideoState==VideoState::Lost) Sleep(1000);

		}
	}
	return rc;
}


/**
Helper function
Attempt to reload lost video.  We may lose video for a number of reasons
such as a network outage or power failure.  This method will attempt to
reconnect to the video stream.  It must reconnect to the same stream as
the orginal connection otherwise the video buffer will not be the correct
size.
*/
bool CPushPinRTSP::ReloadVideo(TransportUrl * url)
{
	bool rc=true;
	int ret = 0;
	try
	{
		TRACE_DEBUG( "Reload video");
		Stop();
		m_lostFrameBufferCount=0;
		m_bufferCount=0;
		//TransportUrl *url=((CPushSourceRTSP*)this->m_pFilter)->m_transportUrl;
		if(m_streamMedia!=NULL) delete m_streamMedia;
		m_streamMedia=NULL;
		m_streamMedia=new CstreamMedia();
		ret =  m_streamMedia->rtspClientOpenStream((const char *)url->get_RtspUrl());
		if(ret!=0)
		{
			TRACE_ERROR("Unable to open stream ret=%d", ret);
			rc=false;
		}else{
			ret = m_streamMedia->rtspClientPlayStream(url->get_RtspUrl());
			if(ret!=0)
			{
				TRACE_ERROR("Unable to open stream ret=%d", ret);
				rc=false;
			}
		}

		/*if we have not received a frame then switch to unicast for the next try
		if(!m_firstFrame)
		{
			TRACE_DEBUG( "Reload using unicast connection");
		}
		*/
	}
	catch(...)
	{
		TRACE_ERROR( "Reload Video unexpected error");
		rc=false;
	}

	return rc;
}

/**
* Any special work that needs to happen when the playback thread
* is created.
*/
HRESULT CPushPinRTSP::OnThreadCreate(void)
{
	HRESULT hr=S_OK;
	hr=CSourceStream::OnThreadCreate();
	TransportUrl *url=((CPushSourceRTSP*)this->m_pFilter)->m_transportUrl;

	TRACE_INFO("Open Stream");
	if(m_streamMedia==NULL)
		m_streamMedia=new CstreamMedia();
	int ret =  m_streamMedia->rtspClientOpenStream(url->get_RtspUrl());
	if(ret!=0)
	{
		TRACE_ERROR("Unable to open stream ret=%d", ret);
		hr=E_FAIL;
	}

	TRACE_ERROR( "OnThreadCreate.  HRESULT =  %#x", hr);
	return hr;
}
/**
* Cleanup once the playback thread is killed
*/
HRESULT CPushPinRTSP::OnThreadDestroy(void)
{
	CAutoLock cAutoLockShared(&m_cSharedState);
	HRESULT hr=S_OK;
//	CSourceStream::Stop();
	hr=CSourceStream::OnThreadDestroy();
	int ret =  m_streamMedia->rtspClientCloseStream();

	if(m_streamMedia!=NULL)
		delete m_streamMedia;
	m_streamMedia=NULL;

	TRACE_INFO( "OnThreadDestroy.  HRESULT = %#x", hr);
	return hr;
}

/**
* FillBuffer is about to get called for the first time
* do any prep work here
*/
HRESULT CPushPinRTSP::OnThreadStartPlay(void)
{
	HRESULT hr=S_OK;
	hr=CSourceStream::OnThreadStartPlay();
	TRACE_INFO("Play Stream");
	TransportUrl *url=((CPushSourceRTSP*)this->m_pFilter)->m_transportUrl;
	int ret =  m_streamMedia->rtspClientPlayStream(url->get_RtspUrl());
	if(ret!=0)
	{
		TRACE_ERROR("Unable to play stream ret=%d", ret);
		hr=E_FAIL;
	}

	TRACE_DEBUG( "OnThreadStartPlay.  HRESULT =  %#x", hr);
	return hr;
}
/**
* Stopping FillBuffer and playback thread
*/
HRESULT CPushPinRTSP::Stop(void)
{
	CAutoLock cAutoLockShared(&m_cSharedState);
	HRESULT hr=S_OK;
	TRACE_DEBUG( "Stopping playback");
	if(m_streamMedia!=NULL)
	{
		int ret =  m_streamMedia->rtspClientCloseStream();
		if (ret < 0)
		{
			TRACE_ERROR( "Unable to close rtsp video stream ret=%d", ret);
			return E_FAIL;	
		}
	}
	TRACE_INFO( "Stop.  HRESULT = %#x", hr);
	return hr; 
}
