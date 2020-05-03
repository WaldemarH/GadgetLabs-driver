#include "main.h"
#include "application.h"


BOOL CApplication::Mutex_Lock()
{
//Lock mutex.
//
//Notice:
//Mutex (kernel object) is opened with unique ID so we can be sure it's the only one in the system.
	m_hMutex = CreateMutex( NULL, FALSE, _T( CPA_MUTEX_NAME ) );

//Check if current application is already started.
	DWORD	lastError = GetLastError();
	if (
	  ( lastError == ERROR_ALREADY_EXISTS )
	  ||
	  ( lastError == ERROR_ACCESS_DENIED )
	)
	{
	//Mutex is already opened -> GUI is already opened -> ignore open GUI request.
		return FALSE;
	}

	return TRUE;
}

void CApplication::Mutex_Unlock()
{
	if ( m_hMutex != NULL )
	{
		CloseHandle( m_hMutex );
		m_hMutex = NULL;
	}
}
