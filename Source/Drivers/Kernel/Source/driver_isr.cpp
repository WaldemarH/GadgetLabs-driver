//*****************************************************************************
//*****************************************************************************
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
//*****************************************************************************
//*****************************************************************************
//
// PURPOSE:
//		Contains InterruptServiceRoutine and DeferredProcedureCall/s for audio
//		part of the driver.
//
// STATUS:
//		Finished.
//
//*****************************************************************************
//*****************************************************************************
#include "main.h"
#include "card_altera_firmware.h"
#include "card_plx.h"
#include "asio_handle.h"


//***************************************************************************
// ProcedureName:
//		InterruptServiceRoutine
//
// Explanation:
//		Procedure checks if interrupt has came from GadgetLabs card and if it
//		had, it stops the INT flag and signals the DPC to handle the request.
//
// IRQ Level:
//		Device IRQ Level
//
// --------------------------------------------------------------------------
// Interrupt - IN
//		Caller-supplied pointer to an interrupt object. 
//
// ServiceContext - IN
//		Caller-supplied pointer to context information,
//		specified in a previous call to IoConnectInterrupt.
//
// --------------------------------------------------------------------------
// Return:
//		TRUE (IRQ was from our card and we have done our stuff),
//		FALSE (IRQ wasn't from our card).
//
#pragma CODE_LOCKED
BOOLEAN driver_c::InterruptServiceRoutine_Static
(
	IN struct _KINTERRUPT*	Interrupt,
	IN PVOID				ServiceContext
)
{
	card_c*		pCard_master = (card_c*)ServiceContext;

//--------------------------------------------------------------------------
//Did interrupt come from GL card?
//
//Read PLX chip Interrupt Control/Status register.
	if ( ( pCard_master->ReadDword( pCard_master->m_IoPlx, PLX_INT_CN_ST ) & PLX_INT_CN_ST__INT1_ST ) == 0 )
	{
	//Card interrupt is not enabled.
		return FALSE;
	}

//--------------------------------------------------------------------------
//Process interrupt
	return pDriver->InterruptServiceRoutine( pCard_master );
}

