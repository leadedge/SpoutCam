//////////////////////////////////////////////////////////////////////////
//  This file contains routines to register / Unregister the 
//  Directshow filter 'Spout Cam'
//  We do not use the inbuilt BaseClasses routines as we need to register as
//  a capture source
//////////////////////////////////////////////////////////////////////////

#pragma comment(lib, "kernel32")
#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")
#pragma comment(lib, "advapi32")
#pragma comment(lib, "winmm")
#pragma comment(lib, "ole32")
#pragma comment(lib, "oleaut32")

#include "cam.h"

//<==================== VS-START ====================>
#ifdef DIALOG_WITHOUT_REGISTRATION
#pragma comment(lib, "Comctl32.lib")
#endif

#include "camprops.h"
//<==================== VS-END ======================>

#include <olectl.h>
#include <initguid.h>
#include <dllsetup.h>

#define CreateComObject(clsid, iid, var) CoCreateInstance( clsid, NULL, CLSCTX_INPROC_SERVER, iid, (void **)&var);

STDAPI AMovieSetupRegisterServer( CLSID clsServer, LPCWSTR szDescription, LPCWSTR szFileName, LPCWSTR szThreadingModel = L"Both", LPCWSTR szServerType = L"InprocServer32" );
STDAPI AMovieSetupUnregisterServer( CLSID clsServer );

//
// The NAME OF THE CAMERA CAN BE CHANGED HERE
//<==================== VS-START ====================>
// Actual name now defined as LPCTSTR literal makro in cam.h.
// This was needed because the NAME() makros in debug mode need a LPCTSTR,
// and therefor the debug version previously didn't compile.
const WCHAR SpoutCamName[] = L"" SPOUTCAMNAME;
//<==================== VS-END ======================>

//
// THE CLSID CAN BE CHANGED HERE
//
// For multiple cameras :
//
// 1) Change the name as above
// 2) Increment the last value of the CLSID e.g. 0x33, 0x34 etc.
//
// Then a separate camera with a different name and CLSID can be registered.
// Importantly, the resulting ".ax" files should be saved in different locations
// with different file names, so that they can be registered/unregistered separately.
// Match the file name with the camera name above to keep track of what you are doing.
// 
// You must use regsvr32 for registration.
// SpoutCamSettings is hard-coded for SpoutCam but can still be used to set fps
// and resolution. These settings are recorded in the registry and will be 
// picked up by all versions of SpoutCam, regardless of the name.
//
// When returning to the default SpoutCam, change the CLSID to back to the original
// {8E14549A-DB61-4309-AFA1-3578E927E933}
// 0x8e14549a, 0xdb61, 0x4309, 0xaf, 0xa1, 0x35, 0x78, 0xe9, 0x27, 0xe9, 0x33
//
DEFINE_GUID(CLSID_SpoutCam, 0x8e14549a, 0xdb61, 0x4309, 0xaf, 0xa1, 0x35, 0x78, 0xe9, 0x27, 0xe9, 0x33);

//<==================== VS-START ====================>
// And the property page we support

// {CD7780B7-40D2-4F33-80E2-B02E009CE63F}
DEFINE_GUID(CLSID_SpoutCamPropertyPage,
	0xcd7780b7, 0x40d2, 0x4f33, 0x80, 0xe2, 0xb0, 0x2e, 0x0, 0x9c, 0xe6, 0x3f);

// {6CD0D97B-C242-48DA-916D-28856D00754B}
DEFINE_GUID(IID_ICamSettings,
	0x6cd0d97b, 0xc242, 0x48da, 0x91, 0x6d, 0x28, 0x85, 0x6d, 0x00, 0x75, 0x4b);
//<==================== VS-END ======================>

const AMOVIESETUP_MEDIATYPE AMSMediaTypesVCam = 
{ 
    &MEDIATYPE_Video, 
    &MEDIASUBTYPE_NULL 
};

const AMOVIESETUP_PIN AMSPinVCam=
{
    L"Output",             // Pin string name
    FALSE,                 // Is it rendered
    TRUE,                  // Is it an output
    FALSE,                 // Can we have none
    FALSE,                 // Can we have many
    &CLSID_NULL,           // Connects to filter - obsolete
    NULL,                  // Connects to pin - obsolete
    1,                     // Number of types
    &AMSMediaTypesVCam     // Pointer to media types
};

const AMOVIESETUP_FILTER AMSFilterVCam =
{
    &CLSID_SpoutCam,	// Filter CLSID
	SpoutCamName,		// String name
    MERIT_DO_NOT_USE,   // Filter merit
						// Recommended for capture (http://msdn.microsoft.com/en-us/library/windows/desktop/dd388793%28v=vs.85%29.aspx)
    1,                  // Number pins
    &AMSPinVCam         // Pointer to pin information
};

