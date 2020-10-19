/*

				SpoutCam
	
	Receives frames from a memory share or texture share Sender
	Simulates a webcam to other programs.

	The Sender must be started before the webcam is loaded
	by other software because this sets the frame size
	to the frames being sent by the Sender
	Currently there is no way to change the frame size
	after the filter has been loaded. If there is no Sender, 
	the default is 640x480 with a noise image

	Copyright 2013-2020 Lynn Jarvis - spout@zeal.co

	"SpoutCam" is free software: 
	you can redistribute it and/or modify it under the terms of the GNU
	Lesser General Public License as published by the Free Software Foundation, 
	either version 3 of the Licensfilbe, or (at your option) any later version.

	Credit for original capture source filter authored by Vivek :

	https://plus.google.com/100981910546057697518/posts

	Downloaded from :

	http://tmhare.mvps.org/downloads/vcam.zip

	Credit for alterations for Skype compatibility - John MacCormick, 2012.

	https://github.com/johnmaccormick/MultiCam
		
	"Spout" distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	See also the GNU general public license text file provided.
	If not found, see <http://www.gnu.org/licenses/>.

	SpoutCam is a modification of code by these authors and changes are clearly
	documented in this file. Any problems cannot be attributed to authors of
	previous versions.

	//
	// Revisions :
	//

	19-08-13 -started changes for synchronisation of FillBuffer
	with whether the filter is active or inactive - based on :
	22-08-13
		- used RED5 mods http://comSender.googlecode.com/svn/trunk/
		- succeeded in getting it working with Resolume and VLC
		- cleanup
	24.08-13
	found that 854 width is not supported, probably because widths have to be multiples of 4
	More info here :
	http://www.codeproject.com/Articles/437617/DirectShow-Virtual-Video-Capture-Source-Filter-in
	17-11-13 - noted a crash if "settings" are selected to show the property page
				which does not exist : possible information here :
				http://msdn.microsoft.com/en-us/library/windows/desktop/dd375480%28v=vs.85%29.aspx
	01.09.13 - minor cleanup
	28-09-13 - first release combined wgl interop and memory share
	13-10-13 - revision and cleanup
	16.10.13 - changed to ofxConnector to avoid confusion with original dxConnector
	09.11.13 - changed from memcpy to CopyMemory in vertflipbuf - very slight speed advantage
	11.11.13 - change project name to SpoutCam
	14.11.13 - check for intialized to avoid repeat of InitSharing in FillBuffer
	16.11.13 - added disconnect flag to protect against incorrect texture sharehandle or image size
	17.11.13 - added check for gl context in destructor to free fbo and texture and delete the context
	06.12.13 - changed ofxConnector to the same as for the FFGL plugins SpoutReceiver and videoshare
	06.12.13 - noted that the share handle can change even though the size remains the same
			   if a Vizzable device is dropped into the chain in Ableton. Allowed for that by reset
			   only if the image size remains the same. Still a problem if the image size changes.
	07-12-13 - added check for load of interop even though extensions loaded OK
			   It is possible that the extensions load OK, but that initialization will still fail
			   This occurs when wglDXOpenDeviceNV fails - noted on dual graphics machines with NVIDIA Optimus
	09.12.13 - changed to dll instead of constructor class
	12.12.13 - added a separate function to detect texture Sender change
	12.12.13 - allowed for change of active Sender as long as the image size is the same
	12.12.13 - changed back to spxConnector class instead of Spout.dll
	13.13.13 - started looking at a property page
	14.12.13 - found bug that if a Sender not running would leave initsharing with connected true
			 - changed default size from 640x480 to 320x240
	16.12.13 - moved hardware compatibility test to constructor but found that OPenGL context is needed
			   due to wglGetProcAddress
	19.12.13 - updated spxConnector after work with Processing sender project
	19.12.13 - removed cleanup after compatibility check and modified function in connector code
	19.12.13 - tidied up logic of initial compatibility check
	26.12.13 - added modified spxconnector again after more work with JSpout
	31.12.13 - Used Glew instead of dynamic load of extensions - in new version of spxConnector
	03.01.14 - changed to cleanded up connector code named back to ofxConnector
	17.01.14 - removed console window - inadvertently left active
	29.01.14 - included modified memoryshare with sender mutex
	07.02.14 - cleanup
	10.02.14 - major change to render the shared texture to a local texture then fillbuffer
			   This allows the camera image size to be independent of the shared texture image size
			   Limitation is that the camera has no properties to change size and remains as started
			   Benefit is that the initial size can be any size or shape
	14.02.14 - added modified memoryshare with check for sharing name on init
	26.02.14 - cleanup
	10.04.14 - changed all int width and height and functions to unsigned int
			 - changed to spxConnector common for 64bit compatibility
	24.04-14 - changed default frame rate to 60fps instead of 30fps - cured "stuttering" in Yawcam
			   VLC still needs strict timing control - simple method freezes
	05-07-14 - major change to use Spout SDK
	14-07-14 - used copymemory in FlipVertical instead of memcpy
	--------------------------------------------------------
	24.08.14 - recompiled with MB sendernames class revision
	29.08.14 - recompiled after changes with FFGL plugins
	03.09.14 - used DrawSharedTexture in render to local texture in FillBuffer
			 - update after SDK testing
	30.08.14 - Recompiled with SDK update
	12.10.14 - recompiled for release
	21.10.14 - Recompile for update V 2.001 beta
	02.01.15 - Added GL_BGR to extensions to avoid using GLEW
			   Recompile after SDK changes
	04.02.15 - added conditional compile for DirectX 9 or DirectX 11
			 - changed to direct OpenGL context creation rather than using glut
	08.02.15 - Set texture format for ReceiveImage to GL_RGB. SDK default is now GL_RGBA.
	25.04.15 - Changed from graphics auto detection to set DirectX mode to optional installer
	08.06.15 - Created a dummy window for OpenGL context creation due to SetPixelFormat error in Mapio
			 - needs testing with different programs and 64bit Windows 7 and 8
	17.06.15 - Transferred project to Win7 64bit SSD drive
			   Missing strmbase.lib - generated from DirectShow base classes - copied to a "libs" folder
	07.07.15 - Transferred project back to Win 7 32bit 
	17.08.15 - Removed SetDX9 - now done by registry setting
			   Recompile for 2.004 release 32bit Win32 VS2010 /MT
	15.09.15 - Recompiled for 2.005 memoryshare, changed memoryshare read section. 32bit Win32 VS2010 /MT.
	19.09.15 - Removed DX9 option
	20.09.15 - Sorted memoryshare readtexture RGBA external and RGB internal and local invert
	22.09.15 - Work on property page archived and removed from working source.
	27.09.15 - Modifications for Skype compatibility based on MultiCam by John MacCormick.
			 - changed back to simple timing method due to crash in Skype with RED5 method.
	28.09.15 - Found NULL clock with GetSyncSource for Skype, so fixed crash and enabled detailed timimg again.
	11.10.15 - Renamed DrawSharedTexture to DrawReceivedTexture to avoid confusion with SDK function
	15.12.15 - Changed dll name from "Spout Cam to "SpoutCam" - in dll.cpp
	15.12.15 - Rebuild for 2.005 VS2012 /MT
	11.03.16 - Major change for ReceiveImage into a local texture and rgb buffer and rescaling functions
	25.03.16 - Revised isExtensionSupported for glGetStringi
	20.05.16 - Remove testing of sender selection - not working.
	09.06.16 - Rebuild for 2.005 release  VS2012 /MT
	18.06.16 - Changes to ReceiveImage to use BGR filter buffer directly if possible
	23.06.16 - change back to 2.004 logic for access locks for texture read/write
	30.06.16 - VLC debugging added assert throughout
			   OK with vlc-2.2.0-win32
			   OK with vlc-2.2.1-win32
			   Does not crash with vlc-2.2.2-win32, but format is vertical and no image
			   Crash with vlc-2.2.3-win32
			   Crash with vlc-2.2.4-win32
			   OK with vlc-3.0.0-20160613-2343-git-win32 - 13th June - earliest nightly build
			   OK with vlc-3.0.0-20160629-0242-git-win32 - 29th June - latest nightly build
	03.06.16 - Rebuild with VS2015 / Windows 8
			   Rebuilt base classes - Note http://tmhare.mvps.org/faqs.htm#section_compile
			   You have the "Treat wchar_t as built-in type (Yes (/Zc:wchar_t))"
			   (under Project Properties -> C++ -> Language)
	10.07.16   Modified for "SpoutCamConfig" for starting fps and resolution
	13.07.16   Update for VS2015
	29.07.16   Update with memcpy optimisations in Spout SDK
	11.08.16   Debugging - SpoutControls to use sender selected by SpoutCamConfig if it is running
	06.10.16   Removed SpoutControls for release
			   Removed ASSERT debugging
			   Default resolution and frame rate so that the behaviour is the same as previous versions
	10.01.17   Start change to Spout 2.006
	23.01.17   Rebuild for Spout 2.006

	25.07.18   Rebuild for VS2017
			   Using the complete VS2017 project by Valentin Schmidt
			   (https://github.com/59de44955ebd/SpoutCam)

	21.12.18   Update Visual Studio to 15.9.4
	03.01.19   Changed to revised registry functions in SpoutUtils
			   Set PBO option off due to problems with 64 bit
			   Move all mode queries after OpenSpout
			   Rebuild Win32 and x64 for Spout 2.007, VS2017 /MT
			   Using build project by Valentin Schmidt
			   TODO - cleanup
	04.06.19   Rebuild VS2017 /MT 32/64bit
	17.10.19   Major change from OpenGL to DirectX 11
			   Added SpoutDX class
			   Removed unused Spout SDK files
			   Rebuild VS2017 /MT 32/64bit
	14.01.20   Use system time to calculate elapsed time if no parent clock.
	14.01.10   Add version information in cam.rc
			   Update to revised 2.007 SpoutSDK files
			   Rebuild 2.007 VS2017 /MT 32/64bit DirectX 11 Version 2.001 
	02.02.20   Correct resampling functions for staging texture pitch
			   Rebuild 2.007 VS2017 /MT 32/64bit DirectX 11 Version 2.002
	10.05.20   Revise for 2.007 functions
			   Rebuild 2.007 VS2017 /MT 32/64bit DirectX 11 Version 2.003
	09.06.20   Revise SpoutDX class
			   Rebuild 2.007 VS2017 /MT 32/64bit DirectX 11 Version 2.004
	23.06.30   Revise SpoutDX class and SpoutCopy
			   Memoryshare not supported for DirectX version
			   Rebuild 2.007 VS2017 /MT 32/64bit DirectX 11 Version 2.005
	23.06.30   Revise SpoutDX class
			   Changes to OpenDirectX and ReceiveRGBimage arguments
			   A couple of comments changed
			   Rebuild 2.007 VS2017 /MT 32/64bit DirectX 11 Version 2.006
	05.07.20   Update to latest SpoutDX class and latest SpoutCamSettings
			   Rebuild 2.007 VS2017 /MT 32/64bit DirectX 11 Version 2.007
	09.07.20   Correct incorrect colours for RGBA senders
			   Add rgba2rgb to SpoutCopy
			   Correct Version numbering from .0008 to .008
			   Rebuild 2.007 VS2017 /MT 32/64bit DirectX 11 Version 2.008
	01.08.20   Allow for name change to allow multiple cameras of different names
			   See dll.cpp for const WCHAR SpoutCamName[] and CLSID_SpoutCam
			   Test build VS2017 and VS2019 
			   Version 2.009
	02.08.20   Add special option to lock to a specific sender
			   See: SetReceiverName
			   Version 2.010
	3.08.20	   Improved efficiency of ReceiveRGBimage via spoutCopy::rgba2rgb
			   Version 2.011
	02.09.20   Revise ReadRGBpixels in SpoutDX.cpp to avoid memory leak
			   (https://github.com/leadedge/SpoutCam/issues/2)
			   Xor-shift random function to speed up static image
			   Version 2.012
	22.09.20   Update with SpoutDX from develop branch
			   Version 2.013
	28.09.20   Change GetMediatype for Zoom compatibility
			   Limit to one position and start with position 0
			   Change default fps from 60 to 30
			   Update with latest SpoutDX from develop branch
			   Version 2.014 - GitHub commit
	29.09.20   Add ReleaseReceiver if ReceiveRGBimage fails
			   Change default frame time to 333333 (30fps)
			   Change back to simple timing in FillBuffer due to freeze with Zoom
			   Add duration and Sleep to keep cycle correct
			   TODO : compatibility check with other hosts
	01.10.20   Move SetResolution to after registry check
			   Version 2.015
	02.10.20   Look for dropped frames and adjust
			   SpoutDX updated for async readback from GPU using two staging textures
	03.10.20   GitHub release Master v2.016 - Version 2.016
	04.10.20   Update with Revised SpoutDX ReceiveImage function
	06.10.20   Add "public IAMDroppedFrames" to the declaration of CVCamStream in "cam.h"
			   to prevent problems due to missing property dialog. Thanks to Valentin Schmidt.
			   Allow for receiving from DX9 senders in SpoutDX
			   Add options for image mirror, flip and swap (RGB <> BGR)
			   Requires SpoutCamSettings - Version 2.005 or greater
			   Verson 2.017
	09.10.20   Valentin Schmidt: Various changes to add a settings dialog
			   Verson 2.018
	09.10.20   Valentin Schmidt: added Mirror/Flip/Swap options to dialog
			   Verson 2.019
	12.10.20   CVCam::CreateInstance - SetProcessDPIAware() for clear options dialog
			   put_Settings simplify code for flags
			   Introduce ReleaseCamReceiver()
			   Remove connect/disconnect test
			   Showstatic if user and filter resolutions are different
			   Update verison.h - Version 2.020
			   Correct CSpoutCamProperties::OnApplyChanges()
			   for handle to fps and resolution controls for old comparison
	13.10.20   CSpoutCamProperties::OnApplyChanges() to restore 
			   fps & resolution list items on response to no change
			   Correct pvscc->InputSize.cx/cy in GetStreamCaps
			   Clean up unused code
			   Version 2.021
	13.10.20   Correct Flip true by default
			   Made seed for xorshiftRand static to prevent frozen image
			   Returned to timing by RED5
			   Conditional receive if DirectX initialization failed
			   Version 2.022
	16.10.20   FillBuffer correct cast for TimeGetTime calculations
			   Use TimeBeginPeriod for maximum precision of TimeGetTime and Sleep
			   DirectX device not required - in SpoutDX class.
			   Do not go to static on first DirectX initialize.
	17.10.20   Change to Unicode for WCHAR folder name support
			   Copy files to SpoutCam2017_DX project and GitHub DirectX master branch
			   Verson 2.023
	19.10.20   Add Spout website URL to properties dialog.
			   Add disable warnings checkbox mode to camprops.cpp.
			   Only for properties dialog - see OnReceiveMessage
			   Change version.h for unicode wide strings
			   Verson 2.024

*/

