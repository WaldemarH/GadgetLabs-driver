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
//      card.cpp
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
#include "channel_base.h"

//***************************************************************************
// ProcedureName:
//		Constructor / Destructor
//
// Explanation:
//		Procedure will initialize/free base channel class.
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
channel_base_c::channel_base_c
(
	IN card_c*			pCard,
	IN bool				isInput,
	IN bool				isStereo,
	IN unsigned long	channelSize_inSamples
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

//Initialize variables.
	m_pCard = pCard;

	//Types.
	m_Type = ( isInput == true ) ? CHANNEL_TYPE_IN : CHANNEL_TYPE_OUT;
	m_Format = ( isStereo == true ) ? CHANNEL_FORMAT_STEREO : CHANNEL_FORMAT_SURROUND;
	m_Type_client = CHANNEL_TYPE_CLIENT_NON;
	m_BitSize = CHANNEL_BIT_SIZE_24;

	//State.
	m_State = KSSTATE_STOP;

	//Addresses.
	m_Channel_size = channelSize_inSamples;

	//Samples clock definitions.
	m_Clock_playOffset.clock = 0;
	m_Clock_timestamp_streamStart.clock = 0;
	m_Clock_writeOffset.clock = 0;

	m_Clock_timestamp_streamStart_isSet = NO;
	m_Clock_writeOffset_newStreamPointer = YES;

	m_Clock_samplesLost = 0;

	//AVStream variables
	m_pPin = NULL;
	m_BufferSize_copyInCurrentPass = 0;
	m_BufferSize_nChannels_inSamples = 0;

	//Process buffers...
	m_pCopyBuffersProcedure = NULL;
	m_nChannels = 2;		//stereo by default... for All channel it will be set to proper value in Set_CopyBuffersProcedure_AVStream

#ifdef DBG_AND_LOGGER
	Procedure_Exit( STATUS_SUCCESS );
#endif
}

//channel_base_c::~channel_base_c()
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
void* __cdecl channel_base_c::operator new ( size_t size )
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	void*		pVoid;
	if ( size > 0 )
	{
		pVoid = (void*)ExAllocatePoolWithTag( NonPagedPool, size, MEMORY_TAG_CLASS_CHANNEL_BASE );
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
void __cdecl channel_base_c::operator delete ( void* pMemory )
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	if ( pMemory != NULL )
	{
		ExFreePoolWithTag( pMemory, MEMORY_TAG_CLASS_CHANNEL_BASE );
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( STATUS_SUCCESS );
#endif
}
