#include "stdafx.h"
#include "MyRTSPClient.h"
#include "MyUsageEnvironment.h"
#include "live.h"


#define keepAliveTimer 60 * 1000000 //how many micro seconds between each keep alive

static unsigned rtspClientCount = 0; // Counts how many streams (i.e., "RTSPClient"s) are currently in use.

HANDLE hDataThreadReady;

/**
opens the RTSPClient stream
*/
void openURL(UsageEnvironment& env, char const* rtspURL);

void openURL(UsageEnvironment& env, char const* rtspURL);
// RTSP 'response handlers':
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);

void onScheduledDelayedTask(void* clientData);

// Other event handler functions:
void subsessionAfterPlaying(void* clientData); // called when a stream's subsession (e.g., audio or video substream) ends
void subsessionByeHandler(void* clientData); // called when a RTCP "BYE" is received for a subsession
void streamTimerHandler(void* clientData);
static void StreamClose(void *p_private);
//void StreamClose(void *p_private);

// called at the end of a stream's expected duration (if the stream has not already signaled its end using a RTCP "BYE")

// Used to iterate through each stream's 'subsessions', setting up each one:
void setupNextSubsession(RTSPClient* rtspClient);

// Used to shut down and close a stream (including its "RTSPClient" object):
void shutdownStream(RTSPClient* rtspClient, int exitCode = 1);

// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const RTSPClient& rtspClient) {
	return env << "[URL:\"" << rtspClient.url() << "\"]: ";
}

// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const MediaSubsession& subsession) {
	return env << subsession.mediumName() << "/" << subsession.codecName();
}



/*char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";*/
static int b64_decode( char *dest, char *src )
{
	const char *dest_start = dest;
	int  i_level;
	int  last = 0;
	int  b64[256] = {
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
		52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
		-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
		15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
		-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
		41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
	};

	for( i_level = 0; *src != '\0'; src++ )
	{
		int  c;

		c = b64[(unsigned int)*src];
		if( c == -1 )
		{
			continue;
		}

		switch( i_level )
		{
		case 0:
			i_level++;
			break;
		case 1:
			*dest++ = ( last << 2 ) | ( ( c >> 4)&0x03 );
			i_level++;
			break;
		case 2:
			*dest++ = ( ( last << 4 )&0xf0 ) | ( ( c >> 2 )&0x0f );
			i_level++;
			break;
		case 3:
			*dest++ = ( ( last &0x03 ) << 6 ) | c;
			i_level = 0;
		}
		last = c;
	}

	*dest = '\0';

	return dest - dest_start;
}

static unsigned char* parseH264ConfigStr( char const* configStr,
	unsigned int& configSize )
{
	char *dup, *psz;
	int i, i_records = 1;

	if( configSize )
		configSize = 0;

	if( configStr == NULL || *configStr == '\0' )
		return NULL;

	psz = dup = _strdup( configStr );

	/* Count the number of comma's */
	for( psz = dup; *psz != '\0'; ++psz )
	{
		if( *psz == ',')
		{
			++i_records;
			*psz = '\0';
		}
	}

	unsigned char *cfg = new unsigned char[5 * strlen(dup)];
	psz = dup;
	for( i = 0; i < i_records; i++ )
	{
		cfg[configSize++] = 0x00;
		cfg[configSize++] = 0x00;
		cfg[configSize++] = 0x00;
		cfg[configSize++] = 0x01;

		configSize += b64_decode( (char*)&cfg[configSize], psz );
		psz += strlen(psz)+1;
	}

	//free( dup );
	return cfg;
}


/*------------------------------------------------------------*/
/*------------------------------------------------------------*/
static void TaskInterrupt( void *p_private )
	{
		char *event = (char*)p_private;

		/* Avoid lock */
		*event = 0xff;
		TRACE_ERROR( "TaskInterrupt");
	}