#pragma warning(disable:4244)
#pragma warning(disable:4711)

#include "cam.h"

// This is just a fast rand so that the static image isn't too slow for higher resolutions
// Based on Marsaglia's xorshift generator (http://www.jstatsoft.org/v08/i14/paper)
// and copied from (https://excamera.com/sphinx/article-xorshift.html)
static uint32_t seed = 7; // 100% random seed value
static uint32_t xorshiftRand()
{
	seed ^= seed << 13;
	seed ^= seed >> 17;
	seed ^= seed << 5;
	return seed;
}

//////////////////////////////////////////////////////////////////////////
//  CVCam is the source filter which masquerades as a capture device
//////////////////////////////////////////////////////////////////////////
CUnknown * WINAPI CVCam::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    ASSERT(phr);

	// debug console window
	// OpenSpoutConsole(); // Empty console
	// EnableSpoutLog(); // Show error logs
	// printf("SpoutCamDX ~~ Vers 2.024\n");

	// For clear options dialog for scaled display
	SetProcessDPIAware();

    CUnknown *punk = new CVCam(lpunk, phr);

    return punk;
}

CVCam::CVCam(LPUNKNOWN lpunk, HRESULT *phr) : 
	CSource(NAME(SPOUTCAMNAME), lpunk, CLSID_SpoutCam) //VS: replaced SpoutCamName with makro SPOUTCAMNAME, NAME() expects LPCTSTR
{
    ASSERT(phr);

    CAutoLock cAutoLock(&m_cStateLock);
    
	// Create the one and only output pin
    m_paStreams = (CSourceStream **)new CVCamStream*[1];
	m_paStreams[0] = new CVCamStream(phr, this, (LPCWSTR)&SpoutCamName);

	if (phr) *phr = S_OK; //VS
}

