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


#ifdef GL_LOGGER

#pragma DATA_LOCKED

#pragma CODE_LOCKED
void logger_c::LogFast_Reset()
{
	m_LogFast_characterPresent = 0;
	m_LogFast_save = NO;
}

//Must be protected by spinlock.
#pragma CODE_LOCKED
void __stdcall logger_c::LogFast
(
	IN const char*		pText,	//max 256 character per full string
	IN ...
)
{
	if (
	  ( m_pLogFast != NULL )
	  &&
	  ( ( m_LogFast_characterPresent + 256 ) < BUFFER_SIZE_TEXT )		//log until buffer is full
	  //&&
	  //( m_LogFast_save == NO )
	)
	{
		va_list		va;

	//Get string variables and create the string.
		va_start( va, pText );
		m_LogFast_characterPresent += vsprintf( m_pLogFast+m_LogFast_characterPresent, pText, va );

	#if defined( DBG_PRINT )
		DbgPrint( m_pLogFast );
		m_LogFast_characterPresent = 0;
	#endif
	}
	return;
}
//PASSIVE_LEVEL!!
#pragma CODE_LOCKED
void logger_c::LogFast_Save()
{
	if (
	  //( m_LogFast_save == YES )
	  //&&
	  ( m_pLogFast != NULL )
	  &&
	  ( m_LogFast_characterPresent > 0 )
	)
	{
	//#if defined( DBG_PRINT )
	//	DbgPrint( m_pLogFast );
	//#endif
	#if defined ( GL_LOGGER )
		m_pLogFast[m_LogFast_characterPresent] = 0;
		m_pLogFast[m_LogFast_characterPresent+1] = 0;

		Log_Procedure( "Fast Logger", false, 0, 0 );
		Save( m_pLogFast );
		Log_Procedure( "Fast Logger", false, true, 0 );
	#endif

		m_LogFast_characterPresent = 0;
		m_LogFast_save = NO;
	}
}

#endif //GL_LOGGER
