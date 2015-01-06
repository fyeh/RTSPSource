
#include "stdafx.h"
#include "Logger.h"
#include "PushSource.h"
#include "PushGuids.h"
#pragma managed


#pragma warning(disable:4710)  // 'function': function not inlined (optimzation)
using namespace std;


/**
* Setup the direct show filter name PushSourceRTSP
*/
CPushSourceRTSP::CPushSourceRTSP(IUnknown *pUnk, HRESULT *phr)
           : CSource("RTSP Filter", pUnk, CLSID_PushSourceRTSP)
{
	System::AppDomain::CurrentDomain->AssemblyResolve += gcnew System::ResolveEventHandler( &CPushSourceRTSP::MyResolveEventHandler);
	InitializeTraceHelper(0,Write);
	TRACE_INFO( "Starting Filter");

	//m_callBackInstance = NULL;
    // The pin magically adds itself to our pin array.
    m_pPin = new CPushPinRTSP(phr, this);

	if (phr)
	{
		if (m_pPin == NULL)
		{
			*phr = E_OUTOFMEMORY;
			 TRACE_ERROR( "Could not create pin.  HRESULT =  %#x", *phr);

		}else{
			*phr = S_OK;
			 TRACE_INFO( "Pin created successfully.");
		}
	}  

}

/**
* Destructor
* \see Close
*/
CPushSourceRTSP::~CPushSourceRTSP()
{
	HRESULT hr=S_OK;

	TRACE_INFO( "Source Filter Desctructor.  Pin count= %d",this->GetPinCount());

	try
	{
		if(m_pPin !=NULL)
		{
			try
			{
				hr=m_pPin->Disconnect();
				TRACE_INFO( "Pin disconnect.  HRESULT =  %#x", hr);
			}
			catch (...)
			{
				TRACE_INFO("Catch");
			}
			hr=m_pPin->Inactive();		
			TRACE_INFO( "Pin Inactive.  HRESULT =  %#x", hr);

			hr=this->RemovePin(m_pPin);
			TRACE_INFO( "Pin remove.  HRESULT =  %#x", hr);


			delete m_pPin;
		}
		m_pPin=NULL;



		TRACE_INFO( "Releasing URL");
		if(m_transportUrl !=NULL)
			delete m_transportUrl;
		m_transportUrl=NULL;

		Stop();
		TRACE_INFO( "Destruction complete for %s", m_currentFile.c_str() );
	}
	catch (...)
	{
		TRACE_INFO("Exception in Source Filter Destructor");
	}
}

/**
* Instantiate the com interface
*/
CUnknown * WINAPI CPushSourceRTSP::CreateInstance(IUnknown *pUnk, HRESULT *phr)
{
	CPushSourceRTSP *pNewFilter = new CPushSourceRTSP(pUnk, phr );

	if (phr)
	{
		if (pNewFilter == NULL) 
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}
    return pNewFilter;

}

/**
* Direct Show override
*/
STDMETHODIMP CPushSourceRTSP::GetCurFile(LPOLESTR * ppszFileName, AM_MEDIA_TYPE *pmt)
{
	USES_CONVERSION;
    CheckPointer(ppszFileName, E_POINTER);
    *ppszFileName = NULL;

	LPWSTR name=L"RTSPFilter";

	if (m_currentFile.length()>0) 
    {
        *ppszFileName = (LPOLESTR)
        QzTaskMemAlloc(sizeof(WCHAR) * (1+lstrlenW(name)));

        if (*ppszFileName != NULL) 
        {
            lstrcpyW(*ppszFileName,name);
        }
    }

    if(pmt) 
    {
        ZeroMemory(pmt, sizeof(*pmt));
        pmt->majortype = MEDIATYPE_NULL;
        pmt->subtype = MEDIASUBTYPE_NULL;
    }

	TRACE_DEBUG("GetCurFile");
    return S_OK;
}

