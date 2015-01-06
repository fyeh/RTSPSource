#pragma once
#include "stdafx.h"

#define LOGLEVEL_OFF 0
#define LOGLEVEL_CRITICAL 1
#define LOGLEVEL_ERROR 2
#define LOGLEVEL_WARN 3
#define LOGLEVEL_INFO 4
#define LOGLEVEL_VERBOSE 5
#define LOGLEVEL_DEBUG 6

#define TRACE_CRITICAL(x,...) Log(LOGLEVEL_CRITICAL, __FUNCTION__, x, ##__VA_ARGS__) 
#define TRACE_ERROR(x,...) Log(LOGLEVEL_ERROR, __FUNCTION__, x, ##__VA_ARGS__) 
#define TRACE_WARN(x,...) Log(LOGLEVEL_WARN, __FUNCTION__, x, ##__VA_ARGS__) 
#define TRACE_INFO(x,...) Log(LOGLEVEL_INFO, __FUNCTION__, x, ##__VA_ARGS__) 
#define TRACE_VERBOSE(x,...) Log(LOGLEVEL_VERBOSE, __FUNCTION__, x, ##__VA_ARGS__) 
#define TRACE_DEBUG(x,...) Log(LOGLEVEL_DEBUG, __FUNCTION__, x, ##__VA_ARGS__) 

extern int g_logLevel;

extern "C"
{
	 typedef void (logHandler)(int level, const char* msg);
	_declspec(dllexport) void InitializeTraceHelper(int level,logHandler* callback);
	_declspec(dllexport) void Log(int level, const char * functionName, const char * lpszFormat, ...);
}
