
#pragma once
#include "stdafx.h"
#include "trace.h"

/**
Registry settings for filter
IMPORTANT: IF YOU CHANGE A DEFAULT UPDATE SAMPLE GRABBER!
*/
class Settings
{

public:

	/**
	If the stream is not specified in the query string then use
	this value as the stream number.
	Default = 2
	*/
	static int DefaultStream()
	{
		return GetRegistryValue(TEXT("DefaultStream"),2,1);
	}

	/**
	If no Framerate is specified in the query string then use
	this value as the framerate.
	Default = 12
	*/
	static int DefaultFramerate()
	{
		return GetRegistryValue(TEXT("DefaultFramerate"),12,1);
	}

	/**
	If no Logging level is specified in the query string then use
	this value as the LogLevel.
	Default = 0 - Errors only
	*/
	static int DefaultLogLevel()
	{
		return GetRegistryValue(TEXT("DefaultLogLevel"),LOGLEVEL_OFF,LOGLEVEL_OFF,LOGLEVEL_DEBUG);
	}

	/**
	If no frame queue is specified in the query string then use
	this value as the size of the queue.
	Default = 200
	*/
	static int DefaultFrameQueueSize()
	{
		return GetRegistryValue(TEXT("DefaultFrameQueueSize"),200,1);
	}
	/**
	The upper limit of supported framerates per the query string.
	Default = 30
	*/
	static int MaxFramerate()
	{
		return GetRegistryValue(TEXT("MaxFramerate"),30,1);
	}

	/**
	The number seconds that the filter will wait before attempting
	to reload the video pipeline if no video is found.  This value 
	is used when we detect the camera is online but we are not recieving
	video, typically this means switching from multicast to unicast
	Default = 15
	*/
	static int RetryConnectionTime()
	{
		return GetRegistryValue(TEXT("RetryConnectionTime"),15,1);
	}
	/**
	This setting is the number of seconds that the filter will wait
	before attempting to reconnect to a lost video source.  For example
	a network or power issue could interrupt video and this is the number
	of seconds we will wait until we attempt to reconnect.
	Default = 60
	*/
	static int ReloadLostVideoTime()
	{
		return GetRegistryValue(TEXT("ReloadLostVideoTime"),60,10);
	}
	/**
	The number seconds to run through while the filter stabilizes after a reload.  
	Default = 5
	*/
	static int ReloadFrameBufferSeconds()
	{
		return GetRegistryValue(TEXT("ReloadFrameBufferSeconds"),15,1,120);
	}	
	/**
	The number seconds to wait before we take action for a lost frame
	problem. Often when video is lost the same frame will be sent over 
	and over again.  This value represents how many times we get the
	same frame before we fall into the lost video buffering mode. A value
	to low could cause performance problems if the video can self recover
	a value to high will cause lengthy delays.
	Default = 10
	*/
	static int LostFrameCount()
	{
		return GetRegistryValue(TEXT("LostFrameCount"),120,1);
	}
private:
	static DWORD GetRegistryValue(LPCTSTR valueName, DWORD defaultValue, int minValue=-1, int maxValue=-1)
	{
		USES_CONVERSION;
		DWORD ret=defaultValue;
		DWORD level = 0;
		HKEY   hkey;
		DWORD  dwDisposition; 

		try
		{
			if(::RegCreateKeyEx(HKEY_CURRENT_USER, 
			TEXT("Software\\Microsoft\\MediaPlayer\\Player\\Schemes\\RTSPSource"), 
			0, 
			NULL, 
			REG_OPTION_NON_VOLATILE, 
			KEY_ALL_ACCESS, 
			NULL, 
			&hkey, 
			&dwDisposition) == ERROR_SUCCESS)
			{
				DWORD dwType=REG_DWORD;
				DWORD dwSize=sizeof(DWORD);

				int result = RegQueryValueEx(hkey, valueName,  NULL, &dwType,(LPBYTE)&ret, &dwSize);
				RegCloseKey(hkey);

				if(minValue>0 && minValue>ret)
				{
					//log4cpp::Category::getRoot().errorStream() << "Registry Setting "<<T2A(valueName) <<" = " <<ret <<" is to low, using minimum setting of "<< minValue;
					ret=minValue;
				}
			}
			//log4cpp::Category::getRoot().debugStream() << "Registry Setting "<< valueName  << " = " << ret;
		}
		catch(...)
		{
			//log4cpp::Category::getRoot().errorStream() << "Error getting registry value " << valueName;
		}
		return ret;
	}
};