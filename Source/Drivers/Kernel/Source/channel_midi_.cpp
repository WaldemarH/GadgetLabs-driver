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
//      File contains the channel class functions.
//
// STATUS:
//      Unfinished/Untested.
//
//***************************************************************************
//***************************************************************************
#include "main.h"
#include "channel_midi.h"

//***************************************************************************
// ProcedureName:
//		Constructor / Destructor
//
// Explanation:
//		Procedure will initialize/free midi channel class.
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
//		Non.
//
#pragma CODE_PAGED
channel_midi_c::channel_midi_c
(
	IN card_c*			pCard,
	IN bool				isInput
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

//Initialize variables.
	m_pCard = pCard;

	//Types.
	m_Type = ( isInput == true ) ? CHANNEL_TYPE_IN : CHANNEL_TYPE_OUT;

	//State.
	m_State = KSSTATE_STOP;
	m_Type_client = CHANNEL_TYPE_CLIENT_NON;

	//Samples clock definitions.
	//m_Clock_playOffset.clock = 0;
	//m_Clock_timestamp_streamPause.clock = 0;
	//m_Clock_timestamp_streamStart.clock = 0;
	//m_Clock_writeOffset.clock = 0;

	//m_Clock_timestamp_streamStart_isSet = NO;
	//m_Clock_writeOffset_newStreamPointer = YES;

	//m_Clock_samplesLost = 0;

	//AVStream variables
	m_pPin = NULL;

	//Process buffers...
	m_pCopyBuffersProcedure = NULL;

	m_Data_In;
	m_Data_Out_lastOffset = 0;

	//m_Timestamp_streamStart = 0;
	//m_Timestamp_streamStart_isSet = NO;

#ifdef DBG_AND_LOGGER
	Procedure_Exit( STATUS_SUCCESS );
#endif
}

//channel_midi_c::~channel_midi_c()
//{
//}

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
void* __cdecl channel_midi_c::operator new ( size_t size )
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	void*		pVoid;
	if ( size > 0 )
	{
		pVoid = (void*)ExAllocatePoolWithTag( NonPagedPool, size, MEMORY_TAG_CLASS_CHANNEL_MIDI );
	}
	else
	{
		pVoid = NULL;
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( ( pVoid != NULL ) ? STATUS_SUCCESS : STATUS_INSUFFICIENT_RESOURCES );
#endif
	return pVoid;
}

#pragma CODE_PAGED
void __cdecl channel_midi_c::operator delete ( void* pMemory )
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	if ( pMemory != NULL )
	{
		ExFreePoolWithTag( pMemory, MEMORY_TAG_CLASS_CHANNEL_MIDI );
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( STATUS_SUCCESS );
#endif
}
