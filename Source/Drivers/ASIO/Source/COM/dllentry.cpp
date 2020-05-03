//==========================================================================;
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1992 - 1996  Microsoft Corporation.  All Rights Reserved.
//
//--------------------------------------------------------------------------;

//
// classes used to support dll entrypoints for COM objects.
//
// #include "switches.h"
#include "..\main.h"

#include "combase.h"
#include "com_factory.h"
#include "wxdebug.h"

#ifdef DEBUG
#include <tchar.h>
#endif

extern int						g_cTemplates;
extern CFactoryTemplate			g_Templates[];

DWORD							g_amPlatform;		// VER_PLATFORM_WIN32_WINDOWS etc... (from GetVersionEx)
OSVERSIONINFO					g_osInfo;

// --- COM entrypoints -----------------------------------------
// DllRegisterServer


//called by COM to get the class factory object for a given class
extern "C" STDAPI DllGetClassObject(REFCLSID rClsID,REFIID riid,void **pv)
{
#ifdef ASIO_LOGGER
//Create logger.
	if ( pLogger == NULL )
	{
		pLogger = new logger_c();
	}

//Function start.
	Procedure_Start_Global();
#endif

	if ( !(riid == IID_IUnknown) && !(riid == IID_IClassFactory))
	{
		return E_NOINTERFACE;
	}

//Traverse the array of templates looking for one with this class id
	for (int i = 0; i < g_cTemplates; i++)
	{
		const CFactoryTemplate * pT = &g_Templates[i];
		if (pT->IsClassID(rClsID))
		{
		//Found a template - make a class factory based on this template
			*pv = (LPVOID) (LPUNKNOWN) new CClassFactory(pT);
			if (*pv == NULL)
			{
				return E_OUTOFMEMORY;
			}
			((LPUNKNOWN)*pv)->AddRef();
			return NOERROR;
		}
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, TRUE );
#endif
	return CLASS_E_CLASSNOTAVAILABLE;
}

//Call any initialization routines
void DllInitClasses(BOOL bLoading)
{
//Traverse the array of templates calling the init routine if they have one.
	for ( int i = 0; i < g_cTemplates; i++ )
	{
		const CFactoryTemplate * pT = &g_Templates[i];
		if ( pT->m_lpfnInit != NULL )
		{
			(*pT->m_lpfnInit)( bLoading, pT->m_ClsID );
		}
	}
}

// called by COM to determine if this dll can be unloaded
// return ok unless there are outstanding objects or a lock requested
// by IClassFactory::LockServer
//
// CClassFactory has a static function that can tell us about the locks,
// and CCOMObject has a static function that can tell us about the active
// object count
extern "C" STDAPI DllCanUnloadNow()
{
#ifdef ASIO_LOGGER
//Create logger.
	if ( pLogger == NULL )
	{
		pLogger = new logger_c();
	}

//Function start.
	Procedure_Start_Global();
#endif

	HRESULT		Status;
	if ( CClassFactory::IsLocked() || CBaseObject::ObjectsActive() )
	{
		Status = S_FALSE;
	}
	else
	{
		Status = S_OK;
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, TRUE );
#endif
	return Status;
}


// --- standard WIN32 entrypoints --------------------------------------
//extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);
//BOOL WINAPI DllEntryPoint(HINSTANCE hInstance,ULONG ulReason,LPVOID pv)
extern "C" BOOL WINAPI DllMain (HINSTANCE hInstance,ULONG ulReason,LPVOID pv)
//extern "C" BOOL WINAPI DllEntryPoint (HINSTANCE hInstance,ULONG ulReason,LPVOID pv)
{
#ifdef ASIO_LOGGER
//Create logger.
	if ( pLogger == NULL )
	{
		pLogger = new logger_c();
	}

//Function start.
	Procedure_Start_Global();
#endif

	switch (ulReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls(hInstance);
		DbgInitialise(hInstance);

		{
		// The platform identifier is used to work out whether full unicode support is available or not.
		//Hence the default will be the lowest common denominator - i.e. N/A
			g_amPlatform = VER_PLATFORM_WIN32_WINDOWS; // win95 assumed in case GetVersionEx fails

			g_osInfo.dwOSVersionInfoSize = sizeof(g_osInfo);
			if (GetVersionEx(&g_osInfo))
			{
				g_amPlatform = g_osInfo.dwPlatformId;
			}
			else
			{
				DbgLog((LOG_ERROR, 1, "Failed to get the OS platform, assuming Win95"));
			}
		}
		DllInitClasses(TRUE);
		break;
	}
	case DLL_PROCESS_DETACH:
	{
		DllInitClasses(FALSE);
		break;
	}
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, TRUE );
#endif
	return TRUE;
}


