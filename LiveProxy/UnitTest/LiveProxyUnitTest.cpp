// PushSourceUnitTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <stdlib.h>
#include <conio.h>
#include <fstream>
#include <ctime>

//#include "..\Cisco\PushSource\TransportUrl.h"
#include "..\trace.h"
#include "..\live.h"
#include "..\MediaQueue.h"
#include "..\VideoDecoder.h"
#include "..\MyUsageEnvironment.h"

std::ofstream logFile;
bool FileExists(const char *fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}
int countLines(LPCSTR file) { 
    int number_of_lines = 0;
    std::string line;
    std::ifstream myfile(file);

    while (std::getline(myfile, line))
        ++number_of_lines;
    return number_of_lines;
}
void Write(int pass, char * lpszFormat, ...)
{
	static char szMsg[512];
	va_list argList;
	va_start(argList, lpszFormat);
	try
	{
		vsprintf(szMsg,lpszFormat, argList);

		WORD color=FOREGROUND_BLUE;
				
		if(pass==1)
			color=FOREGROUND_GREEN;

		if(pass==2)
			color=FOREGROUND_RED;

		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);  // Get handle to standard output
		SetConsoleTextAttribute(hConsole, color|BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE|BACKGROUND_INTENSITY);
		std::cout << szMsg << std::endl;
		logFile << szMsg << std::endl;
	}
	catch(...)
	{
		strcpy(szMsg ,"DebugHelper:Invalid string format!");
	}
	va_end(argList);
}
//bool TransportTest(LPCTSTR str, const char * server, const char * port, const char *camera,const char * domain, const char * user, const char * password, 
//	int width, int height, bool secure, int framerate,  int loglevel, int frameqsize,const char * token, const char * sessionID)
//{
//	USES_CONVERSION;
//	bool rc=true;
//	Write(1,T2A(str));
//	TransportUrl *url=new TransportUrl(str);
//
//	//scheme
//	if(0==strcmp(url->get_Scheme(),"ciscosource"))
//	{
//		Write(1,"\tPASS: Scheme");
//	}else{
//		Write(2,"\tFAIL: Scheme = %s",url->get_Scheme());
//		rc=false;
//	}
//
//	//domain
//	if(0==strcmp(url->get_Domain(),domain))
//	{
//		Write(1,"\tPASS: Domain");
//	}else{
//		Write(2,"\tFAIL: Domain = %s",url->get_Domain());
//		rc=false;
//	}
//
//	//server
//	if(0==strcmp(url->get_Server(),server))
//	{
//		Write(1,"\tPASS: Server");
//	}else{
//		Write(2,"\tFAIL: Server = %s",url->get_Server());
//		rc=false;
//	}
//
//	//port
//	if(0==strcmp(url->get_Port(),port))
//	{
//		Write(1,"\tPASS: Port");
//	}else{
//		Write(2,"\tFAIL: Port = %s",url->get_Port());
//		rc=false;
//	}
//	//camera
//	if(0==strcmp(url->get_Camera(),camera))
//	{
//		Write(1,"\tPASS: Camera");
//	}else{
//		Write(2,"\tFAIL: Camera = %s",url->get_Camera());
//		rc=false;
//	}
//	//user
//	if(0==strcmp(url->get_UserName(),user))
//	{
//		Write(1,"\tPASS: User");
//	}else{
//		Write(2,"\tFAIL: User = %s",url->get_UserName());
//		rc=false;
//	}
//	//password
//	if(0==strcmp(url->get_Password(),password))
//	{
//		Write(1,"\tPASS: Password");
//	}else{
//		Write(2,"\tFAIL: Password = %s",url->get_Password());
//		rc=false;
//	}
//	//width
//	if(url->get_Width()==width)
//	{
//		Write(1,"\tPASS: Width");
//	}else{
//		Write(2,"\tFAIL: Width = %d",url->get_Width());
//		rc=false;
//	}
//	//Height
//	if(url->get_Height()==height)
//	{
//		Write(1,"\tPASS: Height");
//	}else{
//		Write(2,"\tFAIL: Height = %d",url->get_Height());
//		rc=false;
//	}
//
//	//secure
//	if(url->get_IsSecure()==secure)
//	{
//		Write(1,"\tPASS: Secure");
//		if(port <0)
//		{
//			if(url->get_IsSecure())
//			{
//				if(0==strcmp(url->get_Port(),"443"))
//				{
//					Write(1,"\tPASS: Is Secure Port");
//				}else{
//					Write(2,"\tFAIL: Is Secure Port = %s <> 443",url->get_Port());
//					rc=false;
//				}
//			}else{
//				if(0==strcmp(url->get_Port(),"80"))
//				{
//					Write(1,"\tPASS: Is Secure Port");
//				}else{
//					Write(2,"\tFAIL: Is Secure Port = %s <> 80",url->get_Port());
//					rc=false;
//				}
//			}
//		}
//	}else{
//		Write(2,"\tFAIL: Secure = %s",url->get_IsSecure()?"True":"False");
//		rc=false;
//	}
//
//	//Framerate
//	if(url->get_Framerate()==framerate)
//	{
//		Write(1,"\tPASS: Frame Rate");
//	}else{
//		Write(2,"\tFAIL: Frame Rate = %d",url->get_Framerate());
//		rc=false;
//	}
//
//	//Frame Queue Size
//	if(url->get_FrameQueueSize()==frameqsize)
//	{
//		Write(1,"\tPASS: FrameQueueSize");
//	}else{
//		Write(2,"\tFAIL: FrameQueueSize = %d",url->get_FrameQueueSize());
//		rc=false;
//	}
//	//Log Level
//	if(g_logLevel==loglevel)
//	{
//		Write(1,"\tPASS: Log Level");
//	}else{
//		Write(2,"\tFAIL: Log Level = %d",g_logLevel);
//		rc=false;
//	}
//
//	//Check the RTSP value
//	if(0==strcmp(token,"") )
//	{
//		if(0 ==strcmp(sessionID,""))
//		{
//			if(!url->hasUrl() && 0==strcmp(url->get_RtspUrl(),""))
//			{
//				//ok now set an address as see if we pass
//				url->set_Token("device_host","my_fake_token");
//				std::string rtsp="rtsp://device_host/" + std::string(camera) + "?token=my_fake_token";
//				if(url->hasUrl() && 0==strcmp(url->get_RtspUrl(),rtsp.c_str()))
//				{
//					Write(1,"\tPASS: RTSP Address");
//				}else{
//					Write(2,"\tFAIL: RTSP Address Token = %s",url->get_RtspUrl());
//				}
//			}else{
//				Write(2,"\tFAIL: RTSP Address URL = %s",url->get_RtspUrl());
//				rc=false;
//			}
//		}
//	}else{
//		std::string rtsp="rtsp://"+std::string(server)+"/" + std::string(camera) + "?token="+std::string(token);
//		if(url->hasUrl() && 0==strcmp(url->get_RtspUrl(),rtsp.c_str()))
//		{
//			Write(1,"\tPASS: RTSP Address");
//		}else{
//			Write(2,"\tFAIL: RTSP Address = %s",url->get_RtspUrl());
//			rc=false;
//		}
//	}
//
//		//Check the SessionID value
//	if(0!=strcmp(sessionID,""))
//	{
//		//rtsp://10.0.0.21/StreamingSetting?version=1.0&sessionID=76535169&action=getRTSPStream&ChannelID=1&ChannelName=Channel1
//		std::string rtsp="rtsp://"+std::string(server)+"/StreamingSetting?version=1.0&sessionID="+std::string(sessionID)+"&action=getRTSPStream&ChannelID=1&ChannelName=Channel1";
//		if(url->hasUrl() && 0==strcmp(url->get_RtspUrl(),rtsp.c_str()))
//		{
//			Write(1,"\tPASS: RTSP Address Session ID");
//		}else{
//			Write(2,"\tFAIL: RTSP Address Session ID = %s",url->get_RtspUrl());
//			rc=false;
//		}
//	}
//
//	if(0==strcmp(url->get_Scheme(),"ciscosource"))
//	{
//		Write(1,"\tPASS: Log Level");
//	}else{
//		Write(2,"\tFAIL: Log Level = %d",g_logLevel);
//		rc=false;
//	}
//	return rc;
//}
void writeTestHeader(char * header)
{
	Write(0,"--------------------------------------");
	Write(0,header);
	Write(0,"--------------------------------------");
}
//bool TransportTest1()
//{
//	LPCTSTR url=L"ciscosource://ec2-54-219-61-94.us-west-1.compute.amazonaws.com?camera=98620a82-327f-4094-8537-385b21eaee04&User=admin&Password=Admin123&Width=720&Height=480";
//	const char *  server="ec2-54-219-61-94.us-west-1.compute.amazonaws.com";
//	const char * port="443";
//	const char * camera="98620a82-327f-4094-8537-385b21eaee04";
//	const char * domain="";
//	const char * user="admin";
//	const char * password="Admin123";
//	int width=720;
//	int height=480;
//	bool secure =true;
//	int framerate=12;
//	int loglevel=0; 
//	int frameqsize=200;
//	const char * token="";
//	const char * sessionID="";
//
//	writeTestHeader("Transport Test #1 - Typical");
//	return TransportTest(url, server, port,camera, domain,user,password,width, height,secure,framerate,loglevel,frameqsize,token,sessionID);
//}
//bool TransportTest2()
//{
//	LPCTSTR url=L"ciscosource://ec2-54-219-61-94.us-west-1.compute.amazonaws.com:443?camera=98620a82-327f-4094-8537-385b21eaee04&Domain=HorthSystems&User=David&Password=Horth&Width=720&Height=480&Secure=True&Framerate=30&Loglevel=5&FrameQueueSize=100";
//	const char *  server="ec2-54-219-61-94.us-west-1.compute.amazonaws.com";
//	const char * port="443";
//	const char * camera="98620a82-327f-4094-8537-385b21eaee04";
//	const char * domain="HorthSystems";
//	const char * user="David";
//	const char * password="Horth";
//	int width=720;
//	int height=480;
//	bool secure =true;
//	int framerate=30;
//	int loglevel=5; 
//	int frameqsize=100;
//	const char * token="";
//	const char * sessionID="";
//
//	writeTestHeader("Transport Test #2 - Full");
//	return TransportTest(url, server, port,camera, domain,user,password,width, height,secure,framerate,loglevel,frameqsize,token,sessionID);
//}
//bool TransportTest3()
//{
//	LPCTSTR url=L"ciscosource://ec2-54-219-61-94.us-west-1.compute.amazonaws.com:?camera=98620a82-327f-4094-8537-385b21eaee04&Token=98620a82-327f-4094-8537-385b21eaee04^LVEAMO^50^0^0^1382283819^e57cd1b438d6dba6356b5b0952c21f2bf56c5d27&Width=720&Height=480";
//	const char *  server="ec2-54-219-61-94.us-west-1.compute.amazonaws.com";
//	const char * port="443";
//	const char * camera="98620a82-327f-4094-8537-385b21eaee04";
//	const char * domain="";
//	const char * user="";
//	const char * password="";
//	int width=720;
//	int height=480;
//	bool secure =true;
//	int framerate=12;
//	int loglevel=0; 
//	int frameqsize=200;
//	const char * token="98620a82-327f-4094-8537-385b21eaee04^LVEAMO^50^0^0^1382283819^e57cd1b438d6dba6356b5b0952c21f2bf56c5d27";
//	const char * sessionID="";
//
//	writeTestHeader("Transport Test #3 - User supplied token");
//	return TransportTest(url, server, port,camera, domain,user,password,width, height,secure,framerate,loglevel,frameqsize,token, sessionID);
//}
//bool TransportTest4()
//{
//	LPCTSTR url=L"ciscosource://10.0.0.22?SessionId=70762529&Width=1920&Height=1024";
//	const char *  server="10.0.0.22";
//	const char * port="443";
//	const char * camera="";
//	const char * domain="";
//	const char * user="";
//	const char * password="";
//	int width=1920;
//	int height=1024;
//	bool secure =true;
//	int framerate=12;
//	int loglevel=0; 
//	int frameqsize=200;
//	const char * token="";
//	const char * sessionID="70762529";
//
//	writeTestHeader("Transport Test #4 - Direct Camera Access");
//	return TransportTest(url, server, port,camera, domain,user,password,width, height,secure,framerate,loglevel,frameqsize,token, sessionID);
//}

