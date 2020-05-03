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
//		card.cpp
//
// PURPOSE:
//		File contains the card class functions.
//
// STATUS:
//		Unfinished/Untested.
//
//***************************************************************************
//***************************************************************************
#include "main.h"
#include "sync_start.h"


#pragma DATA_LOCKED

//***************************************************************************
// ProcedureName:
//		AddReference/RemoveReference
//
// Explanation:
//		Procedure will add/remove channel from channels references.
//
//		Notice:
//		Channel must not be Added twice... there is no protection for this.
//
// IRQ Level:
//		ANY LEVEL
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
NTSTATUS syncStart_c::AddReference( channel_base_c* pChannel )
{
	NTSTATUS			Status = STATUS_SUCCESS;

//Validate...
	//Channels can only be added if syncStart is not in run state.
	if ( m_Channels_nInRunningState > 0 )
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//Add channel reference.
	if ( SUCCESS( Status ) )
	{
		m_ppChannels[m_Channels_nReferences] = pChannel;

	//Rise number of channels referenced.
		++m_Channels_nReferences;
	}

//#ifdef GL_LOGGER
//	pLogger->LogFast( "syncStart_c::AddReference: m_Channels_nReferences: %d\n", m_Channels_nReferences );
//#endif

	return Status;
}

#pragma CODE_LOCKED
NTSTATUS syncStart_c::RemoveReference( channel_base_c* pChannel )
{
	NTSTATUS			Status = STATUS_UNSUCCESSFUL;

//Find and remove channel reference.
	channel_base_c*			pChannel_temp;
	channel_base_c**		ppChannels_temp = m_ppChannels;

	while ( ( pChannel_temp = *ppChannels_temp ) != NULL )
	{
	//Did we find it?
		if ( pChannel_temp == pChannel )
		{
		//Yes -> remove channel from references and update channels array.
			Status = STATUS_SUCCESS;

			//Lower the reference counter.
			--m_Channels_nReferences;

			//Move all upper channels 1 level down.
			do
			{
			//Move down.
				pChannel_temp = ppChannels_temp[0] = ppChannels_temp[1];

			//Update
				++ppChannels_temp;

			} while ( pChannel_temp != NULL );

			break;
		}

	//Next...
		++ppChannels_temp;
	}

//#ifdef GL_LOGGER
//	pLogger->LogFast( "syncStart_c::RemoveReference: m_Channels_nReferences: %d\n", m_Channels_nReferences );
//#endif
	return Status;
}


//***************************************************************************
// ProcedureName:
//		Channel_fromRunState/Channel_toRunState
//
// Explanation:
//		Procedure will decrease/increase number of channels in run state.
//
// IRQ Level:
//		ANY LEVEL
//
// Synhronization:
//		None
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return Codes:
//		non
//
#pragma CODE_LOCKED
NTSTATUS syncStart_c::Channel_FromRunState()
{
	NTSTATUS			Status = STATUS_SUCCESS;

//Validate...
	if ( m_Channels_nInRunningState == 0 )
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//Decrease number of channels in running state.
	if ( SUCCESS( Status ) )
	{
		--m_Channels_nInRunningState;
	}

#ifdef GL_LOGGER
	pLogger->Log( logger_c::LOGGER_SEND_TO_DEBUG_PRINT, "syncStart_c::Channel_FromRunState: Status: %d\n", Status );
#endif

	return Status;
}

#pragma CODE_LOCKED
NTSTATUS syncStart_c::Channel_ToRunState()
{
	NTSTATUS			Status = STATUS_SUCCESS;

//Validate...
	if ( m_Channels_nInRunningState == m_Channels_nReferences )
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//Increase number of channels in running state.
	if ( SUCCESS( Status ) )
	{
		++m_Channels_nInRunningState;
	}

#ifdef GL_LOGGER
	pLogger->Log( logger_c::LOGGER_SEND_TO_DEBUG_PRINT, "syncStart_c::Channel_ToRunState: Status: %d\n", Status );
#endif
	return Status;
}


bool syncStart_c::Channels_StartStreaming()
{
#ifdef GL_LOGGER
	pLogger->Log( logger_c::LOGGER_SEND_TO_DEBUG_PRINT, "syncStart_c::Channels_StartStreaming: %s\n", ( m_Channels_nInRunningState == m_Channels_nReferences ) ? "true" : "false" );
#endif


	return ( m_Channels_nInRunningState == m_Channels_nReferences );
}
