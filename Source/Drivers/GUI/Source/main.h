
#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN			//Exclude rarely-used stuff from Windows headers
#endif

#ifndef WINVER
#define WINVER 0x0501			//XP+
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501		//XP+
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0501	//XP+
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0501		//Change this to the appropriate value to target other versions of IE.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS		//some CString constructors will be explicit
#define _AFX_ALL_WARNINGS						//turns off MFC's hiding of some common and often safely ignored warning messages
#include <afxwin.h>								// MFC core and standard components
#include <afxext.h>								// MFC extensions
#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>							//MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>								//MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <afxcontrolbars.h>						//MFC support for ribbons and control bars

#include <string.h>
#include "..\Resources\resource.h"

#define ELEMENTS_IN_ARRAY( a )		( sizeof(a)/sizeof(a[0]) )

extern const UINT	GWM_REFRESH;
