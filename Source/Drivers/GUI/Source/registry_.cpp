#include "main.h"
#include "registry.h"

registry_c::registry_c()
{
}

BOOL registry_c::Read( registry_data_s* pData )
{
	BOOL	Status = TRUE;

//Open registry key.
	HKEY		hKey_GadgetLabs;

	if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE, REGISTRY_GL_ROOT_PATH, 0, KEY_ALL_ACCESS, &hKey_GadgetLabs ) )
	{
	//Read 
		DWORD	variableSize = sizeof( registry_data_s );

		if ( ERROR_SUCCESS != RegQueryValueEx( hKey_GadgetLabs, REGISTRY_DATA, 0, NULL, (LPBYTE)pData, &variableSize ) )
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

	return Status;
}

BOOL registry_c::Save( registry_data_s* pData )
{
	BOOL	Status = TRUE;

//Open/Create registry key.
	DWORD		disposition;
	HKEY		hKey_GadgetLabs;

	if ( ERROR_SUCCESS == RegCreateKeyEx( HKEY_LOCAL_MACHINE, REGISTRY_GL_ROOT_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey_GadgetLabs, &disposition ) )
	{
	//Save
		if ( ERROR_SUCCESS != RegSetValueEx( hKey_GadgetLabs, REGISTRY_DATA, 0, REG_BINARY, (LPBYTE)pData, sizeof( registry_data_s ) ) )
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

	return Status;
}

BOOL registry_c::DeleteKey( HKEY hKey, LPCTSTR lpSubKey )
{
	BOOL	Status;

	if ( ERROR_SUCCESS != RegDeleteKey( hKey, lpSubKey ) )
	{
		Status = TRUE;
	}
	else
	{
		Status = FALSE;
	}

	return Status;
}