/**
* What interfaces are we interested in
*/
STDMETHODIMP CPushSourceRTSP::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	HRESULT hr=S_OK;
	try
	{
		TRACE_DEBUG( "NonDelegatingQueryInterface");
		CheckPointer(ppv,E_POINTER);

		if (riid == IID_IFileSourceFilter) 
		{
			TRACE_DEBUG( "NonDelegatingQueryInterface - IID_IFileSourceFilter");
			hr= GetInterface((IFileSourceFilter *)this, ppv);
		} 
		else 
		{
			char msg[255];
			sprintf(msg, "REFIID=%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",  
					riid.Data1, riid.Data2, riid.Data3,
					riid.Data4[0], riid.Data4[1], 
					riid.Data4[2], riid.Data4[3],
					riid.Data4[4], riid.Data4[5], 
					riid.Data4[6], riid.Data4[7] ); 
			TRACE_DEBUG( msg);
			hr= CSource::NonDelegatingQueryInterface(riid, ppv);
		}
	}
	catch(...)
	{
		TRACE_ERROR( "--Exception---------------------");
		TRACE_ERROR( "NonDelegatingQueryInterface. ");
		TRACE_ERROR( "--------------------------------");
		hr=E_FAIL;
	}

	//not really an error, we just don;t support that interface
	if(hr!=S_OK)
		TRACE_DEBUG( "NonDelegatingQueryInterface.  HRESULT =  %#x", hr);
	return hr;
}
STDMETHODIMP_(ULONG) CPushSourceRTSP::NonDelegatingAddRef()
{
	ULONG ret=CSource::NonDelegatingAddRef();
	TRACE_DEBUG( "NonDelegatingAddRef.  RET =  %#x", ret);
	return ret;
}
STDMETHODIMP_(ULONG) CPushSourceRTSP::NonDelegatingRelease()
{
	ULONG ret=CSource::NonDelegatingRelease();
	return ret;	
}

/**
* This is the initial load call. 
* When the media element source property is set the uri used to load our
* filter and then passed in as the file name. We then use that file name
* to setup our working environment

/see TransportUrl
*
* This is the first stop to check when you don't see any video.  If the
* media element source is set properly this should be called.  If not
* either the source is wrong, it must be in the format <b>RTSPSource://ip?queurystring</b>
* or the filter is not properly registered with the system, 
* try <b>regsvr32 RTSPsource.ax</b>
*/
STDMETHODIMP CPushSourceRTSP::Load(LPCOLESTR lpwszFileName, const AM_MEDIA_TYPE *pmt)
{
	USES_CONVERSION;
	try
	{
		//&action=getRTSPStream&ChannelID=1&ChannelName=Channel1
		m_transportUrl=new TransportUrl(lpwszFileName);

		//checks for the help message.
		m_currentFile=W2A(lpwszFileName);
		TRACE_INFO( "Load.  URL = %s", m_currentFile.c_str());
	}
	catch(...)
	{
		return VFW_E_INVALID_FILE_FORMAT;
	}

	return S_OK;
}

/**
* We have been requested to stop video, just tell the base
* which will kill the pin for us.
*/
STDMETHODIMP CPushSourceRTSP::Stop(void)
{
	HRESULT hr=S_OK;
	hr=CSource::Stop();
	TRACE_INFO( "Stop.  HRESULT =  %#x", hr);

	return hr;
}

/**
Because the application using our filter can be anywhere on the machine
the filter will need some help finding our supporting dlls.  The resolve handler
gets called everytime the filter tries to load a dll it needs but can
not find.  Since will know the installer puts all of our dlls in the 
same directory as the filter, we get our directory and use its path
to load the dll
*/
#pragma managed(push, on)
System::Reflection::Assembly^ CPushSourceRTSP::MyResolveEventHandler(System::Object^ sender, System::ResolveEventArgs^ args)
{
	TCHAR  cCurrentPath[FILENAME_MAX];
	GetModuleFileName( GetModuleHandle(TEXT("RTSPSource.ax") ), cCurrentPath, FILENAME_MAX );
	System::String^ dir = gcnew System::String(cCurrentPath);
	dir=dir->Substring(0,dir->LastIndexOf("\\"));
	System::IO::Directory::SetCurrentDirectory(dir);

		
	if(args->Name->ToLower()->StartsWith(gcnew System::String("log4net")))
		return System::Reflection::Assembly::LoadFile(dir+"\\log4net.DLL");
	
	if(args->Name->ToLower()->StartsWith(gcnew System::String("HelpLib")))
		return System::Reflection::Assembly::LoadFile(dir+"\\HelpLib.DLL");

	TRACE_INFO(  "MyResolveEventHandler %s",args->Name);
	TRACE_ERROR( "Unable to find and load RTSP support dlls");
	return nullptr;
}
#pragma managed(pop)
