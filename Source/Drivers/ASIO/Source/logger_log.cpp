#include "main.h"
#include "logger.h"


#ifdef ASIO_LOGGER

void __stdcall logger_c::Log
(
	IN const char*		pText,
	IN ...
)
{
//Get temporary buffer.
	char*		pBuffer = new char[ BUFFER_SIZE_TEXT ];
	if ( pBuffer == NULL )
	{
	//Print/Save string.
		Save( "ERROR in Log (STATUS_INSUFFICIENT_RESOURCES)" );
		return;
	}

//Get string variables and create the string.
	va_list		va;

	va_start( va, pText );
	vsprintf_s( pBuffer, BUFFER_SIZE_TEXT, pText, va );

//Save string.
#ifdef ASIO_PRINT
	OutputDebugString( pBuffer );
#endif
	Save( pBuffer );

//Free buffer.
	delete[] pBuffer;

	return;
}

void __stdcall logger_c::Log
(
	IN LOGGER_SELECT		selectProcedure,
	...
)
{
//Get temporary buffer.
	char*		pBuffer = new char[ BUFFER_SIZE_TEXT ];
	if ( pBuffer == NULL )
	{
	//Print/Save string.
		Save( "ERROR in Log (STATUS_INSUFFICIENT_RESOURCES)" );
		return;
	}

//Call selected procedure.
	va_list		va;
	va_start( va, selectProcedure );
	void**		ppVariables = (void**)va;

	switch ( selectProcedure )
	{
	case LS_Channels_Open: Logger_Channels_Open( pBuffer, ppVariables ); break;
	case LS_Channels_Open2: Logger_Channels_Open2( pBuffer, ppVariables ); break;
	case LS_CreateBuffers: Logger_CreateBuffers( pBuffer, ppVariables ); break;
	case LS_Future: Logger_Future( pBuffer, ppVariables ); break;
	case LS_GetChannelInfo: Logger_GetChannelInfo( pBuffer, ppVariables ); break;
	}

//Save string.
#ifdef ASIO_PRINT
	OutputDebugString( pBuffer );
#endif
	Save( pBuffer );

//Free buffer.
	delete[] pBuffer;

	return;
}

void logger_c::Log_Procedure
(
	IN char*			pProcedureName,
	IN bool				isGlobalProcedure,
	IN bool				exit,
	IN ASIOError		Status_Error,
	IN ASIOBool			Status_Bool			//set to -1 if not used
)
{
//Get temporary buffer.
	char*		pBuffer = new char[ BUFFER_SIZE_TEXT ];
	if ( pBuffer == NULL )
	{
	//Print/Save string.
		Save( "ERROR in Log_Procedure (STATUS_INSUFFICIENT_RESOURCES)" );
		return;
	}

//Get current time.
	char		buffer_time[128];

	GetDriverTime( (char*)&buffer_time, sizeof( buffer_time ) );

//Create the string.
	if ( exit == FALSE )
	{
	//Procedure start.
		int			offset;

		if ( isGlobalProcedure == TRUE )
		{
			offset = sprintf_s( pBuffer, BUFFER_SIZE_TEXT, PROCEDURE_BORDER );
		}
		else
		{
			offset = 0;
		}

		sprintf_s( pBuffer+offset, BUFFER_SIZE_TEXT - offset, "%s -> Start (%s)\n", pProcedureName, buffer_time );
	}
	else
	{
	//Procedure exit.
		if ( Status_Bool == ASIOTrue )
		{
			sprintf_s( pBuffer, BUFFER_SIZE_TEXT, "%s -> return=ASIOTrue (%s)\n", pProcedureName, buffer_time );
		}
		else if( Status_Bool == ASIOFalse )
		{
			sprintf_s( pBuffer, BUFFER_SIZE_TEXT, "%s -> return=ASIOFalse (%s)\n", pProcedureName, buffer_time );
		}
		else
		{
			switch ( Status_Error )
			{
			case ASE_OK :
				sprintf_s( pBuffer, BUFFER_SIZE_TEXT, "%s -> return=ASE_OK (%s)\n", pProcedureName, buffer_time );
				break;
			case ASE_SUCCESS :
				sprintf_s( pBuffer, BUFFER_SIZE_TEXT, "%s -> return=ASE_SUCCESS (%s)\n", pProcedureName, buffer_time );
				break;
			case ASE_NotPresent :
				sprintf_s( pBuffer, BUFFER_SIZE_TEXT, "%s -> return=ASE_NotPresent (%s)\n", pProcedureName, buffer_time );
				break;
			case ASE_HWMalfunction :
				sprintf_s( pBuffer, BUFFER_SIZE_TEXT, "%s -> return=ASE_HWMalfunction (%s)\n", pProcedureName, buffer_time );
				break;
			case ASE_InvalidParameter :
				sprintf_s( pBuffer, BUFFER_SIZE_TEXT, "%s -> return=ASE_InvalidParameter (%s)\n", pProcedureName, buffer_time );
				break;
			case ASE_InvalidMode :
				sprintf_s( pBuffer, BUFFER_SIZE_TEXT, "%s -> return=ASE_InvalidMode (%s)\n", pProcedureName, buffer_time );
				break;
			case ASE_SPNotAdvancing :
				sprintf_s( pBuffer, BUFFER_SIZE_TEXT, "%s -> return=ASE_SPNotAdvancing (%s)\n", pProcedureName, buffer_time );
				break;
			case ASE_NoClock :
				sprintf_s( pBuffer, BUFFER_SIZE_TEXT, "%s -> return=ASE_NoClock (%s)\n", pProcedureName, buffer_time );
				break;
			case ASE_NoMemory :
				sprintf_s( pBuffer, BUFFER_SIZE_TEXT, "%s -> return=ASE_NoMemory (%s)\n", pProcedureName, buffer_time );
				break;
			default :
				sprintf_s( pBuffer, BUFFER_SIZE_TEXT, "%s -> return=%X (%s)\n", pProcedureName, Status_Error, buffer_time );
				break;
			}
		}
	}

//Save string.
#ifdef ASIO_PRINT
	OutputDebugString( pBuffer );
#endif
	Save( pBuffer );

//Free buffer.
	delete[] pBuffer;

	return;
}

void logger_c::Save
(
	IN char*	pBuffer
)
{
//Can we log?
	if ( m_CanLog == FALSE )
	{
	//We can't.
		return;
	}

//Enter critical section.
	EnterCriticalSection( &m_CriticalSection_logger );

//Open file handle.
	HANDLE		hFile = CreateFile( m_LoggingFilePath, GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, 0 );

//Set file offset and save buffer.
	DWORD		bytesWritten;

	SetFilePointer( hFile, 0, 0, FILE_END );
	WriteFile( hFile, pBuffer, (DWORD)strlen(pBuffer), &bytesWritten, 0 );

//Close handle.
	CloseHandle( hFile );

//Release critical section.
	LeaveCriticalSection( &m_CriticalSection_logger );
}

#endif //ASIO_LOGGER