DWORD WINAPI rtspRecvDataThread( LPVOID lpParam )
{
	TRACE_INFO("Starting receive data thread");
	CstreamMedia* rtspClient = (CstreamMedia*)lpParam;
	char*  event = &(rtspClient->event);

	// Begin by setting up our usage environment:
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment* env = CMyUsageEnvironment::createNew(*scheduler);

	// Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
	// to receive (even if more than stream uses the same "rtsp://" URL).
	MyRTSPClient* client = MyRTSPClient::createNew(*env, rtspClient->get_Url(), rtspClient->get_Format());

	if (client == NULL) {
		TRACE_ERROR("Failed to create a RTSP client for URL %s, Msg %s", rtspClient->get_Url(),  env->getResultMsg());
		return -1;
	}

	client->set_StreamTrack(rtspClient->stream[0]);
	++rtspClientCount;

	rtspClient->scheduler=scheduler;
	rtspClient->env=env;
	rtspClient->rtsp=client;


	// Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
	// Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
	// Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
	TRACE_INFO("Sending describe command");
	client->sendDescribeCommand(continueAfterDESCRIBE); 

	// All subsequent activity takes place within the event loop:
	TRACE_INFO("Starting the event loop");
	env->taskScheduler().doEventLoop(); // does not return

	return 0; // We never actually get to this line; this is only to prevent a possible compiler warning

}

/**
Begin by creating a "RTSPClient" object.  Note that there is a
separate "RTSPClient" object for each stream that we wish
to receive (even if more than stream uses the same "rtsp://" URL).
not that we need that in this filter
*/
void openURL(UsageEnvironment& env,  char const* rtspURL) {
	// Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
	// to receive (even if more than stream uses the same "rtsp://" URL).
	TRACE_INFO("open URL %s ",rtspURL);
	MyRTSPClient* client = MyRTSPClient::createNew(env, rtspURL, "H264");
	if (client == NULL) {
		TRACE_ERROR("Failed to create a RTSP client msg: %s for URL %s ",env.getResultMsg(), rtspURL);
		env << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " << env.getResultMsg() << "\n";
		return;
	}

	++rtspClientCount;

	// Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
	// Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
	// Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
	client->sendDescribeCommand(continueAfterDESCRIBE); 
}


/*------------------------------------------------------------*/
// Implementation of the RTSP 'response handlers':
/*------------------------------------------------------------*/
/**
Response to the sendDescribeCommand 
*/
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString) {
	TRACE_INFO("Continue after describe Code=%d Result=%s",resultCode,resultString);

	UsageEnvironment& env = rtspClient->envir(); // alias

	StreamClientState& scs = ((MyRTSPClient*)rtspClient)->scs; // alias

	if (resultCode != 0) {
		TRACE_ERROR(  "Failed to get a SDP description:");
		goto cleanup;
	}

	char* sdpDescription = resultString;
	TRACE_INFO("Got a SDP description: %s", sdpDescription);

	// Create a media session object from this SDP description:
	scs.session = MediaSession::createNew(env, sdpDescription);
	delete[] sdpDescription; // because we don't need it anymore
	if (scs.session == NULL) {
		TRACE_ERROR( "Failed to create a MediaSession object from the SDP description: {0}",env.getResultMsg());
		goto cleanup;
	} else if (!scs.session->hasSubsessions()) {
		TRACE_ERROR( "This session has no media subsessions (i.e., no m=lines)");
		goto cleanup;
	}

	// Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
	// calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
	// (Each 'subsession' will have its own data source.)
	scs.iter = new MediaSubsessionIterator(*scs.session);
	setupNextSubsession(rtspClient);
	return;

	// An unrecoverable error occurred with this stream.
cleanup:
	shutdownStream(rtspClient);

}