// Retrieves pointers to the supported interfaces on an object.
// This method calls IUnknown::AddRef on the pointer it returns.
HRESULT CVCam::QueryInterface(REFIID riid, void **ppv)
{
	//Forward request for IAMStreamConfig & IKsPropertySet to the pin
    if(riid == _uuidof(IAMStreamConfig) || 
		riid == _uuidof(IAMDroppedFrames)  ||
		riid == _uuidof(IKsPropertySet))
        return m_paStreams[0]->QueryInterface(riid, ppv);
	//<==================== VS-START ====================>
	else if (riid == IID_ICamSettings)
		return GetInterface((ICamSettings *)this, ppv);
	else if (riid == IID_ISpecifyPropertyPages)
		return GetInterface((ISpecifyPropertyPages *)this, ppv);
	//<==================== VS-END ======================>
    else
        return CSource::QueryInterface(riid, ppv);
}

// If a filter cannot deliver data for some reason, it returns VFW_S_CANT_CUE. 
HRESULT CVCam::GetState(DWORD dw, FILTER_STATE *pState)
{
	CheckPointer(pState, E_POINTER);
	*pState = m_State;
	if (m_State == State_Paused) {
		return VFW_S_CANT_CUE;
	}
	else
		return S_OK;
}

STDMETHODIMP CVCam::JoinFilterGraph(
		__inout_opt IFilterGraph * pGraph,
		__in_opt LPCWSTR pName)
{
	HRESULT hr = CBaseFilter::JoinFilterGraph(pGraph, pName);
	return hr;
}

//<==================== VS-START ====================>

//////////////////////////////////////////////////////////////////////////
// ISpecifyPropertyPages interface
//////////////////////////////////////////////////////////////////////////

// GetPages
// Returns the clsid's of the property pages we support
STDMETHODIMP CVCam::GetPages(CAUUID *pPages)
{
	CheckPointer(pPages, E_POINTER);
	pPages->cElems = 1;
	pPages->pElems = (GUID *)CoTaskMemAlloc(sizeof(GUID));
	if (pPages->pElems == NULL)
		return E_OUTOFMEMORY;
	pPages->pElems[0] = CLSID_SpoutCamPropertyPage;
	return S_OK;
}

STDMETHODIMP CVCam::put_Settings(DWORD dwFps, DWORD dwResolution, DWORD dwMirror, DWORD dwSwap, DWORD dwFlip)
{
	((CVCamStream *)m_paStreams[0])->put_Settings(dwFps, dwResolution, dwMirror, dwSwap, dwFlip);
	return S_OK;
}

//<==================== VS-END ======================>

