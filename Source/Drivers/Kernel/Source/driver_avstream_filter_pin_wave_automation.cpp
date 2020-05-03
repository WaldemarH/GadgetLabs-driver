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
//		avstream.cpp
//
// PURPOSE:
//		File contains the avstream subdriver class.
//
// STATUS:
//		Unfinished/Untested.
//
//***************************************************************************
//***************************************************************************
#include "main.h"
#include "card_altera_firmware.h"

#pragma DATA_LOCKED

//---------------------------------------------------------------------------------------
//-- WAVE Property pages ----------------------------------------------------------------
//---------------------------------------------------------------------------------------
//NOTICE:
//We assume that all property procedures are called at PASSIVE_LEVEL.
#pragma CODE_LOCKED
NTSTATUS driver_c::PropertyItem_Wave_Audio_Position_Handler_Get
(
	IN PIRP						pIrp,
	IN PKSIDENTIFIER			pRequest,
	IN OUT PKSAUDIO_POSITION	pData
)
{
#ifdef DBG_AND_LOGGER
	//DbgPrint( __FUNCTION__ );
//	Logger_SaveProcedureString(
//	  "PropertyItem_Wave_Audio_Position_Handler_Get",
//	  TRUE,
//	  FALSE,
//	  0
//	);
#endif*/

	NTSTATUS		Status = STATUS_SUCCESS;

//Initialize variables.
	PKSPIN			pPin = KsGetPinFromIrp( pIrp );
	if ( pPin == NULL )
	{
		Status = STATUS_UNSUCCESSFUL;
	}

	//Cards...
	card_c*			pCard = NULL;
	card_c*			pCard_master = NULL;

	if ( SUCCESS( Status ) )
	{
		Status = pDriver->GetCard_FromPin( pPin, &pCard );
	}
	if ( SUCCESS( Status ) )
	{
		pCard_master = pCard->GetMasterCard();
	}

	//Channel...
	channel_base_c*		pChannel_base = NULL;

	if ( SUCCESS( Status ) )
	{
		Status = pCard->GetChannel_FromPin_Wave( pPin, &pChannel_base );
	}

//Get clock position.
	if ( SUCCESS( Status ) )
	{
		sampleClock_s			clock_current;
		sampleClock_s			clock_timestamp_streamPause;
		sampleClock_s			clock_timestamp_streamStart;
		unsigned long			clock_timestamp_streamStart_isSet;
		sampleClock_s			clock_play;
		sampleClock_s			clock_write;

	//Get the offset values.
	//
	//Acquire channelInfo spinLock.
		KIRQL		spinLock_oldLevel;

		KeAcquireSpinLock( &pDriver->m_SpinLock_channel_wave, &spinLock_oldLevel );

		//---------------------------------------------------------------------
		//Get current clock.
		//
		//Acquire int spinLock (notice!!!! acquire spinlocks in the same order as everywhere else or you'll get a deadlock).
			if ( pChannel_base->m_Type == channel_base_c::CHANNEL_TYPE_OUT )
			{
				KIRQL		spinLock_oldLevel_ISR = KeAcquireInterruptSpinLock( pCard_master->m_pIRQObject );

				//Get last set clock.
					clock_current.clock = pCard_master->m_Clock_samples.clock;

				//Update clock to current value.
					WORD	clock_loWord = pCard_master->ReadWord( pCard_master->m_IoWave, SC_SAMPLE_COUNTER );
					if ( clock_current.clock != CLOCK_ZERO_TIMESTAMP )		//ignore until ISR sets the clock for the first time
					{
					//Clock was set before -> check if card counter wrapped.
						if ( clock_loWord < clock_current.words.word_loLo )
						{
							clock_current.clock += 0x10000;
						}
					}
					clock_current.words.word_loLo = clock_loWord;	//even this will get ignored if clock_timestamp_streamStart_isSet is not set yet

				KeReleaseInterruptSpinLock( pCard_master->m_pIRQObject, spinLock_oldLevel_ISR );
			}
		//---------------------------------------------------------------------
		//Get channel clock.
			clock_timestamp_streamPause = pChannel_base->m_Clock_timestamp_streamPause;
			clock_timestamp_streamStart = pChannel_base->m_Clock_timestamp_streamStart;
			clock_timestamp_streamStart_isSet = pChannel_base->m_Clock_timestamp_streamStart_isSet;
			clock_play = pChannel_base->m_Clock_playOffset;
			clock_write = pChannel_base->m_Clock_writeOffset;

			//pLogger->LogFast( "AP_Get: streamStart: 0x%.8I64X, isSet: %d\n", clock_timestamp_streamStart, clock_timestamp_streamStart_isSet );

		KeReleaseSpinLock( &pDriver->m_SpinLock_channel_wave, spinLock_oldLevel );

	//Is stream already producing a sound... is it at or past of it's first sample?
		if ( pChannel_base->m_Type == channel_base_c::CHANNEL_TYPE_IN )
		{
		//Input...
			pData->PlayOffset = clock_play.clock;
		}
		else
		{
		//Output...
			if (
			  ( clock_timestamp_streamStart_isSet == YES )
			  &&
			  ( clock_current.clock >= clock_timestamp_streamStart.clock )
			)
			{
			//Set the playOffset (length of the played stream).
				if ( clock_timestamp_streamPause.clock > 1 )
				{
				//Current channel is in PAUSE state -> use PAUSE timestamp.
				//
				//Notice:
				//Clock must freeze regarding the system timer.
					clock_play.clock = clock_timestamp_streamPause.clock - clock_timestamp_streamStart.clock;
				}
				else
				{
				//Channel is in RUN state -> use runnig timer.
				//
				//Notice:
				//If channel has come out of PAUSE state then start timestamp was updated to skip the time in which the channel was in PAUSE state.
					clock_play.clock = clock_current.clock - clock_timestamp_streamStart.clock;
				}
				//DbgPrint( "PlayOffset: 0x%.8I64X\n", clock_play.clock );

			//Convert single channel sample oriented clock to packed multichannel clock.
				clock_play.clock *= pChannel_base->m_nChannels;

			//Convert to bytes.
				switch ( pChannel_base->m_BitSize )
				{
				case channel_base_c::CHANNEL_BIT_SIZE_32 :
					samplesToBytes32( clock_play.clock );
					break;

				case channel_base_c::CHANNEL_BIT_SIZE_24 :
					samplesToBytes24( clock_play.clock );
					break;

				case channel_base_c::CHANNEL_BIT_SIZE_16 :
				default :
					samplesToBytes16( clock_play.clock );
					break;
				}

			//Set KSAUDIO_POSITION structure.
			//
			//Play: - PlayOffset: The byte(sample) that has just been played by the card.
			//      - WriteOffset: The last byte(sample) in current playback buffer.
			//
			//Record: - PlayOffset: The byte(sample) that has just been captured.
			//        - WriteOffset: The last byte(sample) in curent recording buffer.
			//
				pData->PlayOffset = clock_play.clock;
			}
			else
			{
			//No clock was set or stream is not producing sound yet.
				pData->PlayOffset = 0;
			}
		}
		pData->WriteOffset = clock_write.clock;

		//pLogger->LogFast( "AP_Get: PlayOffset: 0x%.8I64X, WriteOffset: 0x%.8I64X\n\n", pData->PlayOffset, pData->WriteOffset );

		//KeReleaseSpinLock( &pDriver->m_SpinLock_channel_wave, spinLock_oldLevel );
	}
//
//#ifdef DBG_PRINT
//	if ( FAILED( Status ) )
//	{
//		DbgPrint(
//		  "ERROR: PropertyItem_Wave_Audio_Position_Handler_Get returned error(0x%X).\n",
//		  Status
//		);
//	}
//#endif
///*#ifdef GL_LOGGER
//	Logger_SaveProcedureString(
//	  "PropertyItem_Wave_Audio_Position_Handler_Get",
//	  TRUE,
//	  TRUE,
//	  Status
//	);
//#endif
//*/
#ifdef DBG_AND_LOGGER
	//DbgPrint( __FUNCTION__ );
#endif
	return Status;
}
#pragma CODE_LOCKED
NTSTATUS driver_c::PropertyItem_Wave_Audio_Position_Handler_Set
(
	IN PIRP						pIrp,
	IN PKSIDENTIFIER			pRequest,
	IN OUT PKSAUDIO_POSITION	pData
)
{
#ifdef DBG_AND_LOGGER
	//DbgPrint( __FUNCTION__ );
#endif

	NTSTATUS		Status = STATUS_SUCCESS;

//Initialize variables.
	PKSPIN			pPin = KsGetPinFromIrp( pIrp );
	if ( pPin == NULL )
	{
		Status = STATUS_UNSUCCESSFUL;
	}

	//Cards...
	card_c*			pCard = NULL;

	if ( SUCCESS( Status ) )
	{
		Status = pDriver->GetCard_FromPin( pPin, &pCard );
	}

	//Channel...
	channel_base_c*		pChannel_base = NULL;

	if ( SUCCESS( Status ) )
	{
		Status = pCard->GetChannel_FromPin_Wave( pPin, &pChannel_base );
	}

//Set clock to the new position.
//
//Notice:
//I don't see the point of this procedure, but hay it's user's choice.
	if ( SUCCESS( Status ) )
	{
	//Acquire channelInfo spinLock.
		KIRQL		spinLock_oldLevel;

		KeAcquireSpinLock( &pDriver->m_SpinLock_channel_wave, &spinLock_oldLevel );

			pChannel_base->m_Clock_timestamp_streamStart.clock = pData->PlayOffset;
			pChannel_base->m_Clock_timestamp_streamStart_isSet = YES;

		KeReleaseSpinLock( &pDriver->m_SpinLock_channel_wave, spinLock_oldLevel );
	}

	//pLogger->Log( logger_c::LOGGER_SEND_TO_DEBUG_PRINT, "AP_Set: PlayOffset: 0x%.8I64X, WriteOffset: 0x%.8I64X\n\n", pData->PlayOffset, pData->WriteOffset );

//#ifdef DBG_PRINT
//	if ( Status != STATUS_SUCCESS )
//	{
//		DbgPrint(
//		  "ERROR: PropertyItem_Wave_Audio_Position_Handler_Set returned error(0x%X).\n",
//		  Status
//		);
//	}
//#endif
#ifdef DBG_AND_LOGGER
	//DbgPrint( __FUNCTION__ );
#endif
	return Status;
}
