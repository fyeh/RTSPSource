#include "stdafx.h"
#include "MyUsageEnvironment.h"
#include "trace.h"

CMyUsageEnvironment*
CMyUsageEnvironment::createNew(TaskScheduler& taskScheduler) {
  return new CMyUsageEnvironment(taskScheduler);
}



CMyUsageEnvironment::CMyUsageEnvironment(TaskScheduler& taskScheduler) : BasicUsageEnvironment(taskScheduler)
{
}


CMyUsageEnvironment::~CMyUsageEnvironment(void)
{
}

UsageEnvironment& CMyUsageEnvironment::operator<<(char const* str) {
  if (str == NULL) str = "(NULL)"; // sanity check
  m_logstream << str;
  write();
  return *this;
}

UsageEnvironment& CMyUsageEnvironment::operator<<(int i) {
  m_logstream << i;
  return *this;
}

UsageEnvironment& CMyUsageEnvironment::operator<<(unsigned u) {
  m_logstream << u;
  return *this;
}

UsageEnvironment& CMyUsageEnvironment::operator<<(double d) {
  m_logstream << d;
  return *this;
}

UsageEnvironment& CMyUsageEnvironment::operator<<(void* p) {
  m_logstream << p;
  return *this;
}

void  CMyUsageEnvironment::write(){
	int len = get_size();
    if(m_logstream.str ()[len-1] == '\n')
	{
		std::string str(m_logstream.str());
		TRACE_DEBUG(str.c_str());
		m_logstream.str(std::string());
	}
}