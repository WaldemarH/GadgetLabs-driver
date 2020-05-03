#pragma once

#ifdef ASIO_LOGGER
//Includes...


#define Procedure_Start_Global()						pLogger->Log_Procedure( __FUNCTION__, true, false, 0, -1 )
#define Procedure_Start_Local()							pLogger->Log_Procedure( __FUNCTION__, false, false, 0, -1 )
#define Procedure_Exit( Status_Error, Status_Bool )		pLogger->Log_Procedure( __FUNCTION__, false, true, (ASIOError)Status_Error, (ASIOBool)Status_Bool )

class logger_c
{
//Constructor/Destructor
	public: logger_c();
	public: ~logger_c();

	#define MAX_LOG_FILES				0x40

//Log
	public: void Log_Procedure( IN char* pProcedureName, IN bool isGlobalProcedure, IN bool exit, IN ASIOError Status_Error, IN ASIOBool Status_Bool );
	#define PROCEDURE_BORDER "-------------------------------------------------------------------------\n"

	public: enum LOGGER_SELECT
	{
		LS_Channels_Open,
		LS_Channels_Open2,
		LS_CreateBuffers,
		LS_Future,
		LS_GetChannelInfo
	};

	public: void __stdcall Log( IN const char* pString, IN ... );
	public: void __stdcall Log( IN LOGGER_SELECT selectProcedure, IN ... );

	private: void Logger_Channels_Open( IN char* pBuffer, IN void** ppVariables );
	private: void Logger_Channels_Open2( IN char* pBuffer, IN void** ppVariables );
	private: void Logger_CreateBuffers( IN char* pBuffer, IN void** ppVariables );
	private: void Logger_Future( IN char* pBuffer, IN void** ppVariables );
	private: void Logger_GetChannelInfo( IN char* pBuffer, IN void** ppVariables );

//Logging file
	private: char						m_LoggingFilePath[MAX_PATH];

	#define PAGE_SIZE					0x1000
	#define BUFFER_SIZE_TEXT			6*PAGE_SIZE
	#define BUFFER_SIZE_FILE			8*PAGE_SIZE

	private: void __stdcall Save( IN char* pString );

//Synchronization
	private: __int32					m_CanLog;			//this will be set only at start, so it will not be protected with synhronization
	private: CRITICAL_SECTION			m_CriticalSection_logger;

//Time
	private: __int32 GetDriverTime( IN OUT char* pBuffer, unsigned long sizeof_buffer );
};
extern logger_c*		pLogger;

#endif //ASIO_LOGGER