/**
Called after the DESCRIBE response
*/
void setupNextSubsession(RTSPClient* rtspClient) 
{
	TRACE_INFO("Setup next session");
	UsageEnvironment& env = rtspClient->envir(); // alias
	StreamClientState& scs = ((MyRTSPClient*)rtspClient)->scs; // alias

	scs.subsession = scs.iter->next();

	if (scs.subsession != NULL) 
	{
		if (!scs.subsession->initiate()) 
		{
			TRACE_ERROR( "Failed to initiate the subsession: &s",env.getResultMsg());
			setupNextSubsession(rtspClient); // give up on this subsession; go to the next one
		} else {
			TRACE_INFO("Initiated the subsession. Ports %d-%d", scs.subsession->clientPortNum(), scs.subsession->clientPortNum()+1);
			// Continue setting up this subsession, by sending a RTSP "SETUP" command:
			rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP);
		}
		return;
	}

	// We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
	scs.duration = scs.session->playEndTime() - scs.session->playStartTime();

	TRACE_INFO("Sending Play Command");
	rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
}

/**
Response to the sendSetupCommand
*/
void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) {
	TRACE_INFO("Continue after setup Code=%d Result=%s",resultCode, resultString);
	do {
		UsageEnvironment& env = rtspClient->envir(); // alias
		StreamClientState& scs = ((MyRTSPClient*)rtspClient)->scs; // alias

		if (resultCode != 0) {
			TRACE_ERROR( "Failed to set up the subsession: %s", env.getResultMsg());
			break;
		}

		TRACE_INFO( "Set up the subsession (client ports %d - %d)",scs.subsession->clientPortNum(),  scs.subsession->clientPortNum());

		// Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
		// (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening until later,
		// after we've sent a RTSP "PLAY" command.)
		MyRTSPClient* client=(MyRTSPClient*)rtspClient;
		scs.subsession->sink = client->get_sink();

		if (scs.subsession->sink == NULL) {
			TRACE_ERROR( "Failed to set up the subsession: %s", env.getResultMsg());
			break;
		}

		TRACE_INFO( "Created a data sink for the subsession");
		scs.subsession->miscPtr = rtspClient; // a hack to let subsession handle functions get the "RTSPClient" from the subsession 
		scs.subsession->sink->startPlaying(*(scs.subsession->readSource()),
			subsessionAfterPlaying, scs.subsession);
		// Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
		if (scs.subsession->rtcpInstance() != NULL) {
			scs.subsession->rtcpInstance()->setByeHandler(subsessionByeHandler, scs.subsession);
		}
	} while (0);

	// Set up the next subsession, if any:
	setupNextSubsession(rtspClient);
}


/**
Response to the sendSetupCommand
*/
void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString) {
	TRACE_INFO("Continue after play Code=%d Result=%s",resultCode, resultString);

	UsageEnvironment& env = rtspClient->envir(); // alias
	StreamClientState& scs = ((MyRTSPClient*)rtspClient)->scs; // alias


	if (resultCode != 0) 
	{
		TRACE_ERROR( "Failed to start playing session: %s", resultString);
		shutdownStream(rtspClient);
		return;
	}

	SetEvent(hDataThreadReady);		
	TRACE_INFO( "Started playing session");

	//setup our keep alive using a delayed task
	TRACE_INFO( "Schedule keep alive task");
	env.taskScheduler().scheduleDelayedTask(keepAliveTimer, onScheduledDelayedTask, rtspClient);
}

/*------------------------------------------------------------*/
// Implementation of the other event handlers:
/*------------------------------------------------------------*/
void subsessionAfterPlaying(void* clientData) 
{
	TRACE_INFO("Session after playing");
	MediaSubsession* subsession = (MediaSubsession*)clientData;
	RTSPClient* rtspClient = (RTSPClient*)(subsession->miscPtr);

	// Begin by closing this subsession's stream:
	Medium::close(subsession->sink);
	subsession->sink = NULL;

	// Next, check whether *all* subsessions' streams have now been closed:
	MediaSession& session = subsession->parentSession();
	MediaSubsessionIterator iter(session);
	while ((subsession = iter.next()) != NULL) 
	{
		if (subsession->sink != NULL) 
			return; // this subsession is still active
	}

	// All subsessions' streams have now been closed, so shutdown the client:
	shutdownStream(rtspClient);
}