///////////////////////////////////////////////////////////
// all inherited virtual functions
///////////////////////////////////////////////////////////
CBasePin *CVCam::GetPin(int n)
{
	CBasePin *pin = CSource::GetPin(n);
	ASSERT(pin!=NULL);
	return pin;
}

int CVCam::GetPinCount()
{
	return CSource::GetPinCount();
}

HRESULT CVCam::StreamTime(CRefTime& rtStream)
{
	return CSource::StreamTime(rtStream);
}

LONG CVCam::GetPinVersion()
{
	return CSource::GetPinVersion();
}

__out_opt LPAMOVIESETUP_FILTER CVCam::GetSetupData()
{
	return CSource::GetSetupData();
}

HRESULT STDMETHODCALLTYPE CVCam::EnumPins(__out  IEnumPins **ppEnum)
{
	return CSource::EnumPins(ppEnum);
}

HRESULT STDMETHODCALLTYPE CVCam::FindPin(LPCWSTR Id, __out  IPin **ppPin)
{
	return CSource::FindPin(Id, ppPin);
}

HRESULT STDMETHODCALLTYPE CVCam::QueryFilterInfo(__out  FILTER_INFO *pInfo)
{
	return CSource::QueryFilterInfo(pInfo);
}

HRESULT STDMETHODCALLTYPE CVCam::QueryVendorInfo(__out  LPWSTR *pVendorInfo)
{
	return CSource::QueryVendorInfo(pVendorInfo);
}

HRESULT STDMETHODCALLTYPE CVCam::Stop( void)
{
	return CSource::Stop( );
}

HRESULT STDMETHODCALLTYPE CVCam::Pause( void)
{
	return CSource::Pause( );
}

HRESULT STDMETHODCALLTYPE CVCam::Run(REFERENCE_TIME tStart)
{
	HRESULT hr = CSource::Run(tStart);
	return hr;
}

HRESULT STDMETHODCALLTYPE CVCam::SetSyncSource(__in_opt  IReferenceClock *pClock)
{
	return CSource::SetSyncSource(pClock);
}

HRESULT STDMETHODCALLTYPE CVCam::GetSyncSource(__deref_out_opt  IReferenceClock **pClock)
{
	return CSource::GetSyncSource(pClock);
}

STDMETHODIMP CVCam::GetClassID(__out CLSID *pClsID)
{
	return CSource::GetClassID(pClsID);
}

ULONG STDMETHODCALLTYPE CVCam::AddRef( void)
{
	return CSource::AddRef( );
}

ULONG STDMETHODCALLTYPE CVCam::Release( void)
{
	return CSource::Release( );
}

HRESULT STDMETHODCALLTYPE CVCam::Register( void)
{
	return CSource::Register( );
}

HRESULT STDMETHODCALLTYPE CVCam::Unregister( void)
{
	return CSource::Unregister( );
}
//////////////////////////////////////////////////////////////////////////

//<==================== VS-START ====================>
HRESULT CVCamStream::put_Settings(DWORD dwFps, DWORD dwResolution, DWORD dwMirror, DWORD dwSwap, DWORD dwFlip)
{
	// Calling Disconnect() would disconnect SpoutCam with a connected filter (e.g. Color Space Converter) in GraphStudioNext, 
	// but something is still missing, you can't reconnect the existing filters with the new cam settings.
	// For now, a better solution is to let the user select "Disconnect all Filters" from the "Graph" menu after SpoutCam's settings
	// were changed by its Properties dialog, then rendering SpoutCam's output pin works fine and all filters are successfully reconnected,
	// taking into account the new fps/resolution settings.

	SetFps(dwFps);
	SetResolution(dwResolution);
	receiver.m_bMirror = (dwMirror > 0);
	receiver.m_bSwapRB = (dwSwap > 0);
	// Flip is true by default - so false will appear inverted
	bInvert = !(dwFlip > 0);

	return GetMediaType(0, &m_mt);
}
//<==================== VS-END ======================>

//////////////////////////////////////////////////////////////////////////
// CVCamStream is the one and only output pin of CVCam which handles 
// all the stuff.
//////////////////////////////////////////////////////////////////////////
CVCamStream::CVCamStream(HRESULT *phr, CVCam *pParent, LPCWSTR pPinName) :
	CSourceStream(NAME(SPOUTCAMNAME), phr, pParent, pPinName), m_pParent(pParent) //VS: replaced SpoutCamName with makro SPOUTCAMNAME, NAME() expects LPCTSTR
{
	bDXinitialized  = false; // DirectX
	bMemoryMode		= false; // Default mode is texture, true means memoryshare
	bInvert         = true;  // Flip vertically
	bInitialized	= false; // Spoutcam receiver
	g_Width			= 640;	 // give it an initial size - this will be changed if a sender is running at start
	g_Height		= 480;
	g_SenderName[0] = 0;
	g_ActiveSender[0] = 0;
	
	// Maximum precision of timeGetTime used in FillBuffer
	timeGetDevCaps(&g_caps, sizeof(g_caps));
	timeBeginPeriod(g_caps.wPeriodMin);

	//
	// Retrieve fps and resolution from registry "SpoutCamConfig"
	//		o Fps
	//			10	0
	//			15	1
	//			25	2
	//			30	3 (default)
	// Compatibility problems with some software if > 60 fps
	//			50	4
	//			60	5
	//
	// Fps from SpoutCamConfig (default 3 = 30)
	if (!ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "fps", &dwFps)) {
		dwFps = 3;
	}

	//		o Resolution
	//			Sender			0
	//			320 x 240		1
	//			640 x 360		2
	//			640 x 480		3 (default if no sender)
	//			800 x 600		4
	//			1024 x 720		5	
	//			1024 x 768		6
	//			1280 x 720		7
	//			1280 x 960		8
	//			1280 x 1024		9
	//			1920 x 1080		10
	//
	// Resolution from SpoutCamConfig (default 0 = active sender)
	if (!ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "resolution", &dwResolution))
	{
		dwResolution = 0;
	}

	// Find out whether memoryshare mode is selected by SpoutSettings
	// (Memoryshare is not supported by DirectX)
	bMemoryMode = receiver.GetMemoryShareMode();

	//
	// Options in SpoutCamSettings
	//

	// Mirror image
	DWORD dwMirror = 0;
	ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "mirror", &dwMirror);

	// RGB <> BGR
	DWORD dwSwap = 0;
	ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "swap", &dwSwap);

	// Flip image
	// Default is flipped due to upside down Windows bitmap
	// If set false, the result comes out inverted
	// bInvert = false; 
	DWORD dwFlip = 0;
	ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "flip", &dwFlip);

	//<==================== VS-START ====================>
	// Now always the new function put_Settings is called, which in turn calls
	// SetFps and SetResolution. Dealing with resolution 0 (Sender) was moved to
	// SetResolution.
	put_Settings(dwFps, dwResolution, dwMirror, dwSwap, dwFlip);
	//<==================== VS-END ======================>

	NumDroppedFrames = 0LL;
	NumFrames = 0LL;

	//
	// Special purpose option : Lock to a specific sender
	//
	// SpoutCam will only connect to the name specied.
	// Any other sender is ignored and SpoutCam will produce static.
	// SpoutCam can be started before or after the sender.
	// Leave this out for SpoutCam to detect to any sender.
	//
	// receiver.SetReceiverName("Spout DX11 Sender"); // Specify the sender name here

}