BOOLEAN driver_c::InterruptServiceRoutine
(
	IN card_c*		pCard_master
)
{
//--------------------------------------------------------------------------
//Card interrupt is enabled -> process the interrupt.
//
//Initialize variables.
	PUSHORT		IoWave_master = pCard_master->m_IoWave;
	WORD		data = pCard_master->ReadWord( IoWave_master, SC_INTERRUPT );

//Is interrupt enabled?
//
//Notice:
//For MIDI the interrupt will come from every card on it's own, where for Wave the intterupt
//will come only from master card.
//
//Also MIDI interrupts are intended only for MIDI input and not for MIDI output.
//Later will be handled by the 1ms timer.
	BOOLEAN			interruptProcessed = FALSE;
	ULONGLONG		timestamp = 0;

	//MIDI
	//
	//Notice:
	//MIDI DPC must have a higher priority as MIDI data is comming at every 320us.
	if ( ( data & SC_UART_INT_ISSUED ) != 0 )
	{
	//Do we have to stop midi streaming?
		if ( pCard_master->m_StopStream_midi == YES )
		{
		//Stop midi engine interrupt.
			pCard_master->WriteWord( IoWave_master, SC_INTERRUPT, data & ~( SC_ENABLE_UART_INT ) );
			pCard_master->m_StopStream_midi = NO;
		}
		else
		{
		//Leave it running -> process interrupt.
			//Get current timestamp.
			timestamp = KeQueryPerformanceCounter( NULL ).QuadPart;

			//Update timestamp only if it was pulled by the DPC.
			if ( pCard_master->m_TimeStamp_midi == CLOCK_ZERO_TIMESTAMP )
			{
				pCard_master->m_TimeStamp_midi = timestamp;
			}

		//Add DispatchCall to queue.
			KeInsertQueueDpc( &pCard_master->m_DPCObject_midi_in, 0, 0 );
		}

	//Interrupt processed.
		interruptProcessed = TRUE;
	}

	//Wave
	if ( ( data & SC_AUDIO_ENGINE_INT_ISSUED ) != 0 )
	{
	//Do we have to stop wave streaming?
		if ( pCard_master->m_StopStream_wave == YES )
		{
		//Stop audio engine interrupt.
			pCard_master->WriteWord( IoWave_master, SC_INTERRUPT, data & ~( SC_ENABLE_AUDIO_ENGINE_INT ) );
			pCard_master->m_StopStream_wave = NO;
		}
		else
		{
		//Leave it running -> process interrupt.
			//Get current timestamp.
			//
			//Notice:
			//Get timestamp and read sample offset must be as close as it can get.
			if ( timestamp == 0 )
			{
			//Timestamp wasn't received yet -> get it now.
				pCard_master->m_TimeStamp_wave = KeQueryPerformanceCounter( NULL ).QuadPart;
			}
			else
			{
			//Timestamp was already received.
				pCard_master->m_TimeStamp_wave = timestamp;
			}

		//Get cards offsets.
			card_c*				pCard = pCard_master;
			while ( pCard != NULL )
			{
				PUSHORT		IoWave = pCard->m_IoWave;

			//Get analog offset.
				pCard->m_ChannelOffset = (unsigned long)pCard->ReadWord( IoWave, SC_SAMPLE_COUNTER );

			//Notice:
			//All the offsets must have samples at the buffer offset or more.
			//In perfect world we would get the values similar to values below... all cards just 1 sample over the border.
			//
			//example:
			//card 1: 0x201 (the value must be bigger or same as 0x200 )
			//card 2: 0x201 (the value must be bigger or same as 0x200 )
			//card 3: 0x201 (the value must be bigger or same as 0x200 )
			//card 4: 0x201 (the value must be bigger or same as 0x200 )
			//	DbgPrint( "\nISR channelOffset: 0x%X (card: 0x%X)\n", pCard_current->m_ChannelOffset, pCard_current );

			//Next card...
				pCard = pCard->m_pCard_slave;
			}
			//pLogger->LogFast( "ISR: offset: 0x%X\n", pCard_master->m_ChannelOffset );
			
		//Clock -> audio position.
			if ( pCard_master->m_Clock_samples.clock != CLOCK_ZERO_TIMESTAMP )
			{
			//Clock was set before -> check if card counter wrapped.
			//
			//Is the current loWord smaller than previous one?
				if ( (WORD)pCard_master->m_ChannelOffset < pCard_master->m_Clock_samples.words.word_loLo )
				{
				//Yes -> the counter wrapped.
					pCard_master->m_Clock_samples.clock += 0x10000;
				}
			}
			pCard_master->m_Clock_samples.words.word_loLo = (WORD)pCard_master->m_ChannelOffset;

			//pLogger->LogFast( "ISR: Master clock: 0x%.8I64X\n", pCard_master->m_Clock_samples.clock );

		//Set new interrupt time (this will also clear the interrupt flag).
		//
		//Because of the hardware design the new value should be:
		//bufferSize + 1 - (channel address & (bufferSize-1))
			data =
			  m_BufferSize_plus1
			  -
			  ( (WORD)pCard_master->m_ChannelOffset & m_BufferSize_minus1 );
			pCard_master->WriteWord( IoWave_master, SC_INTERRUPT_DOWNCOUNTER, data );

		//Add DispatchCall to queue.
			KeInsertQueueDpc( &pCard_master->m_DPCObject_wave, 0, 0 );
		}

	//Interrupt processed.
		interruptProcessed = TRUE;
	}

	return interruptProcessed;
}