/**
*/
void subsessionByeHandler(void* clientData) 
{
	TRACE_INFO("Session after bye");
	MediaSubsession* subsession = (MediaSubsession*)clientData;
	RTSPClient* rtspClient = (RTSPClient*)subsession->miscPtr;
	UsageEnvironment& env = rtspClient->envir(); // alias

	TRACE_INFO( "Received RTCP \"BYE\" on subsession");

	// Now act as if the subsession had closed:
	subsessionAfterPlaying(subsession);
}

/**
The stream has timed out and needs to be shutdown
*/
void streamTimerHandler(void* clientData) 
{
	TRACE_INFO("Stream timer handler");
	MyRTSPClient* rtspClient = (MyRTSPClient*)clientData;
	StreamClientState& scs = rtspClient->scs; // alias

	scs.streamTimerTask = NULL;

	// Shut down the stream:
	shutdownStream(rtspClient);
}

/**
Cleanup abd shutdown the stream
*/
void shutdownStream(RTSPClient* rtspClient, int exitCode) 
{
	TRACE_INFO("Shutdown stream ExitCode=%d",exitCode);
	UsageEnvironment& env = rtspClient->envir(); // alias
	StreamClientState& scs = ((MyRTSPClient*)rtspClient)->scs; // alias

	// First, check whether any subsessions have still to be closed:
	if (scs.session != NULL) { 
		Boolean someSubsessionsWereActive = False;
		MediaSubsessionIterator iter(*scs.session);
		MediaSubsession* subsession;

		while ((subsession = iter.next()) != NULL) {
			if (subsession->sink != NULL) {
				Medium::close(subsession->sink);
				subsession->sink = NULL;
				someSubsessionsWereActive = True;
			}
		}

		if (someSubsessionsWereActive) {
			// Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
			// Don't bother handling the response to the "TEARDOWN".
			rtspClient->sendTeardownCommand(*scs.session, NULL);
		}
	}

	TRACE_INFO( "Closing the stream");
	Medium::close(rtspClient);
	// Note that this will also cause this stream's "StreamClientState" structure to get reclaimed.

	if (--rtspClientCount == 0) {
		// The final stream has ended, so exit the application now.
		// (Of course, if you're embedding this code into your own application, you might want to comment this out.)
		exit(exitCode);
	}
}

void onScheduledDelayedTask(void* clientData)
{
	try
	{
		TRACE_DEBUG("On delayed task (ie Keep Alive)");
		RTSPClient* rtspClient = (RTSPClient*)(clientData);
		UsageEnvironment& env = rtspClient->envir(); // alias
		StreamClientState& scs = ((MyRTSPClient*)rtspClient)->scs; // alias
		MediaSession *session = scs.session;

		char * ret =NULL;
		// Get the session parameter to do a keep alive
		rtspClient->getMediaSessionParameter(*session,NULL,ret);
		free(ret);

		//setup our keep alive using a delayed task
		env.taskScheduler().scheduleDelayedTask(keepAliveTimer, onScheduledDelayedTask, rtspClient);

	}
	catch(...)
	{
		TRACE_ERROR("Exception thrown during keep alive");
	}
}

/*------------------------------------------------------------*/
// CstreamMedia class implementation
/*------------------------------------------------------------*/

/**
Constructor
*/
CstreamMedia::CstreamMedia()
{
	stream		= NULL;
	ms 			= NULL;
	scheduler 	= NULL;
	env  		= NULL;
	rtsp 		= NULL;
	b_tcp_stream = 0; // normal udp
	i_stream = 0;
	nostream = 0;
	event		 = 0;
	m_state  = RTSP_STATE_IDLE;
	m_recvThreadFlag = TRUE;
	hDataThreadReady=NULL;
	hRecvEvent=NULL;
	hRecvDataThread=NULL;
	hFrameListLock=NULL;
}

