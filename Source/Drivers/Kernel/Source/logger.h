//***************************************************************************
//***************************************************************************
//
//                          PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license agreement or
// nondisclosure agreement with Waldemar Haszlakiewicz and may not be
// copied or disclosed except in accordance with the terms of that agreement.
//
//               Copyright (C) 2010 Waldemar Haszlakiewicz
//                            All Rights Reserved.
//
//***************************************************************************
//***************************************************************************
//
// PURPOSE:
//		File contains definitions for logger and debugPrint functions.
//
//***************************************************************************
//***************************************************************************
#pragma once


#if defined( DBG_PRINT )
	#define DBG_AND_LOGGER = 1
#elif defined ( GL_LOGGER )
	#define DBG_AND_LOGGER = 1
#endif

#ifdef DBG_AND_LOGGER

#define Procedure_Start_Global()		pLogger->Log_Procedure( __FUNCTION__, true, false, 0 )
#define Procedure_Start_Local()			pLogger->Log_Procedure( __FUNCTION__, false, false, 0 )
#define Procedure_Exit( Status )		pLogger->Log_Procedure( __FUNCTION__, false, true, Status )

typedef union
{
	PKSDATAFORMAT_DSOUND				DX;
	PKSDATAFORMAT_WAVEFORMATEXTENSIBLE	MME_WDM;
} KSDATAFORMATS;

#define DRV_MME_WDM					0
#define DRV_DX						1

class card_c;
class channel_all_c;
class channel_base_c;
class channel_stereo_c;

class logger_c
{
//As this is a global class we'll simulate constructor and destructor.
	public: logger_c(void);
	public: ~logger_c(void);

//Class memory
	#define MEMORY_TAG_LOGGER_CLASS		'LCGL'	//tag: "LGCL"
	#define MEMORY_TAG_LOGGER_TEXT		'XTGL'	//tag: "LGTX"
	#define MEMORY_TAG_LOGGER_TEMP		'MTGL'	//tag: "LGTM"
	#define MEMORY_TAG_LOGGER_FAST		'TFGL'	//tag: "LGFT"

	public: static void* __cdecl operator new( size_t size );
	public: static void __cdecl operator delete( void* pMemory );

//Log
	public: void Log_Procedure( IN char* pProcedureName, IN bool isGlobalProcedure, IN bool exit, IN unsigned long Status );
	#define PROCEDURE_BORDER "-------------------------------------------------------------------------\n"

	//Logger select numbers are alphabetics representation of procedure names (look down and you will know).
	//Do not reorder -> well if you do you have to reorder loggingProcedures table too.
	public: enum LOGGER_SELECT
	{
		LS_Card_AcquireResources,
		LS_Card_SetCardType,
		LS_Driver_AVStream_Pin_Format_AudioIntersectHandler,
		LS_Driver_AVStream_Pin_Format_SetDataFormat,
		LS_Driver_AVStream_Pin_SetDeviceState,
		LS_Driver_AVStream_Start,
		LS_IOCTL_ASIO_Start,
		LS_IOCTL_DATA_CardsInformation,
		LS_IOCTL_GUI_Notification_GetEvent

		//LS_DeviceDispatch_Create,
		//LS_DeviceDispatch_Control,
		//LS_DeviceDispatch_Start2,
		//LS_GetPinSyncStartStructure_1,
		//LS_GetPinSyncStartStructure_2,
		//LS_InitializeGLValuesFromRegistry,
		//LS_IoctlAsio_OpenCloseChannels,
		//LS_Pin_Close_1,
		//LS_Pin_Close_2,
		//LS_Pin_Create_1,
		//LS_Pin_Create_2,
		//LS_Pin_Create_3,
		//LS_ReleaseSyncStartStructure_1,
		//LS_ReleaseSyncStartStructure_2,
		//LS_SetChannelPosition
	};

	public: enum LOGGER_SEND_TO
	{
		LOGGER_SEND_TO_DEBUG_PRINT,
		LOGGER_SEND_TO_LOG_FILE,
		LOGGER_SEND_TO_BOTH
	};