CFactoryTemplate g_Templates[] = 
{
    {
		SpoutCamName,
        &CLSID_SpoutCam,
        CVCam::CreateInstance,
        NULL,
        &AMSFilterVCam
    }
	//<==================== VS-START ====================>
	,
	{
		L"Settings",
		&CLSID_SpoutCamPropertyPage,
		CSpoutCamProperties::CreateInstance
	}
	//<==================== VS-END ======================>
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

/*
HRESULT RegisterFilter(
  [in]       REFCLSID clsidFilter,
  [in]       LPCWSTR Name,
  [in, out]  IMoniker **ppMoniker,
  [in]       const CLSID *pclsidCategory,
  [in]       const OLECHAR *szInstance,
  [in]       const REGFILTER2 *prf2
);
*/

STDAPI RegisterFilters( BOOL bRegister )
{
    HRESULT hr = NOERROR;
    WCHAR achFileName[MAX_PATH];
    char achTemp[MAX_PATH];
    ASSERT(g_hInst != 0);

    if( 0 == GetModuleFileNameA(g_hInst, achTemp, sizeof(achTemp))) 
        return AmHresultFromWin32(GetLastError());

    MultiByteToWideChar(CP_ACP, 0L, achTemp, lstrlenA(achTemp) + 1, 
                       achFileName, NUMELMS(achFileName));
  
    hr = CoInitialize(0);
    if(bRegister)
    {
		hr = AMovieSetupRegisterServer(CLSID_SpoutCam, SpoutCamName, achFileName, L"Both", L"InprocServer32");
    }

    if( SUCCEEDED(hr) )
    {
        IFilterMapper2 *fm = 0;
        hr = CreateComObject( CLSID_FilterMapper2, IID_IFilterMapper2, fm );
        if( SUCCEEDED(hr) )
        {
            if(bRegister)
            {
                IMoniker *pMoniker = 0;
                REGFILTER2 rf2;
                rf2.dwVersion = 1;
                rf2.dwMerit = MERIT_DO_NOT_USE;
                rf2.cPins = 1;
                rf2.rgPins = &AMSPinVCam;
				hr = fm->RegisterFilter(CLSID_SpoutCam, SpoutCamName, &pMoniker, &CLSID_VideoInputDeviceCategory, NULL, &rf2);
            }
            else
            {
                hr = fm->UnregisterFilter(&CLSID_VideoInputDeviceCategory, 0, CLSID_SpoutCam);
            }
        }

      // release interface
      if(fm)
          fm->Release();
    }

    if( SUCCEEDED(hr) && !bRegister )
        hr = AMovieSetupUnregisterServer( CLSID_SpoutCam );

    CoFreeUnusedLibraries();
    CoUninitialize();
    return hr;
}

// This function will be called by a set-up application, or when the user runs the Regsvr32.exe tool.
// see http://msdn.microsoft.com/en-us/library/windows/desktop/dd376682%28v=vs.85%29.aspx
STDAPI DllRegisterServer()
{
	//<==================== VS-START ====================>
	HRESULT hr = RegisterFilters(TRUE);
	if (SUCCEEDED(hr)) hr = AMovieDllRegisterServer2(TRUE);
	return hr;
	//<==================== VS-END ======================>
}

STDAPI DllUnregisterServer()
{
	//<==================== VS-START ====================>
	HRESULT hr = RegisterFilters(FALSE);
	if (SUCCEEDED(hr)) hr = AMovieDllRegisterServer2(FALSE);
	return hr;
	//<==================== VS-END ======================>
}

//<==================== VS-START ====================>
#ifdef DIALOG_WITHOUT_REGISTRATION

extern "C" {
	HRESULT WINAPI OleCreatePropertyFrameDirect(
		HWND hwndOwner,
		LPCOLESTR lpszCaption,
		LPUNKNOWN* ppUnk,
		IPropertyPage * page);
}

HRESULT ShowFilterPropertyPageDirect(IBaseFilter *pFilter, HWND hwndParent)
{
	TRACE("ShowFilterPropertyPageDirect");
	HRESULT hr;
	if (!pFilter)
		return E_POINTER;
	CSpoutCamProperties * pPage = (CSpoutCamProperties *)CSpoutCamProperties::CreateInstance(pFilter, &hr);
	if (SUCCEEDED(hr))
	{
		hr = OleCreatePropertyFrameDirect(
			hwndParent,             // Parent window
			SpoutCamName,			// Caption for the dialog box
			(IUnknown **)&pFilter,  // Pointer to the filter
			pPage
		);
		pPage->Release();
	}
	return hr;
}
#endif

void CALLBACK Configure()
{
	HRESULT hr;
	IBaseFilter *pFilter;
	CUnknown *pInstance;

	CoInitialize(nullptr);

	// Obtain the filter's IBaseFilter interface.
	pInstance = CVCam::CreateInstance(nullptr, &hr);
	if (SUCCEEDED(hr))
	{
		hr = pInstance->NonDelegatingQueryInterface(IID_IBaseFilter, (void **)&pFilter);
		if (SUCCEEDED(hr))
		{
			// If the filter is registered, this will open the settings dialog.
			hr = ShowFilterPropertyPage(pFilter, GetDesktopWindow());

#ifdef DIALOG_WITHOUT_REGISTRATION
			if (FAILED(hr))
			{
				// The filter propably isn't registered in the system;
				// This will open the settings dialog anyway.
				hr = ShowFilterPropertyPageDirect(pFilter, GetDesktopWindow());
			}
#endif
			pFilter->Release();
		}
		delete pInstance;
	}

	CoUninitialize();
}
//<==================== VS-END ======================>

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  dwReason, LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}
