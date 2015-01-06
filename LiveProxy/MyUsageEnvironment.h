#pragma once
#include "stdafx.h"
#include "basicusageenvironment.hh"
#include <sstream>

/**
Create our own usage environment class so we can capture
the log information and write it our trace log
*/
class CMyUsageEnvironment :
	public BasicUsageEnvironment
{
public:
  static CMyUsageEnvironment* createNew(TaskScheduler& taskScheduler);
public:

	CMyUsageEnvironment(TaskScheduler& taskScheduler);
	~CMyUsageEnvironment(void);

	int get_size(){return m_logstream.tellp();}

	//overloads
	virtual UsageEnvironment& operator<<(char const* str);
	virtual UsageEnvironment& operator<<(int i);
	virtual UsageEnvironment& operator<<(unsigned u);
	virtual UsageEnvironment& operator<<(double d);
	virtual UsageEnvironment& operator<<(void* p);

private:
	void write();

private:
	std::stringstream  m_logstream;
};

