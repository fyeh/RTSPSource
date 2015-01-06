#pragma once
#include "stdafx.h"
#include <atlstr.h>
#include "trace.h"


extern "C"
{
	int g_logLevel=-1;
	logHandler* loggingCallback=NULL;
	/**
	Native interface to managed logger
	*/
	_declspec(dllexport) void Write(int level, const char * msg)
	{
		if(loggingCallback!=NULL)
			loggingCallback(level, msg);
	}

	_declspec(dllexport) void InitializeTraceHelper(int level,logHandler* callback){
		g_logLevel=level;
		loggingCallback=callback;
	}
	/**
	Creates the message
	*/
	_declspec(dllexport) void Log(int level, const char * functionName, const char * lpszFormat, ...)
	{
		if(level>g_logLevel)return;
			static char szMsg[2048];
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
			try{
				Write(level, logMsg.c_str());
			}
			catch(...)
			{
			}
	}
	
}
