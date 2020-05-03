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
// MODULE:
//		logger.cpp
//
// PURPOSE:
//		File contains logger procedures that are needed for saving log strings
//		to log file.
//
// STATUS:
//		Unfinished/Untested.
//
//***************************************************************************
//***************************************************************************
#include "main.h"


#ifdef DBG_AND_LOGGER

#pragma DATA_LOCKED


//***************************************************************************
// ProcedureName:
//		Log
//
// Explanation:
//		Procedure saves the requested text to log file.
//
//		This are really 2 procedures.
//		First acts as Printf and
//		second will call special logging functions that saves big procedure log texts.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// Synhronization:
//		None
//
// --------------------------------------------------------------------------
// 1st
// string: IN
//		Pointer to string requested for save.
//
// 2nd:
// string: IN
//		Pointer to string requested for save.
//
// ...: IN
//		Procedure supplied variables that are used in "printf" function.
//
// 3rd:
// select: IN
//		Constant that defines which predefined log function should be used.
//
// ...: IN
//		Procedure supplied variables that are needed in predefined function.
//
// --------------------------------------------------------------------------
// Return:
//		None.
//
#pragma CODE_LOCKED
void __stdcall logger_c::Log
(
	IN const char*		pText,
	IN ...
)
{
	char*		pBuffer;
	va_list		va;

//Get temporary buffer.
	pBuffer = (char*) ExAllocatePoolWithTag( NonPagedPool, 6*PAGE_SIZE, MEMORY_TAG_LOGGER_TEMP );
	if ( pBuffer == NULL )
	{
	//Print/Save string.
	#if defined( DBG_PRINT )
		DbgPrint( "ERROR in Logger_Print (STATUS_INSUFFICIENT_RESOURCES)" );
	#endif
	#if defined ( GL_LOGGER )
		Save( "ERROR in Logger_Print (STATUS_INSUFFICIENT_RESOURCES)" );
	#endif
		return;
	}

//Get string variables and create the string.
	va_start( va, pText );
	vsprintf( pBuffer, pText, va );

//Print/Save string.
//
//To see DbgPrint messages create/open:
//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Debug Print Filter
//and create DWORD: "DEFAULT" and set value to 0xF.
#if defined( DBG_PRINT )
	DbgPrint( pBuffer );
#endif
#if defined ( GL_LOGGER )
	Save( pBuffer );
#endif

//Free buffer.
	ExFreePoolWithTag( pBuffer, MEMORY_TAG_LOGGER_TEMP );

	return;
}

#pragma CODE_LOCKED
void __stdcall logger_c::Log
(
	IN LOGGER_SEND_TO		sendTo,
	IN const char*			pText,
	IN ...
)
{
	char*		pBuffer;
	va_list		va;

//Get temporary buffer.
	pBuffer = (char*) ExAllocatePoolWithTag( NonPagedPool, 6*PAGE_SIZE, MEMORY_TAG_LOGGER_TEMP );
	if ( pBuffer == NULL )
	{
	//Print/Save string.
	#if defined( DBG_PRINT )
		DbgPrint( "ERROR in Logger_Print (STATUS_INSUFFICIENT_RESOURCES)" );
	#endif
	#if defined ( GL_LOGGER )
		Save( "ERROR in Logger_Print (STATUS_INSUFFICIENT_RESOURCES)" );
	#endif
		return;
	}

//Get string variables and create the string.
	va_start( va, pText );
	vsprintf( pBuffer, pText, va );

//Print/Save string.
//
//To see DbgPrint messages create/open:
//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Debug Print Filter
//and create DWORD: "DEFAULT" and set value to 0xF.
	switch ( sendTo )
	{
	case LOGGER_SEND_TO_DEBUG_PRINT:
	{
	#if defined( DBG_PRINT )
		DbgPrint( pBuffer );
	#endif
		break;
	}
	case LOGGER_SEND_TO_LOG_FILE:
	{
	#if defined ( GL_LOGGER )
		Save( pBuffer );
	#endif
		break;
	}
	case LOGGER_SEND_TO_BOTH:
	{
	#if defined( DBG_PRINT )
		DbgPrint( pBuffer );
	#endif
	#if defined ( GL_LOGGER )
		Save( pBuffer );
	#endif
		break;
	}
	}

//Free buffer.
	ExFreePoolWithTag( pBuffer, MEMORY_TAG_LOGGER_TEMP );

	return;
}


