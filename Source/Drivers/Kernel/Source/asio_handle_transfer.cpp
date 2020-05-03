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
#include "driver_ioctl_asio.h"
#include "card_altera_firmware.h"


#pragma DATA_LOCKED

#pragma CODE_LOCKED
void asioHandle_c::TransferData_Input
(
	ULONGLONG			clock_samples,
	unsigned long		clock_samplesLost
)
{
	NTSTATUS				Status = STATUS_SUCCESS;

	ioctl_asio_switch*		pSwitch = (ioctl_asio_switch*)m_pMemory_locked;

//Validate.
	if (
	  ( pSwitch == NULL )
	  ||
	  ( m_pSwitchEvent == NULL )
	)
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//Do the honky tonk.
	if ( SUCCESS( Status ) )
	{
	//Buffer select.
		//Get previous buffer select.
		unsigned long	bufferSelect = pSwitch->m_BufferSelect;

		//Set new one.
		if ( bufferSelect == ASIO_SWITCH_USE_BUFFER_A )
		{
			bufferSelect = ASIO_SWITCH_USE_BUFFER_B;
		}
		else
		{
			bufferSelect = ASIO_SWITCH_USE_BUFFER_A;
		}

	//Clock and timestamp.
		//Clock...
		if ( pSwitch->m_Clock_samples_first.clock == 0 )
		{
		//This is the first time we are here... set starting point of the clock relative to master card's clock.
			pSwitch->m_Clock_samples.clock = 0;
			pSwitch->m_Clock_samples_first.clock = clock_samples;
		}
		else
		{
		//Not the first time... get current clock position.
			pSwitch->m_Clock_samples.clock = clock_samples - pSwitch->m_Clock_samples_first.clock;

			//Did we lost some buffers?
			if ( clock_samplesLost > 0 )
			{
			//Yes -> resynhronize ASIO driver.
				pSwitch->m_Synhronize = TRUE;
			}
		}

	//Copy buffers.
		channel_stereo_c*		pChannel;
		channel_stereo_c**		ppChannels = &m_pChannels_Stereo_INs[0];

		while ( ( pChannel = *ppChannels ) != NULL )
		{
		//Select buffer to use.
			pChannel->m_BufferSelect = bufferSelect;

		//Process channel.
			//pChannel->m_pCard->WriteWord( pChannel->m_pCard->m_IoWave, SC4_BUS_WAIT_TIMER, 0 );

			//DbgPrint( "ASIO Input transfer: 0x%X\n", pChannel->m_pCard->m_ChannelOffset_In );
			pChannel->TransferData_ASIO( pChannel->m_pCard->m_ChannelOffset_In );

			//DbgPrint( "Wait time IN: %i\n", (unsigned long)pChannel->m_pCard->ReadWord( pChannel->m_pCard->m_IoWave, SC4_BUS_WAIT_TIMER ) );
			//pLogger->LogFast( "Wait time IN: %i\n", (unsigned long)pChannel->m_pCard->ReadWord( pChannel->m_pCard->m_IoWave, SC4_BUS_WAIT_TIMER ) );
			
		//Next...
			++ppChannels;
		}

	//Set new buffer.
		pSwitch->m_BufferSelect = bufferSelect;

	//Fire switch event.
		KeSetEvent( m_pSwitchEvent, EVENT_INCREMENT, FALSE );
	}
}

#pragma CODE_LOCKED
void asioHandle_c::TransferData_Output()
{
//Copy buffers.
	channel_stereo_c*		pChannel;
	channel_stereo_c**		ppChannels = &m_pChannels_Stereo_OUTs[0];

	ioctl_asio_switch*		pSwitch = (ioctl_asio_switch*)m_pMemory_locked;

	while ( ( pChannel = *ppChannels ) != NULL )
	{
	//#ifdef GL_LOGGER
	//	card_c*				pCard = pChannel->m_pCard;
	//	unsigned long		channelOffset = (unsigned long)pCard->ReadWord( pCard->m_IoWave, SC_SAMPLE_COUNTER );

	//	unsigned long		offset_base = channelOffset & pDriver->m_BufferSize_minus1Negated;
	//	unsigned long		offset_out = offset_base + pDriver->m_BufferSize;

	//	if ( pCard->m_Type != card_c::CARD_TYPE_824 )
	//	{
	//	//4xx
	//		offset_out &= SC4_ADDRESS_MASK;
	//	}
	//	else
	//	{
	//	//824
	//		offset_out &= SC8_ADDRESS_MASK;
	//	}

	//	//DbgPrint( "ASIO Output transfer: offset ISR: 0x%X, offset now: 0x%X (from: 0x%X)\n", pCard->m_ChannelOffset_Out, offset_out, channelOffset );
	//	if ( pCard->m_ChannelOffset_Out != offset_out )
	//	{
	//		pLogger->LogFast( "ERROR: TransferData_Output: offset ISR: 0x%X, offset now: 0x%X (from: 0x%X)\n", pCard->m_ChannelOffset_Out, offset_out, channelOffset );
	//	}
	//#endif

		pChannel->m_BufferSelect = pSwitch->m_BufferSelect;

	//Process channel.
		//pChannel->m_pCard->WriteWord( pChannel->m_pCard->m_IoWave, SC4_BUS_WAIT_TIMER, 0 );

		pChannel->TransferData_ASIO( pChannel->m_pCard->m_ChannelOffset_Out );

		//DbgPrint( "Wait time OUT: %i\n", (unsigned long)pChannel->m_pCard->ReadWord( pChannel->m_pCard->m_IoWave, SC4_BUS_WAIT_TIMER ) );
		//pLogger->LogFast( "Wait time OUT: %i\n", (unsigned long)pChannel->m_pCard->ReadWord( pChannel->m_pCard->m_IoWave, SC4_BUS_WAIT_TIMER ) );

	//Next...
		++ppChannels;
	}
}
