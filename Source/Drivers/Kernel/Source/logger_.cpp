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
logger_c*		pLogger;


//***************************************************************************
// ProcedureName: Constructor/Destructor
//
// Explanation:
//		Procedure will initialize/free variables of logger class.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// Synhronization:
//		None
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return Codes:
//		STATUS_SUCCESS or one of the error status values defined in ntstatus.h.
//
#pragma CODE_LOCKED
logger_c::logger_c()
{
	NTSTATUS			Status = STATUS_SUCCESS;

//Get current timeStamp (start of driver timeStamp).
	KeQuerySystemTime( &m_TimeStamp_driverStart );

#if defined ( GL_LOGGER )
//Try to create a new logging file.
	HANDLE				fileHandle;
	IO_STATUS_BLOCK		IoStatusBlock;

	OBJECT_ATTRIBUTES	ObjectAttributes;

	//Initialize.
	m_CanLog = NO;
	RtlInitUnicodeString( &m_LoggingFilePath, L"" );

	//Check if loggerX.txt is present->create it if it isn't.
	for ( int i = 0; i < MAX_LOG_FILES; i++ )
	{
	//Create logging file path.
		_snwprintf( m_LoggingFilePath_container, ELEMENTS_IN_ARRAY(m_LoggingFilePath_container), L"\\??\\C:\\glDriver_logger%d.txt", i );
		RtlInitUnicodeString( &m_LoggingFilePath, m_LoggingFilePath_container );

	//Try to create a new file.
		InitializeObjectAttributes(
		  &ObjectAttributes,
		  &m_LoggingFilePath,
		  OBJ_CASE_INSENSITIVE,
		  NULL,
		  NULL
		);

		Status = ZwCreateFile(
		  &fileHandle,
		  GENERIC_WRITE,
		  &ObjectAttributes,
		  &IoStatusBlock,
		  NULL,
		  FILE_ATTRIBUTE_NORMAL,
		  0,
		  FILE_CREATE,
		  FILE_SYNCHRONOUS_IO_NONALERT,
		  NULL,
		  0
		);

		if ( SUCCESS( Status ) )
		{
		//Close opened file and signal that we can log.
			ZwClose( fileHandle );
			m_CanLog = YES;
			break;
		}
	}

//Initialize text memory.
	m_pFileBuffer_start = NULL;

	if ( SUCCESS( Status ) )
	{
		m_pFileBuffer_start = (char*)ExAllocatePoolWithTag( NonPagedPool, BUFFER_SIZE_FILE, MEMORY_TAG_LOGGER_TEXT );
		if ( m_pFileBuffer_start != NULL )
		{
			m_File_byteOffset.QuadPart = 0;
			RtlFillMemory( (char*)m_pFileBuffer_start, BUFFER_SIZE_FILE, ' ' );		//space

			m_pFileBuffer_current = m_pFileBuffer_start;
		}
		else
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			m_CanLog = NO;
		}
	}

//Fast logging...
	m_pLogFast = NULL;
	m_LogFast_characterPresent = 0;
	m_LogFast_save = NO;

	if ( SUCCESS( Status ) )
	{
		m_pLogFast = (char*)ExAllocatePoolWithTag( NonPagedPool, BUFFER_SIZE_TEXT, MEMORY_TAG_LOGGER_FAST );
		if ( m_pLogFast == NULL )
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
		}
	}

//Initialize mutex.
	if ( SUCCESS( Status ) )
	{
	//We need mutex to synhronize different thread sequencing.
		KeInitializeMutex( &m_Mutex_logger, 0 );
	}
#endif //GL_LOGGER
}

#pragma CODE_LOCKED
logger_c::~logger_c()
{
#if defined ( GL_LOGGER )
	m_CanLog = NO;		//not really needed, but it's another small protection

//Free resources.
	if ( m_pLogFast != NULL )
	{
		ExFreePoolWithTag( m_pLogFast, MEMORY_TAG_LOGGER_FAST );
	}
	if ( m_pFileBuffer_start != NULL )
	{
		ExFreePoolWithTag( m_pFileBuffer_start, MEMORY_TAG_LOGGER_TEXT );
	}

#endif //GL_LOGGER
}

//***************************************************************************
// ProcedureName:
//		operator new/delete
//
// Explanation:
//		Procedure will allocate/deallocate class memory.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// Synhronization:
//		None
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return Codes:
//		...
//
#pragma CODE_PAGED
void* __cdecl logger_c::operator new ( size_t size )
{
	if ( size > 0 )
	{
		return (void*)ExAllocatePoolWithTag( NonPagedPool, size, MEMORY_TAG_LOGGER_CLASS );
	}
	else
	{
		return 0;
	}
}

#pragma CODE_PAGED
void __cdecl logger_c::operator delete ( void* pMemory )
{
	if ( pMemory != NULL )
	{
		ExFreePoolWithTag( pMemory, MEMORY_TAG_LOGGER_CLASS );
	}
}

#endif	//DBG_AND_LOGGER
