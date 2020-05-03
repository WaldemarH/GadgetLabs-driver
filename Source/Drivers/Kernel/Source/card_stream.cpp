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
#include "card_altera_firmware.h"


#pragma DATA_LOCKED

//*****************************************************************************
// ProcedureName:
//		Streaming_Start_xxx/Streaming_Stop_xxx
//
//		Notice:
//		Can only be called on master card.
//
// Explanation:
//		Procedure will start/stop streaming on master and slave cards.
//
// IRQ Level:
//		ANY LEVEL
//
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
// Return:
//		void
//
#pragma CODE_LOCKED
void card_c::Streaming_Midi_Start()
{
//Start card.
//
//Change m_StopStream_midi to NO.
	KIRQL	spinLock_oldLevel = KeAcquireInterruptSpinLock( m_pIRQObject );

		m_StopStream_midi = NO;

		//Data is already protected by stopWavePart = NO ->
		//even if the interrupt occured right after we exit spinLock
		//data value will still be correct.
		WORD	data = ReadWord( m_IoWave, SC_INTERRUPT );

	KeReleaseInterruptSpinLock( m_pIRQObject, spinLock_oldLevel );

//Check if card is already started->if not start it.
	if ( ( data & SC_ENABLE_UART_INT ) == 0 )
	{
	//#ifdef GL_LOGGER
	//	pLogger->LogFast( "card_c::Streaming_Midi_Start: START Engine\n" );
	//#endif

	//Reset timestamper.
		//m_Clock_midi.clock = CLOCK_ZERO_TIMESTAMP;	//this is our 0... need it like this because of input timestamping
		m_TimeStamp_midi = CLOCK_ZERO_TIMESTAMP;

	//Start interrupts.
		WriteWord( m_IoWave, SC_INTERRUPT, data | SC_ENABLE_UART_INT );
	}

	return;
}
#pragma CODE_LOCKED
void card_c::Streaming_Midi_Stop()
{
//Stop midi interrupts.
//
//Notice:
//Here we can run into a problem ->card interrupt was issued just when we are in SpinLock protection->
//now as soon we'll come out of this procedure system will try to
//service our interrupt, but now if we disabled the card here
//our ISR routine will say that is not our card when it really is->system freeze.
//Solution:
//We'll stop the card in ISR.
//
//If system will start a channel before we'll stop the card it wont
//matter as stopTheCard will be cleared at start, so the ISR will
//just process interrupt like nothing happened at all.
	KIRQL	spinLock_oldLevel = KeAcquireInterruptSpinLock( m_pIRQObject );

		m_StopStream_midi = YES;

	KeReleaseInterruptSpinLock( m_pIRQObject, spinLock_oldLevel );

//#ifdef GL_LOGGER
//	pLogger->LogFast( "card_c::Streaming_Midi_Stop: STOP Engine\n" );
//	pLogger->m_LogFast_save = YES;
//#endif

	return;
}

