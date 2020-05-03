#include "main.h"
#include "application.h"
#include "registry.h"


BOOL CApplication::IsInstallationRequest()
{
//Is it installation request?
	CString		commandLine = m_lpCmdLine;

	int			index_32bit = commandLine.Find( "-start_GUI:gadgetlabs_gui_32.exe", 0 );
	int			index_64bit = commandLine.Find( "-start_GUI:gadgetlabs_gui_64.exe", 0 );

	if ( 
	  ( index_32bit > -1 )
	  ||
	  ( index_64bit > -1 )
	)
	{
	//Yes -> start GUI.
		//Initialize variables.
		PROCESS_INFORMATION			processInformation;
		STARTUPINFO					startUpInfo;

		ZeroMemory( &processInformation, sizeof( processInformation ) );
		ZeroMemory( &startUpInfo, sizeof( startUpInfo ) );

		startUpInfo.cb = sizeof(startUpInfo);

		//Start process.
		BOOL	Status = CreateProcess(
		  0,
		  ( index_32bit > -1 ) ? _T( "gadgetlabs_gui_32.exe" ) : _T( "gadgetlabs_gui_64.exe" ),
		  0,
		  0,
		  FALSE,
		  0,
		  0,
		  0,
		  &startUpInfo,
		  &processInformation
		);
		if ( Status != FALSE )
		{
		//Process started...
			CloseHandle( processInformation.hProcess );
			CloseHandle( processInformation.hThread );
		}

	//Check if the old driver's ASIO key still exists and delete it if it does.
		registry_c		registry;

		registry.DeleteKey( HKEY_LOCAL_MACHINE, _T( "Software\\ASIO\\GadgetLabs ASIO Driver" ) );

		return TRUE;
	}
	else
	{
	//Not installation request...
		return FALSE;
	}
}