	public: void __stdcall Log( IN const char* pString, IN ... );
	public: void __stdcall Log( IN LOGGER_SEND_TO sendTo, IN const char* pString, IN ... );
	public: void __stdcall Log( IN LOGGER_SEND_TO sendTo, IN LOGGER_SELECT selectProcedure, IN ... );

	private: void Logger_Card_AcquireResources( IN char* pBuffer, IN void** ppVariables );
	private: void Logger_Card_SetCardType( IN char* pBuffer, IN void** ppVariables );
	private: char* Logger_Driver_AVStream_Pin_Format_DataFormat( char* pBuffer, char* pFormatName, KSDATAFORMATS dataFormat, unsigned long typeDriver );
	private: char* Logger_Driver_AVStream_Pin_Format_DataRange( char* pBuffer, char* pDataRangeName, PKSDATARANGE_AUDIO pDataRange );
	private: void Logger_Driver_AVStream_Pin_Format_AudioIntersectHandler( IN char* pBuffer, IN void** ppVariables );
	private: void Logger_Driver_AVStream_Pin_Format_SetDataFormat( IN char* pBuffer, IN void** ppVariables );
	private: void Logger_Driver_AVStream_Pin_SetDeviceState( IN char* pBuffer, IN void** ppVariables );
	private: void Logger_Driver_AVStream_Start( IN char* pBuffer, IN void** ppVariables );
	private: void Logger_IOCTL_ASIO_Start( IN char* pBuffer, IN void** ppVariables );
	private: void Logger_IOCTL_Data_CardsInformation( IN char* pBuffer, IN void** ppVariables );
	private: void Logger_IOCTL_Gui_Notification_GetEvent( IN char* pBuffer, IN void** ppVariables );

	private: char* Logger_Card_Class( char* pBuffer, card_c* pCard );
	private: char* Logger_Channel_All_Class( char* pBuffer, channel_all_c* pChannel );
	private: char* Logger_Channel_Base_Class( char* pBuffer, channel_base_c* pChannel );
	private: char* Logger_Channel_Stereo_Class( char* pBuffer, channel_stereo_c* pChannel );

//TimeStamp
	private: LARGE_INTEGER				m_TimeStamp_driverStart;

	private: enum TIMESTAMP_TYPE
	{
		TIMESTAMP_TYPE_SHORT,
		TIMESTAMP_TYPE_LONG
	};
	private: __int32 GetDriverTime( IN OUT char* pBuffer, IN TIMESTAMP_TYPE type );

#if defined ( GL_LOGGER )
//Logging file
	#define MAX_LOG_FILES				0x40

	private: UNICODE_STRING				m_LoggingFilePath;
	private: WCHAR						m_LoggingFilePath_container[40];

	//Logger will preallocate 8*PAGE_SIZE nonPaged(PrintDbg uses it also) buffer even if we will
	//use only the first 6 pages. In this way we can be sure that we wont write over the end of
	//the buffer.
	#define BUFFER_SIZE_TEXT			6*PAGE_SIZE
	#define BUFFER_SIZE_FILE			8*PAGE_SIZE

	private: char*						m_pFileBuffer_start;
	private: char*						m_pFileBuffer_current;
	private: LARGE_INTEGER				m_File_byteOffset;

	//Save.
	private: void __stdcall Save( IN char* pString );

//Fast logging...
	public: void __stdcall LogFast( IN const char* pText, IN ... );
	public: void LogFast_Reset();
	public: void LogFast_Save();

	private: char*						m_pLogFast;
	private: unsigned long				m_LogFast_characterPresent;
	public: unsigned long				m_LogFast_save;	//YES/NO
	#define LOGGER_FAST_BUFFER_SIZE		0x100000		//1MB

//Synchronization
	private: __int32					m_CanLog;			//this will be set only at start, so it will not be protected with synhronization
	private: KMUTEX						m_Mutex_logger;

#endif //GL_LOGGER
};
extern logger_c*		pLogger;

#endif
