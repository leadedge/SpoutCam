#pragma once								

#define _VER_BUILDNUM 0							
#define _VER_BUILDNUM_STRING L"0"					

#define _VER_COUNTRY _VER_COUNTRY_AUS			

#define _VER_MAJORVERSION 2				    
#define _VER_MAJORVERSION_STRING L"2"

#define _VER_MINORVERSION 1
#define _VER_MINORVERSION_STRING L"024"     
														
#define _VER_BUGFIXVERSION 0
#define _VER_BUGFIXVERSION_STRING L"0"

#if _VER_BUGFIXVERSION > 0
	#define	_VER_VERSION_STRING	_VER_MAJORVERSION_STRING L"." _VER_MINORVERSION_STRING "." _VER_BUGFIXVERSION_STRING 
#else
	#define	_VER_VERSION_STRING	_VER_MAJORVERSION_STRING L"." _VER_MINORVERSION_STRING 
#endif

// following are used in version info for the windows resource 
#define _VER_ORIGINALFILENAME	L"SpoutCam.ax"
#define _VER_COMPANY			L"Leading Edge Pty Ltd"
#define _VER_FILEDESCRIPTION	L"SpoutCam virtual webcam DirectShow Filter"
#define _VER_PRODUCTNAME		L"SpoutCam"
#define _VER_INTERNALNAME		L"SpoutCam.ax"

#define _VER_COPYRIGHT L"Copyright (c) Lynn Jarvis 2020"
#define _VER_YEAR		L"2020"
