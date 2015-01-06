
#pragma once
#include "stdafx.h"
#include <atlstr.h>
#include "trace.h"


/**
A managed class to help with log4net.
We support 7 levels of logging ranging in levels or verbosity
*/
ref class Logger 
{
public:

	/**
	Writes messages to log file.
	Static method that actually calls our tracehelper in the 
	helper lib.  
	*/
	static void LogMessage(int level, const char * szMsg) 
	{ 
		//check our logging level to make sure we are interested
		if(g_logLevel<level) return;

			try
			{
				System::String ^ msg= gcnew System::String(szMsg);
				HelpLib::TraceHelper::LogLevel logLevel;
				switch(level)
				{
					case LOGLEVEL_DEBUG:
						logLevel=HelpLib::TraceHelper::LogLevel::Debug;
						break;
					case LOGLEVEL_VERBOSE:
						logLevel=HelpLib::TraceHelper::LogLevel::Debug;
						break;
					case LOGLEVEL_INFO:
						logLevel=HelpLib::TraceHelper::LogLevel::Info;
						break;
					case LOGLEVEL_WARN:
						logLevel=HelpLib::TraceHelper::LogLevel::Warning;
						break;
					case LOGLEVEL_ERROR:
						logLevel=HelpLib::TraceHelper::LogLevel::Error;
						break;
					case LOGLEVEL_CRITICAL:
						logLevel=HelpLib::TraceHelper::LogLevel::Error;
						break;
					default:
						return;
				}
				HelpLib::TraceHelper::Write(logLevel, msg);
			}
			catch(...)
			{
			}
	}
};

extern "C"
{
	int g_logLevel=0;
	/**
	Native interface to managed logger
	*/
	_declspec(dllexport) void Write(int level, const char * msg)
	{
			Logger::LogMessage(level, msg);
	}
	/**
	Creates the message
	*/
	_declspec(dllexport) void Log(int level, const char * functionName, const char * lpszFormat, ...)
	{
			static char szMsg[512];
			va_list argList;
			va_start(argList, lpszFormat);
			try
			{
				vsprintf(szMsg,lpszFormat, argList);
			}
			catch(...)
			{
				strcpy(szMsg ,"DebugHelper:Invalid string format!");
			}
			va_end(argList);
			std::string logMsg=static_cast<std::ostringstream*>( &(std::ostringstream() << ::GetCurrentProcessId() << "," << ::GetCurrentThreadId() << "," << functionName << ", " <<  szMsg) )->str();
			Write(level, logMsg.c_str());
	}
	
}