/**
Destructor
Cleanup the streams
*/
CstreamMedia::~CstreamMedia()
{	
	StreamTrack *tr;
	m_state  = RTSP_STATE_IDLE;			
	m_recvThreadFlag = FALSE;		
	if (stream!=NULL)
	{			
		for (int i = 0; i < i_stream; i++)
		{
			tr = (StreamTrack*)stream[i];
			if (tr)
				delete tr;
		}
		free(stream);
	}
	stream = NULL;
			
	if (ms!=NULL)
		Medium::close(ms);
	ms = NULL;

	if(rtsp!=NULL) 
	{
		RTSPClient::close(rtsp);
		MyRTSPClient * crtsp=(MyRTSPClient*)rtsp;
		delete crtsp;
	}			
	rtsp = NULL;

	if(env!=NULL)
		env->reclaim();
	env = NULL;

	if (scheduler!=NULL)
		delete scheduler;
	scheduler = NULL;

	//CloseHandle(hRecvEvent);
	//CloseHandle(hRecvDataThread);
}

/**
Opens an RTSP stream using the supplied filename as the URL
returns 0 if the stream is opened successfully
*/
int CstreamMedia::rtspClientOpenStream(const char* url)
{
	TRACE_INFO("RTSP Open Stream URL=%s",url);
	if (m_state >= RTSP_STATE_OPENED)
	{
		TRACE_ERROR("rtspClientOpenStream already open");
		return 0;
	}

	int verbosityLevel = 0;
	int tunnelOverHTTPPortNum = 0;
	char* sdpDescription;
	MediaSubsessionIterator *iter   = NULL;
	MediaSubsession         *sub    = NULL;
	StreamTrack *tk;

	stream		= NULL;
	ms 			= NULL;
	scheduler 	= NULL;
	env  		= NULL;
	rtsp 		= NULL;	 
	i_stream = 0;
	nostream = 0;
	event  = 0;

	TRACE_DEBUG("create BasicTaskScheduler");
	scheduler =  BasicTaskScheduler::createNew();
	if (scheduler ==	NULL)
	{
		TRACE_ERROR( "BasicTaskScheduler fail");
		goto fail;
	}

	TRACE_DEBUG("create BasicUsageEnvironment");
	env = CMyUsageEnvironment::createNew(*scheduler);
	if (env == NULL)
	{
		TRACE_ERROR( "BasicUsageEnvironment fail");
		goto fail;
	}

	TRACE_DEBUG("create RTSPClient");
	rtsp = RTSPClient::createNew(*env, verbosityLevel, "MyFilter");
	if (rtsp == NULL)
	{		 
		TRACE_ERROR( "create rtsp client fail");
		goto fail;
	}

	TRACE_DEBUG("send rtp options");
	if (rtsp->sendOptionsCmd(url) == NULL)
	{
		TRACE_ERROR( "send optioncmd fail");
		goto fail;
	}

	sdpDescription = rtsp->describeURL(url);
	if (sdpDescription == NULL)
	{
		TRACE_ERROR( "rtspClient->describeStatus");	
		goto fail;
	}

	ms = MediaSession::createNew(*env, sdpDescription);
	delete[] sdpDescription;
	if (ms == NULL)
	{
		TRACE_ERROR( "create MediaSession fail");		
		goto fail;
	}

	iter = new MediaSubsessionIterator(*ms);
	while( ( sub = iter->next() ) != NULL )
	{
		Boolean bInit;

		int i_buffer = 0;
		unsigned const thresh = 200000; /* RTP reorder threshold .2 second (default .1) */

		if( !strcmp( sub->mediumName(), "video" ) )
			i_buffer = 2000000;
		else continue;

		if( !strcmp( sub->codecName(), "X-ASF-PF" ) )
			bInit = sub->initiate( 4 ); /* Constant ? */
		else
			bInit = sub->initiate();

		if( !bInit )
		{
			TRACE_ERROR( "RTP subsession {0} / {1} failed",sub->mediumName(),sub->codecName());
		}
		else
		{
			if( sub->rtpSource() != NULL )
			{
				int fd = sub->rtpSource()->RTPgs()->socketNum();

				/* Increase the buffer size */
				if( i_buffer > 0 )
					increaseReceiveBufferTo(*env, fd, i_buffer );

				/* Increase the RTP reorder timebuffer just a bit */
				sub->rtpSource()->setPacketReorderingThresholdTime(thresh);
			}


			/* Issue the SETUP */
			if(rtsp)
			{
				if( !rtsp->setupMediaSubsession(*sub, false, b_tcp_stream))
				{
					/* if we get an unsupported transport error, toggle TCP use and try again */
					if( !strstr(env->getResultMsg(), "461 Unsupported Transport")
						|| !rtsp->setupMediaSubsession( *sub, false,b_tcp_stream ? false : true))
					{
						TRACE_ERROR( "SETUP of '%s/%s failed (%s)",sub->mediumName(),sub->codecName(),env->getResultMsg()) ;
						continue;
					}
				}
			}

			/* Check if we will receive data from this subsession for this track */
			if( sub->readSource() == NULL )
				continue;

			tk = new StreamTrack;
			if( !tk )
			{
				delete iter;
				TRACE_ERROR( "Failed to allocate a stream track! Yikes!!");
				return (-1);
			}
			tk->pstreammedia = this;
			tk->sub         = sub;			
			tk->waiting   = 0;
			ZeroMemory(&tk->mediainfo, sizeof(MediaInfo));
			tk->mediainfo.b_packetized =  1;
			strncpy(tk->mediainfo.codecName,sub->codecName(), sizeof(tk->mediainfo.codecName));
			tk->mediainfo.duration = sub->playEndTime()-sub->playStartTime();

			if( !strcmp( sub->mediumName(), "video" ) )
			{
				TRACE_INFO("Found video session");
				tk->i_buffer = DUMMY_SINK_RECEIVE_BUFFER_SIZE;
				tk->p_buffer = new char[tk->i_buffer];
				tk->mediainfo.codecType = CODEC_TYPE_VIDEO;
				tk->mediainfo.i_format = FOURCC('u','n','d','f');
				tk->mediainfo.video.fps = sub->videoFPS();
				tk->mediainfo.video.height = sub->videoHeight();
				tk->mediainfo.video.width = sub->videoWidth();
				TRACE_INFO("codec = %s", sub->codecName());
				if( !strcmp( sub->codecName(), "MPV" ) )
				{
					tk->mediainfo.i_format = FOURCC( 'm', 'p', 'g', 'v' );
				}
				else if( !strcmp( sub->codecName(), "H263" ) ||
					!strcmp( sub->codecName(), "H263-1998" ) ||
					!strcmp( sub->codecName(), "H263-2000" ) )
				{
					tk->mediainfo.i_format = FOURCC( 'H', '2', '6', '3' );
				}
				else if( !strcmp( sub->codecName(), "H261" ) )
				{
					tk->mediainfo.i_format = FOURCC( 'H', '2', '6', '1' );
				}
				else if( !strcmp( sub->codecName(), "H264" ) )
				{
					unsigned int i_extra = 0;
					char      *p_extra = NULL;

					tk->mediainfo.i_format = FOURCC( 'h', '2', '6', '4' );
					tk->mediainfo.b_packetized  = false;
					this->m_format = "H264";

					if((p_extra=(char *)parseH264ConfigStr( sub->fmtp_spropparametersets(), i_extra ) ) )
					{
						tk->mediainfo.extra_size = i_extra;
						tk->mediainfo.extra  = new char[i_extra];
						memcpy(tk->mediainfo.extra, p_extra, i_extra );

						delete[] p_extra;
					}
				}
				else if( !strcmp( sub->codecName(), "JPEG" ) )
				{
					tk->mediainfo.i_format = FOURCC( 'M', 'J', 'P', 'G' );
					this->m_format = "JPEG";
				}
				else if( !strcmp( sub->codecName(), "MP4V-ES" ) )
				{
					unsigned int i_extra;
					char      *p_extra;

					tk->mediainfo.i_format = FOURCC( 'm', 'p', '4', 'v' );
					this->m_format = "MP4V";

					if( ( p_extra = (char *)parseGeneralConfigStr( sub->fmtp_config(), i_extra ) ) )
					{
						tk->mediainfo.extra_size = i_extra;
						tk->mediainfo.extra  = new char[i_extra];
						memcpy(tk->mediainfo.extra, p_extra, i_extra );
						delete[] p_extra;
					}
				}
				else if( !strcmp( sub->codecName(), "MP2P" ) ||
					!strcmp( sub->codecName(), "MP1S" ) )
				{
					;
				}
				else if( !strcmp( sub->codecName(), "X-ASF-PF" ) )
				{
					;
				}
			}


			if( sub->rtcpInstance() != NULL )
			{
				;//sub->rtcpInstance()->setByeHandler( StreamClose, tk );
			}

			stream = (StreamTrack**)realloc( stream, sizeof( StreamTrack ) * (i_stream+1));
			stream[i_stream++] = tk;

		}

	}//while find track;

	delete iter;
	if (i_stream <= 0)
	{
		TRACE_ERROR( "don't find stream");
		goto fail;
	}
	m_state = RTSP_STATE_OPENED;

	return(0);

fail:
	m_state = RTSP_STATE_IDLE;
	if (stream==NULL)
		free(stream);
	stream = NULL;

	if (ms==NULL)
		Medium::close(ms);
	ms = NULL;

	if(rtsp==NULL) 
	{
		RTSPClient::close(rtsp);			
		MyRTSPClient * crtsp=(MyRTSPClient*)rtsp;
		delete crtsp;
	}
	rtsp = NULL;

	if(env==NULL)
		env->reclaim();
		env = NULL;
	
	if (scheduler==NULL)
		delete scheduler;
	scheduler = NULL;

	return(-1);
}