void CVCamStream::SetFps(DWORD dwFps)
{
	// The desired average display time of the video frames, in 100-nanosecond units. 
	// 0 - 10fps = 1000000
	// 1 - 15fps =  666667
	// 2 - 25fps =  400000
	// 3 - 30fps =  333333 (default)
	// 4 - 50fps =  200000
	// 5 - 60fps =  166667
	switch(dwFps) {
		case 0 :
			g_FrameTime = 1000000; // 10
			break;
		case 1 :
			g_FrameTime = 666667; // 15
			break;
		case 2 :
			g_FrameTime = 400000; // 25
			break;
		case 3 :
			g_FrameTime = 333333; // 30
			break;
		case 4 :
			g_FrameTime = 200000; // 50
			break;
		case 5 :
			g_FrameTime = 166667; // 60
			break;
		default :
			g_FrameTime = 333333; // default 30
			break;
	}
}

void CVCamStream::SetResolution(DWORD dwResolution)
{
	switch(dwResolution) {

		//<==================== VS-START ====================>
		// Case 0 - use the active sender or default
		case 0:
			{
				// If there is no Sender, getmediatype will use the resolution set by the user
				// Resolution index 0 means that the user has selected "Active sender"
				// Use the resolution of the active sender if one is running
				if (receiver.GetActiveSender(g_SenderName))
				{
					unsigned int width, height;
					HANDLE sharehandle;
					DWORD format;
					if (receiver.GetSenderInfo(g_SenderName, width, height, sharehandle, format))
					{
						// If not fixed to the a selected resolution, use the sender width and height
						g_Width = width;
						g_Height = height;
					}
				}
			}
			break;
		//<==================== VS-END ======================>

		case 1 :
			g_Width  = 320; // 1 
			g_Height = 240;
			break;
		case 2 :
			g_Width  = 640; // 2
			g_Height = 360;
			break;
		case 3 :
			g_Width  = 640; // 3 (default)
			g_Height = 480;
			break;
		case 4 :
			g_Width  = 800; // 4
			g_Height = 600;
			break;
		case 5 :
			g_Width  = 1024; // 5
			g_Height = 720;
			break;
		case 6 :
			g_Width  = 1024; // 6
			g_Height = 768;
			break;
		case 7 :
			g_Width  = 1280; // 7
			g_Height = 720;
			break;
		case 8 :
			g_Width  = 1280; // 8
			g_Height = 960;
			break;
		case 9 :
			g_Width  = 1280; // 9
			g_Height = 1024;
			break;
		case 10 :
			g_Width  = 1920; // 10
			g_Height = 1080;
			break;
		default :
			g_Width  = 640; // 3 (default)
			g_Height = 480;
			break;
	}
}

CVCamStream::~CVCamStream()
{
	if(bInitialized) 
		receiver.ReleaseReceiver();

	if (bDXinitialized)
		receiver.CleanupDX11();

	// End timer precision
	timeEndPeriod(g_caps.wPeriodMin);

} 

HRESULT CVCamStream::QueryInterface(REFIID riid, void **ppv)
{  
	// Standard OLE stuff
    if(riid == _uuidof(IAMStreamConfig))
        *ppv = (IAMStreamConfig*)this;
    else if(riid == _uuidof(IKsPropertySet))
        *ppv = (IKsPropertySet*)this;
	else if(riid == _uuidof(IAMDroppedFrames)  )
		*ppv = (IAMDroppedFrames*)this;
	else
        return CSourceStream::QueryInterface(riid, ppv);

    AddRef();

    return S_OK;
}