#pragma CODE_LOCKED
void __stdcall logger_c::Log
(
	IN LOGGER_SEND_TO		sendTo,
	IN LOGGER_SELECT		selectProcedure,
	...
)
{
	char*		pBuffer;
	void**		ppVariables;
	va_list		va;

//Get temporary buffer.
	pBuffer = (char*) ExAllocatePoolWithTag( NonPagedPool, 6*PAGE_SIZE, MEMORY_TAG_LOGGER_TEMP );
	if ( pBuffer == NULL )
	{
	//Print/Save string.
	#if defined( DBG_PRINT )
		DbgPrint( "ERROR in Logger_Print (STATUS_INSUFFICIENT_RESOURCES)" );
	#endif
	#if defined ( GL_LOGGER )
		Save( "ERROR in Logger_Print (STATUS_INSUFFICIENT_RESOURCES)" );
	#endif
		return;
	}

//Call selected procedure.
	va_start( va, selectProcedure );
	ppVariables = (void**)va;

	switch ( selectProcedure )
	{
	case LS_Card_AcquireResources: Logger_Card_AcquireResources( pBuffer, ppVariables ); break;
	case LS_Card_SetCardType: Logger_Card_SetCardType( pBuffer, ppVariables ); break;
	case LS_Driver_AVStream_Pin_Format_AudioIntersectHandler: Logger_Driver_AVStream_Pin_Format_AudioIntersectHandler( pBuffer, ppVariables ); break;
	case LS_Driver_AVStream_Pin_Format_SetDataFormat: Logger_Driver_AVStream_Pin_Format_SetDataFormat( pBuffer, ppVariables ); break;
	case LS_Driver_AVStream_Pin_SetDeviceState: Logger_Driver_AVStream_Pin_SetDeviceState( pBuffer, ppVariables ); break;
	case LS_Driver_AVStream_Start: Logger_Driver_AVStream_Start( pBuffer, ppVariables ); break;
	case LS_IOCTL_ASIO_Start: Logger_IOCTL_ASIO_Start( pBuffer, ppVariables ); break;
	case LS_IOCTL_DATA_CardsInformation: Logger_IOCTL_Data_CardsInformation( pBuffer, ppVariables ); break;
	case LS_IOCTL_GUI_Notification_GetEvent: Logger_IOCTL_Gui_Notification_GetEvent( pBuffer, ppVariables ); break;
	}

//Print/Save string.
	switch ( sendTo )
	{
	case LOGGER_SEND_TO_DEBUG_PRINT:
	{
	#if defined( DBG_PRINT )
		DbgPrint( pBuffer );
	#endif
		break;
	}
	case LOGGER_SEND_TO_LOG_FILE:
	{
	#if defined ( GL_LOGGER )
		Save( pBuffer );
	#endif
		break;
	}
	case LOGGER_SEND_TO_BOTH:
	{
	#if defined( DBG_PRINT )
		DbgPrint( pBuffer );
	#endif
	#if defined ( GL_LOGGER )
		Save( pBuffer );
	#endif
		break;
	}
	}

//Free buffer.
	ExFreePoolWithTag( pBuffer, MEMORY_TAG_LOGGER_TEMP );

	return;
}

//***************************************************************************
// ProcedureName:
//		Log_Procedure
//
// Explanation:
//		Procedure is inteded for construction of procedure start and exit
//		string -> i.e. it is called at start/exit of the procedure to
//		create borders in the log file and to define the procedure name
//		and exit status.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// Synhronization:
//		None
//
// --------------------------------------------------------------------------
// procedureName: IN
//		Defines procedure name.
//
// isGlobalProcedure: IN
//		Defines if procedure is a global procedure or not.
//		If it is a global procedure then a border line will be added before
//		the procedure name.
//
//		i.e.
//		********************...
//		procedure name...
//
// exit: IN
//		Defines if this is a start or exit part of the procedure.
//		If it is called at exit the procedure will add return Status
//		to the line.
//
// Status: IN
//		Return status of the procedure.
//		Used only if exit is set to true.
//
//
// --------------------------------------------------------------------------
// Return:
//		None.
//
#pragma CODE_LOCKED
void logger_c::Log_Procedure
(
	IN char*			pProcedureName,
	IN bool				isGlobalProcedure,
	IN bool				exit,
	IN unsigned long	Status
)
{
	char*		pBuffer;
	int			offset;

//Get temporary buffer.
	pBuffer = (char*) ExAllocatePoolWithTag( NonPagedPool, BUFFER_SIZE_TEXT, MEMORY_TAG_LOGGER_TEMP );
	if ( pBuffer == NULL )
	{
	//Print/Save string.
	#if defined( DBG_PRINT )
		DbgPrint( "ERROR in Log_Procedure (STATUS_INSUFFICIENT_RESOURCES)" );
	#endif
	#if defined ( GL_LOGGER )
		Save( "ERROR in Log_Procedure (STATUS_INSUFFICIENT_RESOURCES)" );
	#endif
		return;
	}

//Get current time.
	char		bufferTime[128];

	GetDriverTime( bufferTime, TIMESTAMP_TYPE_SHORT );

//Create the string.
	if ( exit == FALSE )
	{
	//Procedure start.
		if ( isGlobalProcedure == TRUE )
		{
			offset = sprintf( pBuffer, PROCEDURE_BORDER );
		}
		else
		{
			offset = 0;
		}

		sprintf( pBuffer+offset, "%s -> Start (%s)\n", pProcedureName, bufferTime );
	}
	else
	{
	//Procedure exit.
		switch ( Status )
		{
		case STATUS_SUCCESS :
			sprintf( pBuffer, "%s -> return=STATUS_SUCCESS (%s)\n", pProcedureName, bufferTime );
			break;
		case STATUS_BUFFER_OVERFLOW :
			sprintf( pBuffer, "%s -> return=STATUS_BUFFER_OVERFLOW (%s)\n", pProcedureName, bufferTime );
			break;
		case STATUS_UNSUCCESSFUL :
			sprintf( pBuffer, "%s -> return=STATUS_UNSUCCESSFUL (%s)\n", pProcedureName, bufferTime );
			break;
		case STATUS_NO_MATCH :
			sprintf( pBuffer, "%s -> return=STATUS_NO_MATCH (%s)\n", pProcedureName, bufferTime );
			break;
		default :
			sprintf( pBuffer, "%s -> return=%X (%s)\n", pProcedureName, Status, bufferTime );
			break;
		}
	}

//Print/Save string.
#if defined( DBG_PRINT )
	DbgPrint( pBuffer );
#endif
#if defined ( GL_LOGGER )
	Save( pBuffer );
#endif

//Free buffer.
	ExFreePoolWithTag( pBuffer, MEMORY_TAG_LOGGER_TEMP );

	return;
}