/**
Start streaming video from the open stream
/sa CstreamMedia::rtspClientOpenStream(const char* url)
*/
int CstreamMedia::rtspClientPlayStream(const char* url)
{
	TRACE_INFO("RTSP play Stream URL=%s",url);
	this->m_url=url;
	if (m_state == RTSP_STATE_OPENED)
	{
		event          = 0;
		hDataThreadReady = CreateEvent(NULL, FALSE, FALSE, NULL);
		hRecvEvent     = CreateEvent(NULL, FALSE, FALSE, NULL);
		hRecvDataThread= CreateThread(NULL, 0, rtspRecvDataThread, this, 0, NULL);

		Sleep(5);
	}
	m_state  = RTSP_STATE_PLAYING;

	return 0;
}

/**
Query the RTSP stream for media information
ie width and height of the frame

*/
int CstreamMedia::rtspClinetGetMediaInfo(enum CodecType codectype, MediaInfo& mediainfo)
{
	TRACE_INFO("RTSP get media info");
	StreamTrack *tr;
	for (int i = 0; i < i_stream; i++)
	{
		tr = (StreamTrack*)stream[i];
		if (tr->mediainfo.codecType == codectype)
		{
			mediainfo = tr->mediainfo;
			return(0);
		}
	}

	return(-1);
}

