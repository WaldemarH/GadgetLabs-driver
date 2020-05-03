#include "main.h"
#include "logger.h"


#ifdef ASIO_LOGGER

__int32 logger_c::GetDriverTime( IN OUT char* pBuffer, unsigned long sizeof_buffer )
{
	SYSTEMTIME		currentTime;

	GetSystemTime( &currentTime );

	return sprintf_s(
	  pBuffer,
	  sizeof_buffer,
	  "%d:%d:%d:%d (h:m:s:ms)",
	  currentTime.wHour,
	  currentTime.wMinute,
	  currentTime.wSecond,
	  currentTime.wMilliseconds
	);
}

#endif //ASIO_LOGGER
