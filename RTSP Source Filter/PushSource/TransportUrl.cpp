
#include "stdafx.h"
#include "TransportUrl.h"
#include "Settings.h"
#include "Logger.h"


/**
Constructor - Decoder
*/
TransportUrl::TransportUrl(LPCTSTR url)
{
	USES_CONVERSION;
	m_url=T2A(url);

	g_logLevel=Settings::DefaultLogLevel();
	m_framerate=Settings::DefaultFramerate();
	m_frameQueueSize=Settings::DefaultFrameQueueSize();
	m_lostFrameCount=Settings::LostFrameCount();

	//log it
	TRACE_INFO( "Parsing: %s", m_url);


	//http://10.0.0.22/viewvideo.cs?version=1.0&sessionID=70762529&action=get
	//m_url =	"rtsp://ec2-54-219-61-94.us-west-1.compute.amazonaws.com/98620a82-327f-4094-8537-385b21eaee04?token=98620a82-327f-4094-8537-385b21eaee04^LVEAMO^50^0^0^1382283819^e57cd1b438d6dba6356b5b0952c21f2bf56c5d27";
	//std::transform(m_url.begin(), m_url.end(), m_url.begin(), std::tolower);

	m_url = std::tr1::regex_replace(m_url, std::tr1::regex("\\\\"), std::string("/"));

	std::tr1::cmatch m1;
	std::tr1::regex_match(m_url.c_str(), m1, std::tr1::regex("(.*)://(.*):?(.*)?\\?(.*)?\?(.*)"));
	if(0>=(int)m1.length())
	{
		TRACE_ERROR( "Unable to parse URL, format is invalid: %s", m_url.c_str());
		throw false;
	}

	//scheme this must equal rtspsource
    m_scheme = m1[1];
	std::transform(m_scheme.begin(), m_scheme.end(), m_scheme.begin(), std::tolower);

	if(m_scheme != "rtspsource")
	{
		TRACE_ERROR( "Invalid source: %s  != rtspsource",m_scheme.c_str());
		throw false;
	}

	m_path = m1[2];

	//Get the query string
	std::string queryString = m1[5];

	m_rtspUrl = "rtsp://" + m_path + "?" + queryString;

	m_width=getInt(queryString, "width");
	if(m_width<=0)
	{
		TRACE_ERROR( "Width is required");
		//throw false;
	}

	m_height=getInt(queryString, "height");
	if(m_height<=0)
	{
		TRACE_ERROR( "Height is required");
		//throw false;
	}

	//optional parameters
	//-----------------------------------------

	//framerate
	int framerate=getInt(queryString, "framerate");
	if(framerate>0)
		m_framerate=framerate;

	if(m_framerate>Settings::MaxFramerate())
	{
		int old=m_framerate;
		m_framerate=Settings::MaxFramerate();
		TRACE_ERROR( "Framerate unsupported (%d), using %d",old,m_framerate);
	}

	//Log level
	int loglevel=getInt(queryString, "loglevel");
	if(loglevel>=LOGLEVEL_OFF && loglevel <= LOGLEVEL_DEBUG)
	{
		g_logLevel=loglevel;
		InitializeTraceHelper(g_logLevel,Write);
	}	else	{
		TRACE_ERROR( "Log Level value unsupported (%d) min=0:Off max=7:Debug, using %d",loglevel,g_logLevel);
	}

	//framequeuesize
	int frameqsize=getInt(queryString, "framequeuesize");
	if(frameqsize >= 1 && frameqsize < 10000){
		m_frameQueueSize=frameqsize;
	}else{
		TRACE_ERROR( "Frame Queue size value unsupported (%d) min=1 max=10,000. Using %d", frameqsize, m_frameQueueSize);
	}
	
	//lostframecount
	int lostFrameCount=getInt(queryString, "lostFrameCount");
	if(lostFrameCount >= 1 && lostFrameCount < 10000){
		m_lostFrameCount=lostFrameCount;
	}else{
		TRACE_ERROR( "Lost frame count value unsupported (%d) min=1 max=10,000. Using %d", lostFrameCount, m_lostFrameCount);
	}

}

/**
Get a int value from the query string.
Parses the query string from the usrl looking for the specified field name
then returns the value as an integer

\param queury The query string from the url
\param fieldName The name of the field in the query string
\param is the fieldName case sensitive
*/
int TransportUrl::getInt(std::string query, LPCSTR fieldName, bool ignoreCase)
{
	std::string val;
	getValue(query, fieldName, val, ignoreCase);
	int nVal=atoi(val.c_str());
	return nVal;
}

/**
Get a string value from the query string.
Parses the query string from the usrl looking for the specified field name
then returns the value as an std::string 

\param queury The query string from the url
\param fieldName The name of the field in the query string
\param is the fieldName case sensitive
*/
void TransportUrl::getValue(std::string query, LPCSTR fieldName, std::string& returnValue, bool ignoreCase)
{
	//if we are ignoring the case convert everything to lower case
	if(ignoreCase)
		std::transform(query.begin(), query.end(), query.begin(), std::tolower);

	returnValue = "";
	std::string field=fieldName;

	std::tr1::cmatch m1;
	std::tr1::regex_match(query.c_str(), m1, std::tr1::regex("(.*)"+field + "=(.*)"));
	if(m1.length()<=1)
		return;

	std::string queryValue = m1[2];
	if(queryValue.find("&")!=std::string::npos)
		queryValue=queryValue.substr(0, queryValue.find("&"));

   std::string::size_type pos = queryValue.find_last_not_of(' ');
	if(pos != std::string::npos) 
	{
		queryValue.erase(pos + 1);
		pos = queryValue.find_first_not_of(' ');
		if(pos != std::string::npos) 
			queryValue.erase(0, pos);
	}  
	else 
	{
		queryValue.erase(queryValue.begin(), queryValue.end());
	}
	TRACE_DEBUG("%s = %s",fieldName, queryValue.c_str());
	returnValue = queryValue.c_str();
	return;
}

/**
Check for integer.
Check to see if the string is an int
/param value string to check
*/
bool TransportUrl::IsInt(std::string value)
{
	USES_CONVERSION;
	bool rc=false;
	try
	{
		std::istringstream iss( value );
 		double dTestSink;
		iss >> dTestSink;

		// was any input successfully consumed/converted?
		if ( ! iss )
			return false;
	 
		// was all the input successfully consumed/converted?
		rc= iss.rdbuf()->in_avail() == 0;
	}
	catch(...)
	{
	}
	return rc;
}

