#pragma once								

#define _VER_BUILDNUM 0							
#define _VER_BUILDNUM_STRING "0"					

#define _VER_COUNTRY _VER_COUNTRY_AUS			

#define _VER_MAJORVERSION 2				    
#define _VER_MAJORVERSION_STRING "2"

#define _VER_MINORVERSION 1
#define _VER_MINORVERSION_STRING ".015"     
														
#define _VER_BUGFIXVERSION 0
#define _VER_BUGFIXVERSION_STRING "0"

#if _VER_BUGFIXVERSION > 0
	#define	_VER_VERSION_STRING	_VER_MAJORVERSION_STRING "." _VER_MINORVERSION_STRING "." _VER_BUGFIXVERSION_STRING 
#else
	#define	_VER_VERSION_STRING	_VER_MAJORVERSION_STRING "." _VER_MINORVERSION_STRING 
#endif

// following are used in version info for the windows resource 
#define _VER_ORIGINALFILENAME	"SpoutCam.ax"
#define _VER_COMPANY			"Leading Edge Pty Ltd"
#define _VER_FILEDESCRIPTION	"SpoutCam virtual webcam DirectShow Filter"
#define _VER_PRODUCTNAME		"SpoutCam"
#define _VER_INTERNALNAME		"SpoutCam.ax"

#define _VER_COPYRIGHT "Copyright (c) Lynn Jarvis 2020"
#define _VER_YEAR		"2020"
