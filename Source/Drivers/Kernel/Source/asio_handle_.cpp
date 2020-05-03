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
#include "asio_handle.h"


#pragma DATA_LOCKED


#pragma CODE_LOCKED
asioHandle_c::asioHandle_c( asioHandle_c* pPrevious ) : linkedArray_c( (linkedArray_c*)pPrevious )
{
//Initialize variables...
	//Buffers memory...
	m_pMDL = NULL;
	m_pMemory_locked = NULL;

	//Channels...
	RtlZeroMemory( &m_pChannels_Stereo_INs, sizeof( m_pChannels_Stereo_INs ) );
	RtlZeroMemory( &m_pChannels_Stereo_OUTs, sizeof( m_pChannels_Stereo_OUTs ) );

	//Handle...
	m_HandleID = 0;
	m_HandleID_last = 1;		//only for master class

	//Pending IRP...
	m_pIRP_pending = NULL;

	//Switch event.
	m_pSwitchEvent = NULL;
}

#pragma CODE_LOCKED
asioHandle_c::~asioHandle_c()
{
//Free any remaining resources.
//
//Notice:
//If anything is still set at this time we can be in serious problem.
//And there is no guarantee that system wont blue screen... well it shouldn't but you never know.
//
	//Stop/Close channels.
	Channels_CloseAll();

	//Remove the IRP from the pending list.
	//
	//If IRP has already been removed from the list the m_pIRP_pending will be NULL.
	if ( m_pIRP_pending != NULL )
	{
		RemoveEntryList( &m_pIRP_pending->Tail.Overlay.ListEntry );
		m_pIRP_pending = NULL;
	}

	//Dereference switch event.
	if ( m_pSwitchEvent != NULL )
	{
		ObDereferenceObject( m_pSwitchEvent );
		m_pSwitchEvent = NULL;
	}

	//Unlock buffer memory.
	if ( m_pMDL != NULL )
	{
	//Unmap locked memory.
		if ( m_pMemory_locked != NULL )
		{
			MmUnmapLockedPages( m_pMemory_locked, m_pMDL );
			m_pMemory_locked = NULL;
		}

	//Unlock memory.
		MmUnlockPages( m_pMDL );

	//Free MDL.
		IoFreeMdl( m_pMDL );
		m_pMDL = NULL;
	}
}

#pragma CODE_LOCKED
unsigned long asioHandle_c::GetHandleID()
{
	return m_HandleID;
}