bool MediaQueueTest1()
{
	bool rc=true;
	writeTestHeader("Media Queue Test #1 - Positive");
#define QSize 200
	CMediaQueue *q=new CMediaQueue(QSize);
	if(q->get_Size()==QSize)
	{
		Write(1,"PASS: Queue size ok");
	}else{
		Write(2,"FAIL:  Queue size is wrong");
		rc=false;
	}

	if(q->get_Count()==0)
	{
		Write(1,"PASS: Queue count ok");
	}else{
		Write(2,"FAIL:  Queue count is wrong");
		rc=false;
	}

	if(q->get_isEmpty())
	{
		Write(1,"PASS: Queue isempty ok");
	}else{
		Write(2,"FAIL:  Queue isempty is wrong");
		rc=false;
	}

	//add 50 items to the q
	for(int i=0;i<50;i++)
	{
		q->put(new FrameInfo());
	}

	if(q->get_Count()==50)
	{
		Write(1,"PASS: Queue count ok");
	}else{
		Write(2,"FAIL:  Queue count is wrong");
		rc=false;
	}

	if(!q->get_isEmpty())
	{
		Write(1,"PASS: Queue isempty ok");
	}else{
		Write(2,"FAIL:  Queue isempty is wrong");
		rc=false;
	}

	//now get all 50
	for(int i=0;i<50;i++)
	{
		FrameInfo* fi=q->get();
		if(fi==NULL)
		{
			Write(2,"FAIL:  Retrieving frame %d",i);
		}
	}

	if(q->get_Count()==0)
	{
		Write(1,"PASS: Queue count ok");
	}else{
		Write(2,"FAIL:  Queue count is wrong");
		rc=false;
	}

	if(q->get_isEmpty())
	{
		Write(1,"PASS: Queue isempty ok");
	}else{
		Write(2,"FAIL:  Queue isempty is wrong");
		rc=false;
	}
	return rc;
}
bool MediaQueueTest2()
{
	bool rc=true;
	writeTestHeader("Media Queue Test #1 - Negative");
#define QSize 100
	CMediaQueue *q=new CMediaQueue(QSize);
	if(q->get_Size()==QSize)
	{
		Write(1,"PASS: Queue size ok");
	}else{
		Write(2,"FAIL:  Queue size is wrong");
		rc=false;
	}

	//add 50 items to the q
	for(int i=0;i<10;i++)
	{
		q->put(new FrameInfo());
	}

	//now try to get 11
	for(int i=0;i<10;i++)
	{
		FrameInfo* fi=q->get();
		if(fi==NULL)
		{
			Write(2,"FAIL:  Retrieving frame %d",i);
		}
	}

	//no get frame 11
	int startTime, endTime, totalTime;
	startTime = time(NULL);
	FrameInfo* fi=q->get();
	endTime = time(NULL);
	if(fi==NULL)
	{
		Write(1,"PASS:  Retrieving frame 11 is NULL");
	}else{
		Write(2,"FAIL:  Queue should be empty");
	}

	totalTime = endTime - startTime;
	if(totalTime>1&&totalTime<3)
	{
		Write(1,"PASS:  Get empty frame timing ok");
	}else{
		Write(2,"FAIL:  Bad timing for empty frame total time=%d",totalTime);
	}

	if(q->get_Count()==0)
	{
		Write(1,"PASS: Queue count ok");
	}else{
		Write(2,"FAIL:  Queue count is wrong");
		rc=false;
	}
	return rc;
}

