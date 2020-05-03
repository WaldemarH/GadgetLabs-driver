//***************************************************************************
//***************************************************************************
//
//                          PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license agreement or
// nondisclosure agreement with Waldemar Haszlakiewicz and may not be
// copied or disclosed except in accordance with the terms of that agreement.
//
//               Copyright (C) 2009 Waldemar Haszlakiewicz
//                            All Rights Reserved.
//
//***************************************************************************
//***************************************************************************
//
// MODULE:
//      card.cpp
//
// PURPOSE:
//      File contains the card class functions.
//
// STATUS:
//      Unfinished/Untested.
//
//***************************************************************************
//***************************************************************************
#include "main.h"
#include "channel_base.h"

#pragma DATA_LOCKED


#pragma CODE_LOCKED
void channel_base_c::Set_Buffer_AVStream( PULONG pBuffer )
{
}

#pragma CODE_LOCKED
void channel_base_c::Set_HardwareAddress( unsigned long offsetInSamples )
{
}


//***************************************************************************
// ProcedureName:
//		ZeroChannelMemory
//
// Explanation:
//		Procedure will zero out the output channels data.
//		And with that make the channel silent.
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
#pragma CODE_LOCKED
NTSTATUS channel_base_c::ZeroChannelMemory_Full( void )
{
//Set buffer variables...
	m_BufferSize_copyInCurrentPass = m_Channel_size * m_nChannels;
	Set_HardwareAddress( 0 );

//Zero out the memory.
	return ZeroChannelMemory();
}
#pragma CODE_LOCKED
NTSTATUS channel_base_c::ZeroChannelMemory( void )
{
	return STATUS_UNSUCCESSFUL;
}

//***************************************************************************
// ProcedureName:
//		Set_CopyBuffersProcedure_AVStream
//
// Explanation:
//		Procedure will select proper DX(WDM) procedure from the required type.
//
// IRQ Level:
//		ANY
//
// --------------------------------------------------------------------------
//
// dwChannelMask: IN
//		Defines which of the channels are enabled.
//
// --------------------------------------------------------------------------
// Return:
//		void
//
#pragma CODE_LOCKED
NTSTATUS channel_base_c::Set_CopyBuffersProcedure_AVStream( unsigned long nChannels, CHANNEL_BIT_SIZE nBitsInSample )
{
	return STATUS_UNSUCCESSFUL;
}