/**
close the stream and clean things up
*/
int CstreamMedia::rtspClientCloseStream(void)
{
	TRACE_INFO("RTSP close stream");
	StreamTrack *tr;

	if (m_state == RTSP_STATE_IDLE)
	{
		TRACE_WARN( "nothting to do?");
		return(0);
	}

	event  = 0xff;


	TRACE_INFO( "Tear down the media session");
	if (rtsp!= NULL  && ms !=NULL)
	{
		rtsp->teardownMediaSession(*ms);
	}

	m_state = RTSP_STATE_IDLE;
	nostream = 1;

	TRACE_INFO( "Terminate the receive thread");
	if (WaitForSingleObject(hRecvEvent, 500) == WAIT_TIMEOUT)
	{
		TRACE_INFO( "rtspClientCloseStream WAIT_TIMEOUT");
		DWORD dwExitCode = 0;
		TerminateThread(hRecvDataThread, dwExitCode);
	}


	if (stream)
	{			
		for (int i = 0; i < i_stream; i++)
		{
			tr = (StreamTrack*)stream[i];
			if (tr)
			{
				delete tr;
			}

		}

		free(stream);
		stream = NULL;
	}

	if (ms !=NULL)
		Medium::close(ms);
	ms = NULL;

	if(rtsp!=NULL) 
		RTSPClient::close(rtsp);			
	rtsp = NULL;

	if(env!=NULL)
		env->reclaim();
	env = NULL;

	if (scheduler!=NULL)
		delete scheduler;
	scheduler = NULL;

	if (hDataThreadReady != INVALID_HANDLE_VALUE)
		CloseHandle(hDataThreadReady);
	hDataThreadReady=NULL;

	if (hRecvEvent != INVALID_HANDLE_VALUE)
		CloseHandle(hRecvEvent);
	hRecvEvent=NULL;

	if (hRecvDataThread != INVALID_HANDLE_VALUE)
		CloseHandle(hRecvDataThread);
	hRecvDataThread=NULL;

	TRACE_INFO( "rtspClientCloseStream");
	return(0);
}

