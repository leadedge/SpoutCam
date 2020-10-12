//////////////////////////////////////////////////////////////////////////
// DSUtil.h: DirectShow helper functions.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

// Conventions:
//
// Functions named "IsX" return true or false.
//
// Functions named "FindX" enumerate over a collection and return the first
// matching instance. 

#pragma once

#include <dshow.h>
#include <strsafe.h>
#include <assert.h>

#ifndef ASSERT
#define ASSERT assert
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if (x) { x->Release(); x = NULL; }
#endif

#ifndef CHECK_HR
#define CHECK_HR(hr) if (FAILED(hr)) { goto done; }
#endif

#define TRACE(_x_) ((void)0)

////////////////////////////////////////
// uncomment the below to trace to debug log, monitor with Sysinternals' DebugView
////////////////////////////////////////
//inline void OutputDebugStringf(const char * format, ...)
//{
//	char buffer[512];
//	va_list args;
//	va_start(args, format);
//	vsnprintf(buffer, 512, format, args);
//	OutputDebugStringA(buffer);
//	va_end(args);
//}
//#define TRACE(...) OutputDebugStringf(__VA_ARGS__);

///////////////////////////////////////////////////////////////////////
// Name: ShowFilterPropertyPage
// Desc: Show a filter's property page.
//
// hwndParent: Owner window for the property page.
///////////////////////////////////////////////////////////////////////
inline HRESULT ShowFilterPropertyPage(IBaseFilter *pFilter, HWND hwndParent)
{
    HRESULT hr;

    ISpecifyPropertyPages *pSpecify = NULL;
	FILTER_INFO FilterInfo;
	CAUUID caGUID;

    if (!pFilter)
	{
        return E_POINTER;
	}

	ZeroMemory(&FilterInfo, sizeof(FilterInfo));
	ZeroMemory(&caGUID, sizeof(caGUID));

    // Discover if this filter contains a property page
    CHECK_HR(hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpecify));
    CHECK_HR(hr = pFilter->QueryFilterInfo(&FilterInfo));
	CHECK_HR(hr = pSpecify->GetPages(&caGUID));

	// Display the filter's property page
	CHECK_HR(hr = OleCreatePropertyFrame(
			hwndParent,             // Parent window
			0,                      // x (Reserved)
			0,                      // y (Reserved)
			FilterInfo.achName,     // Caption for the dialog box
			1,                      // Number of filters
			(IUnknown **)&pFilter,  // Pointer to the filter 
			caGUID.cElems,          // Number of property pages
			caGUID.pElems,          // Pointer to property page CLSIDs
			0,                      // Locale identifier
			0,                      // Reserved
			NULL                    // Reserved
			));

done:
	CoTaskMemFree(caGUID.pElems);
	SAFE_RELEASE(FilterInfo.pGraph); 
    SAFE_RELEASE(pSpecify);
    return hr;
}