//use this address for our test (MUST BE CHANGED each test)
#define filename "rtsp://10.0.0.22/StreamingSetting?version=1.0&sessionID=95652231&action=getRTSPStream&ChannelID=1&ChannelName=Channel1"

bool VideoDecoder1()
{
	bool rc=true;
	writeTestHeader("Video Decoder Test #1 - Positive");
	return rc;
}

bool RtspTest1()
{
	writeTestHeader("RTSP Test #1 - open stream");
	CstreamMedia * _streamMedia=new CstreamMedia();
	int ret = _streamMedia->rtspClientOpenStream((const char *)filename);
	if(ret!=0)
	{
		Write(2,"FAIL: Open Stream ret=%d",ret);
	}
	Write(1,"PASS: Open Stream ret=%d",ret);
	_streamMedia->rtspClientCloseStream();
	return ret==0;
}
bool RtspTest2()
{
	writeTestHeader("RTSP Test #2 - Get Media Info");
	MediaInfo videoMediaInfo;

	CstreamMedia * _streamMedia=new CstreamMedia();
	int ret = _streamMedia->rtspClientOpenStream((const char *)filename);
	if(ret!=0)
	{
		Write(2,"FAIL: Open Stream ret=%d",ret);
		return false;
	}

	Write(1,"PASS: Open Stream ret=%d",ret);
	ret=_streamMedia->rtspClinetGetMediaInfo(CODEC_TYPE_VIDEO, videoMediaInfo);
	bool rc=(0==strcmp("H264",videoMediaInfo.codecName));
	if(ret!=0 || !rc)
	{
		Write(2,"FAIL: Get Media Info ret=%d",ret);
		return false;
	}
	Write(1,"PASS: Get Media Info");

	_streamMedia->rtspClientCloseStream();
	return ret==0;
}
bool RtspTest3()
{
	bool rc=true;
	writeTestHeader("RTSP Test #2 - Play Stream");
	MediaInfo videoMediaInfo;

	CstreamMedia * _streamMedia=new CstreamMedia();
	int ret = _streamMedia->rtspClientOpenStream((const char *)filename);
	if(ret!=0)
	{
		Write(2,"FAIL: Open Stream ret=%d",ret);
	}
	Write(1,"PASS: Open Stream ret=%d",ret);
	ret=_streamMedia->rtspClientPlayStream(filename);
	if(ret==0)
	{
		Write(1,"PASS: Play Stream ret=%d",ret);
	}else{
		Write(2,"FAIL: Play Stream ret=%d",ret);
		rc=false;
	}

	//do something here while we wait for frames to arrive
	int loop = 100;
	while(loop>0)
	{
		::Sleep(250);
		loop--;
	}
	int c=_streamMedia->GetQueueSize();
	if(c==0)
	{
		Write(2,"FAIL: We should have gotten at least one frame by now");
		rc=false;
	}else{
		Write(1,"PASS: We have frames=%d",c);
	}
	_streamMedia->rtspClientCloseStream();
	return rc;
}