/**
Get a video frame from the sink's video queue
*/
bool CstreamMedia::GetFrame(BYTE *pData, int bufferSize)
{
	FrameInfo* frame = NULL;
	if((MyRTSPClient*)rtsp==NULL)
	{
		TRACE_ERROR( "Lost RTSP session graph needs to be  restarted");
		return false;
	}

	if(m_state  != RTSP_STATE_PLAYING)
	{
		TRACE_DEBUG( "RTSP stream is not playing");
		return false;
	}

	try{
		MyVideoSink *sink = ((MyRTSPClient*)rtsp)->get_sink(); // alias
		if(sink!=NULL && sink->get_FrameQueue() != NULL)
		{
			frame  = sink->get_FrameQueue()->get();
			if (frame!=NULL && bufferSize>=frame->frameHead.FrameLen)
			{
				memcpy((char *)pData, frame->pdata, frame->frameHead.FrameLen);
				TRACE_DEBUG( "!!rtspClientReadFrame len = %d count = %d", frame->frameHead.FrameLen , sink->get_FrameQueue()->get_Count());		
				DeleteFrame(frame);
				return (true);
			}		

			TRACE_ERROR( "no frames left in queue rtspClientReadFrame fail");
			return false;
		}
		else{
			TRACE_ERROR( "Lost the video sink, better result the graph");
		}
	}
	catch(...)
	{
		TRACE_ERROR("Exception get frame");
	}
	return false;
}

/*
Sense we allocated the frame on our heap
we need to destory it
**/
void CstreamMedia::DeleteFrame(FrameInfo* frame)
{
	try
	{
		if(frame!=NULL)
			free(frame);
		frame=NULL;
	}
	catch(...)
	{
	}
}

/**
Report back the number of frames left in our video queue
/sa MyRTSPClient
*/
int CstreamMedia::GetQueueSize()
{
	if(rtsp==NULL)
	{
		return 0;
	}
	MyVideoSink *sink = ((MyRTSPClient*)rtsp)->get_sink(); // alias
	if(sink==NULL || sink->get_FrameQueue()==NULL)
	{
		return 0;
	}
	return sink->get_FrameQueue()->get_Count();
}