#pragma CODE_LOCKED
NTSTATUS card_c::Streaming_Wave_Start()
{
	NTSTATUS		Status = STATUS_SUCCESS;

//Validate...
	//Is it a master card?
	if ( m_pCard_master != NULL )
	{
		Status = STATUS_UNSUCCESSFUL;
	}
	
//Start streaming.
	if ( SUCCESS( Status ) )
	{
	//Start master card and it will start all the slave cards too.
	//
	//Change stopWavePart to NO.
		KIRQL	spinLock_oldLevel = KeAcquireInterruptSpinLock( m_pIRQObject );

			m_StopStream_wave = NO;

			//Data is already protected by stopWavePart = NO ->
			//even if the interrupt occured right after we exit spinLock
			//data value will still be correct.
			WORD	data = ReadWord( m_IoWave, SC_INTERRUPT );

		KeReleaseInterruptSpinLock( m_pIRQObject, spinLock_oldLevel );

	//Check if card is already started->if not start it.
		if ( ( data & SC_ENABLE_AUDIO_ENGINE_INT ) == 0 )
		{
		#ifdef GL_LOGGER
			//pLogger->LogFast( "card_c::Streaming_Wave_Start: START Engine\n" );
			pLogger->Log( logger_c::LOGGER_SEND_TO_DEBUG_PRINT, "card_c::Streaming_Wave_Start: START Engine\n" );
		#endif

		//Reset timestamper.
			m_Clock_samples.clock = CLOCK_ZERO_TIMESTAMP;		//this is our 0... need it like this because of input timestamping
			m_Clock_samples_previous.clock = 0;

		//Reset fast logger.
		//#ifdef GL_LOGGER
		//	pLogger->LogFast_Reset();
		//#endif

		//ReSet cards addressPointers.
			Status = AddressPointers_Reset();

		//Enable interrupts.
			if ( SUCCESS( Status ) ) 
			{
			//Set new interrupt time.
			//
			//Because of the hardware design the new value should be:
			//bufferSize + 1 - (channel address & (bufferSize-1))
			//(the last part should be 0 at this stage).
				WriteWord( m_IoWave, SC_INTERRUPT_DOWNCOUNTER, (WORD)pDriver->m_BufferSize_plus1 );

			//Start interrupts.
				WriteWord( m_IoWave, SC_INTERRUPT, data | SC_ENABLE_AUDIO_ENGINE_INT );
			}
		}
	}

	return Status;
}
#pragma CODE_LOCKED
NTSTATUS card_c::Streaming_Wave_Stop()
{
	NTSTATUS		Status = STATUS_SUCCESS;

//Validate...
	//Is it a master card?
	if ( m_pCard_master != NULL )
	{
		Status = STATUS_UNSUCCESSFUL;
	}
	
//Stop streaming.
	if ( SUCCESS( Status ) )
	{
	//Is there a channel still running ( check the slave cards too )?
		unsigned long		areAnyChannelsPlaying = 0;
		card_c*				pCard_temp = this;

		while ( pCard_temp != NULL )
		{
		//Get info.
			areAnyChannelsPlaying |=
			(
			  pCard_temp->m_Channels_nInUse_INs_ASIO
			  |
			  pCard_temp->m_Channels_nInUse_OUTs_ASIO
			  |
			  pCard_temp->m_Channels_nInUse_INs_AVStream
			  |
			  pCard_temp->m_Channels_nInUse_OUTs_AVStream
			);

		//Next...
			pCard_temp = pCard_temp->m_pCard_slave;
		}

	//Stop the card?
		if ( areAnyChannelsPlaying == 0 )
		{
		#ifdef GL_LOGGER
			//pLogger->LogFast( "card_c::Streaming_Wave_Stop: STOP Engine\n" );
			pLogger->Log( logger_c::LOGGER_SEND_TO_DEBUG_PRINT, "card_c::Streaming_Wave_Stop: STOP Engine\n" );
		#endif

		//No more opened channels->stop the wave interrupts.
		//
		//Notice:
		//Here we can run into a problem ->
		//card interrupt was issued just when we are in SpinLock protection->
		//now as soon we'll come out of this procedure system will try to
		//service our interrupt, but now if we disabled the card here
		//our ISR routine will say that is not our card when it really is->
		//system freeze.
		//Solution:
		//We'll stop the card in ISR.
		//
		//If system will start a channel before we'll stop the card it wont
		//matter as stopTheCard will be cleared at start, so the ISR will
		//just process interrupt like nothing happened at all.
			KIRQL	spinLock_oldLevel = KeAcquireInterruptSpinLock( m_pIRQObject );

				m_StopStream_wave = YES;

			KeReleaseInterruptSpinLock( m_pIRQObject, spinLock_oldLevel );

		//#ifdef GL_LOGGER
		//	pLogger->m_LogFast_save = YES;
		//#endif
		}
	}

#ifdef GL_LOGGER
	pLogger->Log( logger_c::LOGGER_SEND_TO_DEBUG_PRINT, "card_c::Streaming_Wave_Stop: Status: %d\n", Status );
#endif

	return Status;
}