//////////////////////////////////////////////////////////////////////////
//  This is the routine where we create the data being output by the Virtual
//  Camera device.
//	Modified as per red5 to allow for dropped frames and reset of time stamps
//
//  http://comSender.googlecode.com/svn/trunk/
//
//////////////////////////////////////////////////////////////////////////
HRESULT CVCamStream::FillBuffer(IMediaSample *pms)
{
	unsigned int imagesize, width, height = 0U;
	long l, lDataLen = 0L;
	bool bResult = false;
	HRESULT hr = S_OK;
    BYTE *pData = nullptr;

	VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)m_mt.Format();

	// If graph is inactive stop cueing samples
	if(!m_pParent->IsActive()) {
		return S_FALSE;
	}

	// Memory share mode not supported by DirectX
	if (bMemoryMode)
		return FALSE;
	
	//
	// Timing - modified from Red5 method
	//

	// Set the timestamps that will govern playback frame rate.
	// The current time is the sample's start.
	REFERENCE_TIME rtNow = m_rtLastTime;
	REFERENCE_TIME avgFrameTime = g_FrameTime; // Desired average frame time is set by the user

	// Create some working info
	REFERENCE_TIME rtDelta, rtDelta2 = 0LL; // delta for dropped, delta 2 for sleep.

	//
	// What time is it REALLY ???
	//
	m_pParent->GetSyncSource(&m_pClock);
	if (m_pClock) {
		m_pClock->GetTime(&refSync1);
		m_pClock->Release();
	}
	else {
		// Some programs do not implement the DirectShow clock and can crash if assumed
		// so we can use TimeGetTime instead. Only millisecond precision, but so is Sleep.
		refSync1 = (REFERENCE_TIME)timeGetTime() * 10000LL; // Cast before the multiply to avoid overflow
	}

	if (NumFrames <= 1)	{
		// initiate values
		refStart = refSync1; // FirstFrame No Drop
		refSync2 = 0;
	}

	rtNow = m_rtLastTime;
	m_rtLastTime = avgFrameTime + m_rtLastTime;

	// IAMDropppedFrame. We only have avgFrameTime to generate image.
	// Find generated stream time and compare to real elapsed time.
	rtDelta = ((refSync1 - refStart) - (((NumFrames)*avgFrameTime) - avgFrameTime));
	if (rtDelta - refSync2 < 0)	{
		// we are early
		rtDelta2 = rtDelta - refSync2;
		DWORD dwSleep = (DWORD)abs(rtDelta2 / 10000LL);
		if (dwSleep >= 1)
			Sleep(dwSleep);
	}
	else if (rtDelta / avgFrameTime > NumDroppedFrames)	{	
		// new dropped frame
		NumDroppedFrames = rtDelta / avgFrameTime;
		// Figure new RT for sleeping
		refSync2 = NumDroppedFrames * avgFrameTime;
		// Our time stamping needs adjustment.
		// Find total real stream time from start time.
		rtNow = refSync1 - refStart;
		m_rtLastTime = rtNow + avgFrameTime;
		pms->SetDiscontinuity(true);
	}

	// The SetTime method sets the stream times when this sample should begin and finish.
	hr = pms->SetTime(&rtNow, &m_rtLastTime);
	// Set true on every sample for uncompressed frames
	hr = pms->SetSyncPoint(true);
	// ============== END OF INITIAL TIMING ============

	// Check access to the sample's data buffer
    pms->GetPointer(&pData);
	if (pData == NULL) {
		return NOERROR;
	}

	// Get the current frame size for texture transfers
    imagesize = (unsigned int)pvi->bmiHeader.biSizeImage;
	width     = (unsigned int)pvi->bmiHeader.biWidth;
	height    = (unsigned int)pvi->bmiHeader.biHeight;
	if (width == 0 || height == 0) {
		return NOERROR;
	}

	// Sizes should be OK, but check again
	unsigned int size = (unsigned int)pms->GetSize();
	// TODO : check imagesize = width*height*3;
	if(size != imagesize) { // imagesize retrieved above
		ReleaseCamReceiver();
		goto ShowStatic;
	}

	// Quit if nothing running at all
	if(!receiver.GetActiveSender(g_ActiveSender)) {
		ReleaseCamReceiver();
		goto ShowStatic;
	}

	// Initialize DirectX if is has not been done
	// TODO : prevent retries
	if(!bDXinitialized) {
		if (!receiver.OpenDirectX11()) {
			return NOERROR;
		}
		bDXinitialized = true;
	} // endif !bDXinitialized
	
	// DirectX is initialized OK
	// Get bgr pixels from the sender bgra shared texture
	// ReceiveImage handles sender detection, connection and copy of pixels
	if (receiver.ReceiveImage(pData, g_Width, g_Height, true, bInvert)) {
		// rgb(not rgba) = true, invert = true
		// If IsUpdated() returns true, the sender has changed
		if (receiver.IsUpdated()) {
			if (strcmp(g_SenderName, receiver.GetSenderName()) != 0) {
				// Only test for change of sender name.
				// The pixel buffer (pData) remains the same size and 
				// ReceiveImage uses resampling for a different texture size
				strcpy_s(g_SenderName, 256, receiver.GetSenderName());
				// Set the sender name to the registry for SpoutCamSettings
				WritePathToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutCam", "sendername", g_SenderName);
			}
		}
		bInitialized = true;
		NumFrames++;
		return NOERROR;
	}
	else {
		ReleaseCamReceiver();
	}

ShowStatic :

	// drop through to default static image if it did not work
	pms->GetPointer(&pData);
	lDataLen = pms->GetSize();
	for (l = 0; l < lDataLen; ++l)
		pData[l] = (char)xorshiftRand(); // rand();

	NumFrames++;

	return NOERROR;

} // FillBuffer


// Conditionally release receiver and reset flag
void CVCamStream::ReleaseCamReceiver()
{
	if (bInitialized) {
		receiver.ReleaseReceiver();
		bInitialized = false;
	}
}


//
// Notify
// Ignore quality management messages sent from the downstream filter
STDMETHODIMP CVCamStream::Notify(IBaseFilter * pSender, Quality q)
{
    return E_NOTIMPL;
} // Notify

//////////////////////////////////////////////////////////////////////////
// This is called when the output format has been negotiated
// Called when a media type is agreed between filters
//////////////////////////////////////////////////////////////////////////
HRESULT CVCamStream::SetMediaType(const CMediaType *pmt)
{
	ASSERT(pmt);
    
	// Pass the call up to my base class
	HRESULT hr = CSourceStream::SetMediaType(pmt);

    return hr;
}

// See Directshow help topic for IAMStreamConfig for details on this method
HRESULT CVCamStream::GetMediaType(int iPosition, CMediaType *pmt)
{
	unsigned int width, height;

	if(iPosition < 0) {
		return E_INVALIDARG;
	}

	if (iPosition > 1) {
		return VFW_S_NO_MORE_ITEMS;
	}
	
	DECLARE_PTR(VIDEOINFOHEADER, pvi, pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER)));
    ZeroMemory(pvi, sizeof(VIDEOINFOHEADER));

 	// Allow for default
	if(g_Width == 0 || g_Height == 0) {
		width  = 640;
		height = 480;
	}
	else {
		// as per Spout sender received
		width	=  g_Width;
		height	=  g_Height;
	}

	pvi->bmiHeader.biSize				= sizeof(BITMAPINFOHEADER);
	pvi->bmiHeader.biWidth				= (LONG)width;
	pvi->bmiHeader.biHeight				= (LONG)height;
	pvi->bmiHeader.biPlanes				= 1;
	pvi->bmiHeader.biBitCount			= 24;
	pvi->bmiHeader.biCompression		= 0; // defaults 
	pvi->bmiHeader.biSizeImage			= 0;
	pvi->bmiHeader.biClrImportant		= 0;
	pvi->bmiHeader.biSizeImage			= GetBitmapSize(&pvi->bmiHeader);

	// The desired average display time of the video frames, in 100-nanosecond units. 
	// 10fps = 1000000
	// 15fps =  666667
	// 30fps =  333333
	// 60fps =  166667
	pvi->AvgTimePerFrame = g_FrameTime; // From SpoutCamConfig - 30fps default

    SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered.
    SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

    pmt->SetType(&MEDIATYPE_Video);
    pmt->SetFormatType(&FORMAT_VideoInfo);
    pmt->SetTemporalCompression(false);

    // Work out the GUID for the subtype from the header info.
    const GUID SubTypeGUID = GetBitmapSubtype(&pvi->bmiHeader);
    pmt->SetSubtype(&SubTypeGUID);
	pmt->SetVariableSize(); // LJ - to be checked
    pmt->SetSampleSize(pvi->bmiHeader.biSizeImage);

    return NOERROR;

} // GetMediaType