//***************************************************************************
// ProcedureName: DeferredProcedureCall_Wave
//
// Explanation:
//
// IRQ Level: DISPATCH_LEVEL
//
// --------------------------------------------------------------------------
//  Dpc - IN
//		Caller-supplied pointer to a KDPC structure, which represents the
//		DPC object that is associated with this CustomDpc routine. 
//
//  DeferredContext - IN
//		Caller-supplied pointer to driver-defined context information that
//		was specified in a previous call to KeInitializeDpc. 
//
//  SystemArgument1 - IN
//		Caller-supplied pointer to driver-supplied information that was
//		specified in a previous call to KeInsertQueueDpc. 
//
//  SystemArgument2 - IN
//		Caller-supplied pointer to driver-supplied information that was
//		specified in a previous call to KeInsertQueueDpc.
//
// --------------------------------------------------------------------------
// Return: None.
//
#pragma CODE_LOCKED
void driver_c::DeferredProcedureCall_Static_Wave
(
	IN PKDPC Dpc,
	IN PVOID DeferredContext,
	IN PVOID SystemArgument1,
	IN PVOID SystemArgument2
)
{
	pDriver->DeferredProcedureCall_Wave( (card_c*)DeferredContext );
	return;
}
void driver_c::DeferredProcedureCall_Wave
(
	IN card_c*		pCard_master
)
{
	int					i;
	card_c*				pCard;
	KIRQL				spinLock_oldLevel;

	unsigned long		cards_offsets[MAX_CARDS];
	unsigned long		clock_samplesLost;
	ULONGLONG			clock_samples;
	ULONGLONG			clock_samples_previous;
	ULONGLONG			timeStamp;

//Save ISR data to local variables, so that if ISR fires again our data wont get corrupted.
	i = 0;
	pCard = pCard_master;

	//Acquire interrupt spinLock.
	spinLock_oldLevel = KeAcquireInterruptSpinLock( pCard_master->m_pIRQObject );

		while ( pCard != NULL )
		{
		//Get offsets.
			cards_offsets[i] = pCard->m_ChannelOffset;

		//Next...
			++i;
			pCard = pCard->m_pCard_slave;
		}

	//Sample clock and time stamp.
		clock_samples = pCard_master->m_Clock_samples.clock;

		//Get previous and set "current" previous.
		clock_samples_previous = pCard_master->m_Clock_samples_previous.clock;
		pCard_master->m_Clock_samples_previous.clock = clock_samples;

		//Get timestamp.
		timeStamp = pCard_master->m_TimeStamp_wave;

	//Release interrupt spinLock.
	KeReleaseInterruptSpinLock( pCard_master->m_pIRQObject, spinLock_oldLevel );

//Update timeStamp so that the time will point at the start of the buffer.
	//Get the size of offset that we need to fix the master time.
	//
	//time = time - delta * [ (1/sampleRate) / 100*10^-9 ] =
	//     = time - delta * [ (1/sampleRate) * 10^9/100 ] =
	//(1.) = time - delta * [ 10^7/sampleRate ] =
	//(2.) = time - ( delta * 10^7 ) / sampleRate  (more precise version)
	//
	//Notice:
	//Delta should be the same for analog and digital.
	ULONGLONG	delta = clock_samples & m_BufferSize_minus1;
	timeStamp -= ( ( delta * 10000000 )/pCard_master->m_SampleRate );

//Update master clock to buffer start edge.
//
//Why?
//We are not really interested where in current buffer are we, the only thing interesting is which buffer is it.
	clock_samples &= pDriver->m_BufferSize_minus1Negated;

//Did we skip any card buffers?
	clock_samplesLost = 0;

	if ( clock_samples_previous != 0 )
	{
	//We had at least 1 DPC call before... check if we lost something.
	//
	//Notice:
	//We have to "bufferize" every clock or offsets can do havoc on the difference result.
		clock_samplesLost = (unsigned long)(
		  clock_samples				//already moved to buffer start edge
		  -
		  ( clock_samples_previous & pDriver->m_BufferSize_minus1Negated )
		);
		if ( clock_samplesLost > 0 )
		{
		//Only count number of samples actualy missing (the "previous" buffer got filled succesfully).
			clock_samplesLost -= pDriver->m_BufferSize;
		}
	}

//Acquire channelInfo spinLock
	KeAcquireSpinLockAtDpcLevel( &pDriver->m_SpinLock_channel_wave );

	//Calculate card buffer offsets.
		unsigned long		channelsInUse_ASIO = 0;
		unsigned long		channelsInUse_AVStream = 0;

		i = 0;
		pCard = pCard_master;
		while ( pCard != NULL )
		{
		//Calculate card buffer offsets.
		//
		//(OUT means the place where we should copy the output samples and
		// IN  means the place from where should we copy the input samples.)
		//
		//
		//  ---------------------------------
		//  |   |IN |   |OUT|   |   |   |   |
		//  ---------------------------------
		//            |
		//            |
		//  current ADC and DAC offset
		//
		//
		//This shows us that DPC must copy new DAC samples to the next buffer and retrieve the ADC samples from previous buffer.
		//
		//Notice:
		//Thanks Linux team for this small and very nice piece of code below.
		//
			unsigned long	offset_base = cards_offsets[i] & pDriver->m_BufferSize_minus1Negated;
			unsigned long	offset_in = offset_base - pDriver->m_BufferSize;
			unsigned long	offset_out = offset_base + pDriver->m_BufferSize;

			if ( pCard->m_Type != card_c::CARD_TYPE_824 )
			{
			//4xx
				offset_in &= SC4_ADDRESS_MASK;
				offset_out &= SC4_ADDRESS_MASK;
			}
			else
			{
			//824
				offset_in &= SC8_ADDRESS_MASK;
				offset_out &= SC8_ADDRESS_MASK;
			}

			pCard->m_ChannelOffset_In = offset_in;
			pCard->m_ChannelOffset_Out = offset_out;

		//Channels in use...
			channelsInUse_ASIO |= pCard->m_Channels_nInUse_INs_ASIO | pCard->m_Channels_nInUse_OUTs_ASIO;
			channelsInUse_AVStream |= pCard->m_Channels_nInUse_INs_AVStream | pCard->m_Channels_nInUse_OUTs_AVStream;

		//Next...
			++i;
			pCard = pCard->m_pCard_slave;
		}

	//Process AVStream...
		if ( channelsInUse_AVStream != 0 )
		{
		//Process outputs first... they are more sensitive to 'time left to process'.
			pCard = pCard_master;
			while ( pCard != NULL )
			{
			//Any AVStream channels to process?
				if ( pCard->m_Channels_nInUse_OUTs_AVStream > 0 )
				{
				//Stereo channels...
					channel_base_c*		pChannel;
					channel_base_c**	ppChannel = (channel_base_c**)pCard->m_pChannels_Stereo_OUTs;

					while ( ( pChannel = *ppChannel ) != NULL )
					{
						pChannel->TransferData_AVStream( pCard->m_ChannelOffset_Out, clock_samples, clock_samplesLost, (unsigned long)pCard_master->m_SampleRate );

					//Next...
						++ppChannel;
					}
					
				//All channel...
					pChannel = pCard->m_pChannels_All_OUTs;
					if ( pChannel != NULL )
					{
						pChannel->TransferData_AVStream( pCard->m_ChannelOffset_Out, clock_samples, clock_samplesLost, (unsigned long)pCard_master->m_SampleRate );
					}
				}

			//Next...
				pCard = pCard->m_pCard_slave;
			}

		//Process inputs...
			pCard = pCard_master;
			while ( pCard != NULL )
			{
			//Any AVStream channels to process?
				if ( pCard->m_Channels_nInUse_INs_AVStream > 0 )
				{
				//Stereo channels...
					channel_base_c*		pChannel;
					channel_base_c**	ppChannel = (channel_base_c**)pCard->m_pChannels_Stereo_INs;

					while ( ( pChannel = *ppChannel ) != NULL )
					{
						pChannel->TransferData_AVStream( pCard->m_ChannelOffset_In, clock_samples, clock_samplesLost, (unsigned long)pCard_master->m_SampleRate );

					//Next...
						++ppChannel;
					}
					
				//All channel...
					pChannel = pCard->m_pChannels_All_OUTs;
					if ( pChannel != NULL )
					{
						pChannel->TransferData_AVStream( pCard->m_ChannelOffset_In, clock_samples, clock_samplesLost, (unsigned long)pCard_master->m_SampleRate );
					}
				}

			//Next...
				pCard = pCard->m_pCard_slave;
			}
		}

	//Process ASIO inputs (outputs are processed directly by the ASIO driver)...
		if ( channelsInUse_ASIO != 0 )
		{
			asioHandle_c*		pAsioHandle = m_ASIO_Handle_master;

			while ( ( pAsioHandle = (asioHandle_c*)pAsioHandle->GetNext() ) != NULL )
			{
			//Do the honky tonk.
				pAsioHandle->TransferData_Input( clock_samples, clock_samplesLost );
			}
		}

	KeReleaseSpinLockFromDpcLevel( &pDriver->m_SpinLock_channel_wave );

	return;
}