#define logfile "C:\\ProgramData\\CiscoFilter\\Logs\\CiscoFilterLog.log"
bool LogFileTest1()
{
	bool rc=true;
	writeTestHeader("LogFileTest #1 - Log file test");

	//delete old log file
	if(FileExists(logfile))
		std::remove(logfile);

	if(0==countLines(logfile))
	{
		Write(1,"PASS: LogFileTest");
	}else{
		Write(2, "File does not exist");
	}

	g_logLevel=0;
	TRACE_CRITICAL("test");
	
	g_logLevel=1;
	TRACE_CRITICAL("test");

	return rc;
}

bool EnvTest1()
{
	writeTestHeader("CiscoUsageEnvironment Test #1 - write to trace helper");
	BasicTaskScheduler* scheduler =  BasicTaskScheduler::createNew();
	if (scheduler ==	NULL)
	{
		Write(2, "BasicTaskScheduler fail");
		return false;
	}
	CMyUsageEnvironment* env = CMyUsageEnvironment::createNew(*scheduler);
	if (env == NULL)
	{
		Write(2, "BasicUsageEnvironment fail");
		return false;
	}
	*env << "test";
	if(0 < env->get_size())
	{
		Write(1,"PASS: MyUsageEnvironment");
	}else{
		Write(2, "stream should not be empty");
	}
	*env << 5 << 5.3 ;
	if(0 < env->get_size())
	{
		Write(1,"PASS: MyUsageEnvironment");
	}else{
		Write(2, "stream should be empty");
	}
	*env << "\n";
	if(0==env->get_size())
	{
		Write(1,"PASS: MyUsageEnvironment");
	}else{
		Write(2, "stream should be empty");
	}

	return true;
}