// This method is called to see if a given output format is supported
HRESULT CVCamStream::CheckMediaType(const CMediaType *pMediaType)
{
	if(*pMediaType != m_mt) 
        return E_INVALIDARG;

    return S_OK;
} // CheckMediaType

//
// This method is called after the pins are connected to allocate buffers to stream data
// This will always be called after the format has been sucessfully
// negotiated. So we have a look at m_mt to see what size image we agreed.
// Then we can ask for buffers of the correct size to contain them.
//
HRESULT CVCamStream::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());
    HRESULT hr = NOERROR;

    VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) m_mt.Format();
    pProperties->cBuffers = 1;
    pProperties->cbBuffer = pvi->bmiHeader.biSizeImage;

	ASSERT(pProperties->cbBuffer);

    // Ask the allocator to reserve us some sample memory. NOTE: the function
    // can succeed (return NOERROR) but still not have allocated the
    // memory that we requested, so we must check we got whatever we wanted.
    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties, &Actual);

    if(FAILED(hr)) return hr;

	// Is this allocator unsuitable?
    if(Actual.cbBuffer < pProperties->cbBuffer) return E_FAIL;

    return NOERROR;

} // DecideBufferSize

// Called when graph is run
HRESULT CVCamStream::OnThreadCreate()
{
    m_rtLastTime = 0;
	dwLastTime = 0;
	NumDroppedFrames = 0;
	NumFrames = 0;

    return NOERROR;

} // OnThreadCreate


