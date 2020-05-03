#include "main.h"
#include "registry.h"


registry_c::registry_c()
{
}

BOOL registry_c::Read( registry_data_s* pData )
{
	BOOL	Status;

//Get host path... name.
	char	hostPath[1024];

	Status = GetHostPath( hostPath );

//Open registry key.
	if ( Status == TRUE )
	{
		HKEY		hKey_GadgetLabs;

		if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE, REGISTRY_GL_ASIO_PATH, 0, KEY_ALL_ACCESS, &hKey_GadgetLabs ) )
		{
		//Read 
			DWORD	variableSize = sizeof( registry_data_s );

			if ( ERROR_SUCCESS != RegQueryValueEx( hKey_GadgetLabs, hostPath, 0, NULL, (LPBYTE)pData, &variableSize ) )
			{
			//The value doesn't exist yet.
				Status = FALSE;
			}

		//Free resources.
			RegCloseKey( hKey_GadgetLabs );
		}
		else
		{
		//The key doesn't exist yet.
			Status = FALSE;
		}
	}

	return Status;
}

BOOL registry_c::Save( registry_data_s* pData )
{
	BOOL	Status = TRUE;

//Get host path... name.
	char	hostPath[1024];

	Status = GetHostPath( hostPath );

//Open/Create registry key.
	if ( SUCCESS( Status ) )
	{
		DWORD		disposition;
		HKEY		hKey_GadgetLabs;

		if ( ERROR_SUCCESS == RegCreateKeyEx( HKEY_LOCAL_MACHINE, REGISTRY_GL_ASIO_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey_GadgetLabs, &disposition ) )
		{
		//Save
			if ( ERROR_SUCCESS != RegSetValueEx( hKey_GadgetLabs, hostPath, 0, REG_BINARY, (LPBYTE)pData, sizeof( registry_data_s ) ) )
			{
			//The value doesn't exist yet.
				Status = FALSE;
			}

		//Free resources.
			RegCloseKey( hKey_GadgetLabs );
		}
		else
		{
		//The key couldn't be created/opened.
			Status = FALSE;
		}
	}

	return Status;
}

BOOL registry_c::GetHostPath( char hostPath[1024] )
{
	BOOL Status = TRUE;

//Get host (ASIO application) path... name.
	DWORD pathLength = GetModuleFileName( GetModuleHandle( NULL ), hostPath, 1024 /*sizeof( hostPath ) doesn't work for some reason*/ );
	if ( pathLength != 0 )
	{
	//Substitute '\\' to '@' in the path as '\' is not allowed in a registry name.
		char*		pHostPath = hostPath;

		while ( pHostPath[0] != 0 )
		{
			if ( hostPath[0] == '\\' )
			{
				hostPath[0] = '@';
			}

		//Next...
			++pHostPath;
		}
	}
	else
	{
		Status = FALSE;
	}

	return Status;
}