int _tmain(int argc, _TCHAR* argv[])
{
	
	logFile << "Writing this to a file.\n";
	logFile.open ("UnitTest.log");
	Write(0,"Starting unit tests");
	//Write(0,"\n\nTransport URL Tests");
	//bool tt1=TransportTest1();
	//bool tt2=TransportTest2();
	//bool tt3=TransportTest3();
	//bool tt4=TransportTest4();

	//these depend on a valid url
	Write(0,"\n\nMedia Queue Tests");
	bool mq1=MediaQueueTest1();
	bool mq2=MediaQueueTest2();

	Write(0,"\n\Log file test");
	bool lft1=LogFileTest1();

	Write(0,"\n\nMyUsageEnvirnoment Test");
	bool cue1=EnvTest1();

	Write(0,"\n\nVideo Decoder Tests");
	bool vc1=VideoDecoder1();

	//these depend on a valid url
	Write(0,"\n\nRTSP Tests");
	bool rtsp1=RtspTest1();
	if(!rtsp1)
	{
		Write(2,"Open failed bailing out of all RTSP tests");
		goto wrapup;
	}
	bool rtsp2=RtspTest2();
	bool rtsp3=RtspTest3();


wrapup:
	logFile.close();

	Write(0,"\n\n");
	if(rtsp1&& rtsp2&&rtsp3)
	{
		Write(1,"<+++++++++++++++++++++++++++++++++++++++>");
		Write(1,"<            All Test Pasesed!          >");
		Write(1,"<+++++++++++++++++++++++++++++++++++++++>");
	}else{
		Write(2,"|---------------------------------------|");
		Write(2,"|          Unit tests failed            |");
		Write(2,"|---------------------------------------|");
	}

	Write(0,"\n\n");
	std::cout << "Press Enter to exit" <<std::endl;
	std::cin.get();
	return 0;
}


