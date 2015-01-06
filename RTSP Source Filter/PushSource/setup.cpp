
#include "stdafx.h"
#pragma unmanaged
#include "PushGuids.h"
#include "PushSource.h"

/**
Register are filter as a directshow filter
*/
void RegisterFilterRTSP()
{
	 HKEY   hkey;
	 DWORD  dwDisposition; 
	 if(RegCreateKeyEx(HKEY_CLASSES_ROOT, 
					   TEXT("RTSPSource"), 
					   0, 
					   NULL, 
					   REG_OPTION_NON_VOLATILE, 
					   KEY_ALL_ACCESS, 
					   NULL, 
					   &hkey, 
					   &dwDisposition) == ERROR_SUCCESS)
	 {
		  LPCTSTR value = TEXT("Source Filter");
		  LPCTSTR clsid = TEXT(FILTER_GUID);
		  
		  int result = RegSetValueEx(hkey, value, 0, REG_SZ, (BYTE*)clsid, 76);//wstrlen(clsid) + 1);
		  RegCloseKey(hkey);
	 }

	 if(RegCreateKeyEx(HKEY_CURRENT_USER, 
					   TEXT("Software\\Microsoft\\MediaPlayer\\Player\\Schemes\\RTSPSource"), 
					   0, 
					   NULL, 
					   REG_OPTION_NON_VOLATILE, 
					   KEY_ALL_ACCESS, 
					   NULL, 
					   &hkey, 
					   &dwDisposition) == ERROR_SUCCESS)
	 {
		  LPCTSTR value = TEXT("Runtime");
		  DWORD clsid = 1;
		  int result = RegSetValueEx(hkey, value, 0, REG_DWORD, (BYTE*)&clsid, sizeof(DWORD));
		  
		  RegCloseKey(hkey);
	 }
}

/**
Simple helper function
*/
void RegisterFilter()
{
	RegisterFilterRTSP();
}


// Note: It is better to register no media types than to register a partial 
// media type (subtype == GUID_NULL) because that can slow down intelligent connect 
// for everyone else.

// For a specialized source filter like this, it is best to leave out the 
// AMOVIESETUP_FILTER altogether, so that the filter is not available for 
// intelligent connect. Instead, use the CLSID to create the filter or just 
// use 'new' in your application.


// Filter setup data
const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{
    &MEDIATYPE_Video,       // Major type
    &MEDIASUBTYPE_NULL      // Minor type
};

const AMOVIESETUP_PIN sudOutputPinRTSP = 
{
    L"Output",      // Obsolete, not used.
    FALSE,          // Is this pin rendered?
    TRUE,           // Is it an output pin?
    FALSE,          // Can the filter create zero instances?
    FALSE,          // Does the filter create multiple instances?
    &CLSID_NULL,    // Obsolete.
    NULL,           // Obsolete.
    1,              // Number of media types.
    &sudOpPinTypes  // Pointer to media types.
};

const AMOVIESETUP_FILTER sudPushSourceRTSP =
{
    &CLSID_PushSourceRTSP,// Filter CLSID
    g_wszPushRTSP,       // String name
    MERIT_DO_NOT_USE,       // Filter merit
    1,                      // Number pins
    &sudOutputPinRTSP    // Pin details
};

// List of class IDs and creator functions for the class factory. This
// provides the link between the OLE entry point in the DLL and an object
// being created. The class factory will call the static CreateInstance.
// We provide a set of filters in this one DLL.

CFactoryTemplate g_Templates[1] = 
{
    { 
      g_wszPushRTSP,               // Name
      &CLSID_PushSourceRTSP,       // CLSID
      CPushSourceRTSP::CreateInstance, // Method to create an instance of MyComponent
      NULL,                           // Initialization function
      &sudPushSourceRTSP           // Set-up information (for filters)
    },
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);    



////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////

STDAPI DllRegisterServer()
{
	RegisterFilter(); 
	HRESULT hr=AMovieDllRegisterServer2( TRUE );
    return hr;
}

STDAPI DllUnregisterServer()
{
	HRESULT hr=AMovieDllRegisterServer2( FALSE );
    return hr;
}

//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  dwReason, 
                      LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}