//***************************************************************************
// ProcedureName: MIDI DeferredProcedureCall
//
// Explanation:
//
// IRQ Level: DISPATCH_LEVEL
//
// --------------------------------------------------------------------------
//  Dpc - IN
//		Caller-supplied pointer to a KDPC structure, which represents the
//		DPC object that is associated with this CustomDpc routine. 
//
//  DeferredContext - IN
//		Caller-supplied pointer to driver-defined context information that
//		was specified in a previous call to KeInitializeDpc. 
//
//  SystemArgument1 - IN
//		Caller-supplied pointer to driver-supplied information that was
//		specified in a previous call to KeInsertQueueDpc. 
//
//  SystemArgument2 - IN
//		Caller-supplied pointer to driver-supplied information that was
//		specified in a previous call to KeInsertQueueDpc.
//
// --------------------------------------------------------------------------
// Return: None.
//
#pragma CODE_LOCKED
void driver_c::DeferredProcedureCall_Static_Midi_In
(
	IN PKDPC Dpc,
	IN PVOID DeferredContext,
	IN PVOID SystemArgument1,
	IN PVOID SystemArgument2
)
{
	pDriver->DeferredProcedureCall_Midi_In( (card_c*)DeferredContext );
	return;
}
void driver_c::DeferredProcedureCall_Midi_In
(
	IN card_c*		pCard
)
{
//Acquire interrupt spinLock.
	ULONGLONG		timestamp;

	KIRQL	spinLock_oldLevel = KeAcquireInterruptSpinLock( pCard->m_pIRQObject );

	//Get timestamp.
		timestamp = pCard->m_TimeStamp_midi;
		pCard->m_TimeStamp_midi = CLOCK_ZERO_TIMESTAMP;

	//Release interrupt spinLock.
	KeReleaseInterruptSpinLock( pCard->m_pIRQObject, spinLock_oldLevel );

//Acquire channelInfo spinLock
	KeAcquireSpinLockAtDpcLevel( &pCard->m_SpinLock_channel_midi_in );

	//MIDI input.
		channel_midi_c*		pChannel_midi = pCard->m_pChannels_Midi_INs;

		if ( pChannel_midi->m_Type_client != channel_midi_c::CHANNEL_TYPE_CLIENT_NON )
		{
			if ( pChannel_midi->m_State == KSSTATE_RUN )
			{
			////Account for lost samples or update start timestamp because of RUN->PAUSE state.
			//	if ( pChannel_midi->m_Timestamp_streamPause > 1 )
			//	{
			//	//Channel has just came out of RUN->PAUSE state -> update start timestamp to account for the ignored time.
			//		ULONGLONG	time_ignore = timestamp - pChannel_midi->m_Timestamp_streamPause;
			//		pChannel_midi->m_Timestamp_streamStart += time_ignore;

			//	//Reset PAUSE timestamp.
			//		pChannel_midi->m_Timestamp_streamPause = 0;
			//	}

			//Data transfer...
				if ( pChannel_midi->m_pCopyBuffersProcedure != NULL )
				{
					(pChannel_midi->*pChannel_midi->m_pCopyBuffersProcedure)( timestamp );
				}
			}
			//else if ( pChannel_midi->m_State == KSSTATE_PAUSE )
			//{
			////If channel just moved to pause state get current timestamp.
			//	if ( pChannel_midi->m_Timestamp_streamPause == 1 )
			//	{
			//	//Yes current channel was just moved to pause state...
			//		pChannel_midi->m_Timestamp_streamPause = timestamp;
			//	}
			//}
		}

//Release channelInfo spinLock
	KeReleaseSpinLockFromDpcLevel( &pCard->m_SpinLock_channel_midi_in );

	return;
}
#pragma CODE_LOCKED
void driver_c::DeferredProcedureCall_Static_Midi_Out
(
	IN PKDPC Dpc,
	IN PVOID DeferredContext,
	IN PVOID SystemArgument1,
	IN PVOID SystemArgument2
)
{
	pDriver->DeferredProcedureCall_Midi_Out( (card_c*)DeferredContext );
	return;
}
void driver_c::DeferredProcedureCall_Midi_Out
(
	IN card_c*		pCard
)
{
//Acquire channelInfo spinLock
	KeAcquireSpinLockAtDpcLevel( &pCard->m_SpinLock_channel_midi_out );

	//MIDI output.
		channel_midi_c*		pChannel_midi = pCard->m_pChannels_Midi_OUTs;

		if ( pChannel_midi->m_Type_client != channel_midi_c::CHANNEL_TYPE_CLIENT_NON )
		{
			if ( pChannel_midi->m_State == KSSTATE_RUN )
			{
			////Account for lost samples or update start timestamp because of RUN->PAUSE state.
			//	if ( pChannel_midi->m_Timestamp_streamPause > 1 )
			//	{
			//	//Channel has just came out of RUN->PAUSE state -> update start timestamp to account for the ignored time.
			//		ULONGLONG	time_ignore = KeQueryPerformanceCounter( NULL ).QuadPart - pChannel_midi->m_Timestamp_streamPause;
			//		pChannel_midi->m_Timestamp_streamStart += time_ignore;

			//	//Reset PAUSE timestamp.
			//		pChannel_midi->m_Timestamp_streamPause = 0;
			//	}

			//Data transfer...
				if ( pChannel_midi->m_pCopyBuffersProcedure != NULL )
				{
					(pChannel_midi->*pChannel_midi->m_pCopyBuffersProcedure)( 0 );
				}
			}
			//else if ( pChannel_midi->m_State == KSSTATE_PAUSE )
			//{
			////If channel just moved to pause state get current timestamp.
			//	if ( pChannel_midi->m_Timestamp_streamPause == 1 )
			//	{
			//	//Yes current channel was just moved to pause state...
			//		pChannel_midi->m_Timestamp_streamPause = KeQueryPerformanceCounter( NULL ).QuadPart;
			//	}
			//}
		}

//Release channelInfo spinLock
	KeReleaseSpinLockFromDpcLevel( &pCard->m_SpinLock_channel_midi_out );

	return;
}
