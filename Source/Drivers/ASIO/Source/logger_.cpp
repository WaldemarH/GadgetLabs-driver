#include "main.h"
#include "logger.h"

#ifdef ASIO_LOGGER
logger_c*		pLogger;

logger_c::logger_c()
{
//Synhronization.
	m_CanLog = FALSE;

	InitializeCriticalSection( &m_CriticalSection_logger );

//Check if loggerX.txt is present->create it if it isn't.
	for ( int i = 0; i < MAX_LOG_FILES; i++ )
	{
	//Create string.
		sprintf_s( m_LoggingFilePath, sizeof(m_LoggingFilePath), "c:\\ASIO_logger%d.txt", i );

		HANDLE		hFile = CreateFile(
		  m_LoggingFilePath,
		  GENERIC_WRITE,
		  0,
		  0,
		  CREATE_NEW,
		  FILE_ATTRIBUTE_ARCHIVE,
		  0
		);
		if ( hFile != INVALID_HANDLE_VALUE )
		{
		//Success -> a new file was created.
			CloseHandle( hFile );

			m_CanLog = TRUE;
			break;
		}
	}
}

#endif //ASIO_LOGGER