//***************************************************************************
// ProcedureName:
//		Save
//
// Explanation:
//		Procedure saves the requested text to log file.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// Synhronization:
//		None
//
// --------------------------------------------------------------------------
// pBuffer: IN
//		Pointer to string requested for save.
//
// --------------------------------------------------------------------------
// Return:
//		None.
//

#if defined ( GL_LOGGER )
#pragma CODE_LOCKED
void logger_c::Save
(
	IN char*			pBuffer
)
{
	OBJECT_ATTRIBUTES	objectAttributes;

	HANDLE				fileHandle = NULL;
	IO_STATUS_BLOCK		ioStatusBlock;

	NTSTATUS			Status = STATUS_SUCCESS;

//Can we log?
	if ( m_CanLog == NO )
		return;

	//Are we at PASSIVE_LEVEL?
	if ( KeGetCurrentIrql() > PASSIVE_LEVEL )
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//Acquire logger mutex.
	if ( SUCCESS( Status ) )
	{
		Status = KeWaitForSingleObject(
		  (void*)&m_Mutex_logger,
		  Executive,
		  KernelMode,
		  FALSE,
		  NULL
		);
	}

//Save string.
	if ( SUCCESS( Status ) )
	{
	//Open logging file.
		InitializeObjectAttributes(
		  &objectAttributes,
		  &m_LoggingFilePath,
		  OBJ_CASE_INSENSITIVE,
		  NULL,
		  NULL
		);

		//Direct disk access.
		Status = ZwCreateFile(
		  &fileHandle,
		  FILE_WRITE_DATA | SYNCHRONIZE,
		  &objectAttributes,
		  &ioStatusBlock,
		  NULL,
		  FILE_ATTRIBUTE_NORMAL,
		  FILE_SHARE_READ,
		  FILE_OPEN,
		  FILE_NO_INTERMEDIATE_BUFFERING | FILE_SYNCHRONOUS_IO_NONALERT,
		  NULL,
		  0
		);

		//Extra check.
		if ( fileHandle == NULL )
		{
			Status = STATUS_UNSUCCESSFUL;
		}

	//Append buffer to disk buffer and save.
		if ( SUCCESS( Status ) )
		{
		//Append string to main buffer.
			strcpy( m_pFileBuffer_current, pBuffer );

		//Write main buffer to disk.
			//Direct disk access.
			ZwWriteFile(
			  fileHandle,
			  NULL,
			  NULL,
			  NULL,
			  &ioStatusBlock,
			  m_pFileBuffer_start,
			  BUFFER_SIZE_FILE,
			  &m_File_byteOffset,
			  NULL
			);

			ZwClose( fileHandle );

		//Check if we are over 2 page boundary.
			//Increase the pointer for size of the new string.
			m_pFileBuffer_current += strlen( pBuffer );

			//Are we over?
			while ( (m_pFileBuffer_current - m_pFileBuffer_start) >= 2*PAGE_SIZE )
			{
			//Copy upper n pages to lower 2 and reSet the upper 2.
				RtlCopyMemory(
				  m_pFileBuffer_start,
				  m_pFileBuffer_start + 2*PAGE_SIZE,
				  BUFFER_SIZE_FILE - 2*PAGE_SIZE
				);
				RtlFillMemory(
				  m_pFileBuffer_start + ( BUFFER_SIZE_FILE - 2*PAGE_SIZE ),
				  2*PAGE_SIZE,
				  ' '	//'\x0A'		//new line
				);

			//Update variables.
				//The upper n pages are now moved -> get position of '\x0'.
				m_pFileBuffer_current = m_pFileBuffer_start;
				m_pFileBuffer_current += strlen( m_pFileBuffer_start );

				//writing offset
				m_File_byteOffset.LowPart += 2*PAGE_SIZE;
			}
		}

	//Release logger mutex.
		KeReleaseMutex( &m_Mutex_logger, FALSE );
	}
	
	return;
}
#endif //GL_LOGGER

#endif	//DBG_AND_LOGGER