//////////////////////////////////////////////////////////////////////////
//  IAMStreamConfig
//////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CVCamStream::SetFormat(AM_MEDIA_TYPE *pmt)
{
	// http://kbi.theelude.eu/?p=161
	if(!pmt) return S_OK; // Default? red5

	VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)(pmt->pbFormat);
	VIDEOINFOHEADER *mvi = (VIDEOINFOHEADER *)(m_mt.Format ());

	if(pvi->bmiHeader.biHeight !=mvi->bmiHeader.biHeight || 
		pvi->bmiHeader.biWidth  != mvi->bmiHeader.biWidth || 
		pvi->bmiHeader.biBitCount !=mvi->bmiHeader.biBitCount  )
		return VFW_E_INVALIDMEDIATYPE;	

	// maximum fps - minimum frame time
	if(pvi->AvgTimePerFrame < 10000000/60)
		return VFW_E_INVALIDMEDIATYPE;
	if(pvi->AvgTimePerFrame < 1)
		return VFW_E_INVALIDMEDIATYPE;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CVCamStream::GetFormat(AM_MEDIA_TYPE **ppmt)
{
    *ppmt = CreateMediaType(&m_mt);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CVCamStream::GetNumberOfCapabilities(int *piCount, int *piSize)
{
	*piCount = 1; // LJ
    *piSize = sizeof(VIDEO_STREAM_CONFIG_CAPS);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CVCamStream::GetStreamCaps(int iIndex, AM_MEDIA_TYPE **pmt, BYTE *pSCC)
{

	unsigned int width, height;

    *pmt = CreateMediaType(&m_mt);

    DECLARE_PTR(VIDEOINFOHEADER, pvi, (*pmt)->pbFormat);

	if (iIndex == 0) iIndex = 1;

	if(g_Width == 0 || g_Height == 0) {
		width  = 640;
		height = 480;
	}
	else {
		// as per sending app
		width	=  g_Width;
		height	=  g_Height;
	}

	pvi->bmiHeader.biCompression	= BI_RGB;
    pvi->bmiHeader.biBitCount		= 24;
    pvi->bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
    pvi->bmiHeader.biWidth			= (LONG)width;
    pvi->bmiHeader.biHeight			= (LONG)height;
    pvi->bmiHeader.biPlanes			= 1;
    pvi->bmiHeader.biSizeImage		= GetBitmapSize(&pvi->bmiHeader);
    pvi->bmiHeader.biClrImportant	= 0;

    SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered.
    SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

    (*pmt)->majortype				= MEDIATYPE_Video;
    (*pmt)->subtype					= MEDIASUBTYPE_RGB24;
    (*pmt)->formattype				= FORMAT_VideoInfo;
    (*pmt)->bTemporalCompression	= false;
    (*pmt)->bFixedSizeSamples		= false;
    (*pmt)->lSampleSize				= pvi->bmiHeader.biSizeImage;
    (*pmt)->cbFormat				= sizeof(VIDEOINFOHEADER);
    
    DECLARE_PTR(VIDEO_STREAM_CONFIG_CAPS, pvscc, pSCC);
    
    pvscc->guid = FORMAT_VideoInfo;
    pvscc->VideoStandard = AnalogVideo_None;
	// Native size of the incoming video signal. 
	// For a compressor, the size is taken from the input pin.
	// For a capture filter, the size is the largest signal the filter 
	// can digitize with every pixel remaining unique.
	// Note  Deprecated.
	pvscc->InputSize.cx         = (LONG)width; // 1920;
    pvscc->InputSize.cy			= (LONG)height; // 1080;
    pvscc->MinCroppingSize.cx	= 0; // LJ was 80 but we don't want to limit it
    pvscc->MinCroppingSize.cy	= 0; // was 60
    pvscc->MaxCroppingSize.cx	= 1920;
    pvscc->MaxCroppingSize.cy	= 1080;
    pvscc->CropGranularityX		= 1; // seems 1 is not necessary
    pvscc->CropGranularityY		= 1;
    pvscc->CropAlignX = 0;
    pvscc->CropAlignY = 0;

    pvscc->MinOutputSize.cx		= 80; // LJ fair enough
    pvscc->MinOutputSize.cy		= 60;
    pvscc->MaxOutputSize.cx		= 1920; // 1080p
    pvscc->MaxOutputSize.cy		= 1080;
    pvscc->OutputGranularityX	= 1;
    pvscc->OutputGranularityY	= 1;
    pvscc->StretchTapsX			= 0;
    pvscc->StretchTapsY			= 0;
    pvscc->ShrinkTapsX			= 0;
    pvscc->ShrinkTapsY			= 0;
	pvscc->MinFrameInterval = 166667;   // 60 fps 333333; // 30fps  // LJ what is the consequence of this ?
    pvscc->MaxFrameInterval = 50000000; // 0.2 fps
    pvscc->MinBitsPerSecond = (80 * 60 * 3 * 8) / 5;
    pvscc->MaxBitsPerSecond = 1920 * 1080 * 3 * 8 * 30; // (integral overflow at 60 - anyway we lock on to 30fps and 1920 might not achieve 60fps)

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//			IAMDroppedFrames
///////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CVCamStream::GetNumNotDropped (long* plNotDropped)
{
	
	if (!plNotDropped) 
		return E_POINTER;
	
	*plNotDropped=NumFrames;
		return NOERROR;
}

HRESULT STDMETHODCALLTYPE CVCamStream::GetNumDropped (long* plDropped)
{
	if (!plDropped) 
		return E_POINTER;
	
	*plDropped=NumDroppedFrames;
		return NOERROR;
}

HRESULT STDMETHODCALLTYPE CVCamStream::GetDroppedInfo (long lSize,long *plArraym,long* plNumCopied)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CVCamStream::GetAverageFrameSize (long* plAverageSize)
{
	if(!plAverageSize)return E_POINTER;
	*plAverageSize=307200; // 640x480
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// IKsPropertySet
//////////////////////////////////////////////////////////////////////////
HRESULT CVCamStream::Set(REFGUID guidPropSet, DWORD dwID, void *pInstanceData, 
                        DWORD cbInstanceData, void *pPropData, DWORD cbPropData)
{
	// Set: Cannot set any properties.
    return E_NOTIMPL;
}

// Get: Return the pin category (our only property). 
HRESULT CVCamStream::Get(
    REFGUID guidPropSet,   // Which property set.
    DWORD dwPropID,        // Which property in that set.
    void *pInstanceData,   // Instance data (ignore).
    DWORD cbInstanceData,  // Size of the instance data (ignore).
    void *pPropData,       // Buffer to receive the property data.
    DWORD cbPropData,      // Size of the buffer.
    DWORD *pcbReturned     // Return the size of the property.
	)
{

    if (guidPropSet != AMPROPSETID_Pin)             return E_PROP_SET_UNSUPPORTED;
    if (dwPropID != AMPROPERTY_PIN_CATEGORY)        return E_PROP_ID_UNSUPPORTED;
    if (pPropData == NULL && pcbReturned == NULL)   return E_POINTER;
    
    if (pcbReturned) *pcbReturned = sizeof(GUID);
    if (pPropData == NULL)          return S_OK;			// Caller just wants to know the size. 
    if (cbPropData < sizeof(GUID))  return E_UNEXPECTED;	// The buffer is too small.
        
    *(GUID *)pPropData = PIN_CATEGORY_CAPTURE;

    return S_OK;
}

// QuerySupported: Query whether the pin supports the specified property.
HRESULT CVCamStream::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport)
{
    if (guidPropSet != AMPROPSETID_Pin) return E_PROP_SET_UNSUPPORTED;
    if (dwPropID != AMPROPERTY_PIN_CATEGORY) return E_PROP_ID_UNSUPPORTED;
    // We support getting this property, but not setting it.
    if (pTypeSupport) *pTypeSupport = KSPROPERTY_SUPPORT_GET; 
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CVCamStream::QueryPinInfo(__out  PIN_INFO *pInfo)
{
	return CSourceStream::QueryPinInfo(pInfo);
}

STDMETHODIMP  CVCamStream::NonDelegatingQueryInterface(REFIID riid, __deref_out void **ppv)
{
	return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT STDMETHODCALLTYPE CVCamStream::ConnectedTo(__out  IPin **pPin)
{
	return CSourceStream::ConnectedTo(pPin);
}

HRESULT STDMETHODCALLTYPE CVCamStream::EnumMediaTypes(__out  IEnumMediaTypes **ppEnum)
{
	return CSourceStream::EnumMediaTypes(ppEnum);
}

LONG  CVCamStream::GetMediaTypeVersion()
{
	return CSourceStream::GetMediaTypeVersion();
}

HRESULT  CVCamStream::CompleteConnect(IPin *pReceivePin)
{
	return CSourceStream::CompleteConnect(pReceivePin);
}

HRESULT  CVCamStream::CheckConnect(IPin *pPin)
{
	return CSourceStream::CheckConnect(pPin);
}

HRESULT  CVCamStream::BreakConnect()
{
	return CSourceStream::BreakConnect();
}

HRESULT STDMETHODCALLTYPE CVCamStream::Connect(IPin *pReceivePin,	__in_opt  const AM_MEDIA_TYPE *pmt)
{
	return CSourceStream::Connect( pReceivePin, pmt);
}

HRESULT STDMETHODCALLTYPE CVCamStream::ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt)
{
	return CSourceStream::ReceiveConnection( pConnector,pmt);
}

HRESULT STDMETHODCALLTYPE CVCamStream::Disconnect( void)
{
	return CSourceStream::Disconnect( );
}

HRESULT STDMETHODCALLTYPE CVCamStream::ConnectionMediaType(__out  AM_MEDIA_TYPE *pmt)
{
	return CSourceStream::ConnectionMediaType(pmt);
}

HRESULT STDMETHODCALLTYPE CVCamStream::QueryDirection(__out  PIN_DIRECTION *pPinDir)
{
	return CSourceStream::QueryDirection(pPinDir);
}

HRESULT STDMETHODCALLTYPE CVCamStream::QueryId(__out  LPWSTR *Id)
{
	return CSourceStream::QueryId(Id);
}

HRESULT STDMETHODCALLTYPE CVCamStream::QueryAccept(const AM_MEDIA_TYPE *pmt)
{
	return CSourceStream::QueryAccept(pmt);
}

HRESULT STDMETHODCALLTYPE CVCamStream::QueryInternalConnections(__out_ecount_part_opt(*nPin, *nPin) IPin **apPin, ULONG *nPin)
{
	return CSourceStream::QueryInternalConnections(apPin, nPin);
}

HRESULT STDMETHODCALLTYPE CVCamStream::EndOfStream( void)
{
	return CSourceStream::EndOfStream( );
}

HRESULT STDMETHODCALLTYPE CVCamStream::BeginFlush( void)
{
	return CSourceStream::BeginFlush( );
}

HRESULT STDMETHODCALLTYPE CVCamStream::EndFlush( void)
{
	return CSourceStream::EndFlush( );
}

HRESULT STDMETHODCALLTYPE CVCamStream::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	return CSourceStream::NewSegment(tStart,tStop, dRate);
}

HRESULT CVCamStream::Active(void)  {
	return CSourceStream::Active();
}

