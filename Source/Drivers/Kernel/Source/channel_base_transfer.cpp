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
#include "channel_stereo.h"
#include "card_altera_firmware.h"


//***************************************************************************
// ProcedureName:
//		TransferData_AVStream
//
// Explanation:
//		Procedure will check if channel needs to be processed and if
//		it will tranfer data to or from the card.
//
// IRQ Level:
//		DPC_LEVEL
//
// Synhronization:
//		channel spinlock
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return Codes:
//		Non.
//
#pragma CODE_LOCKED
void channel_base_c::TransferData_AVStream
(
	unsigned long		offset_inSamples,
	ULONGLONG			clock_masterCard,
	unsigned long		clock_samplesLost,
	unsigned long		sampleRate						//needed for inputs only
)
{
//Is it an Avstream channel?
	if ( m_Type_client != CHANNEL_TYPE_CLIENT_AVSTREAM )
	{
	//Not... exit.
		return;
	}

//Process it.
	if ( m_State == KSSTATE_RUN )
	{
	//Account for lost samples or update start timestamp because of RUN->PAUSE state.
		if ( m_Clock_timestamp_streamPause.clock > 1 )
		{
		//Channel has just came out of RUN->PAUSE state -> update start timestamp to account for the ignored time.
			ULONGLONG	time_ignore = clock_masterCard - m_Clock_timestamp_streamPause.clock;
			m_Clock_timestamp_streamStart.clock += time_ignore;

		//Reset PAUSE timestamp.
			m_Clock_timestamp_streamPause.clock = 0;
		}
		else
		{
		//Channel was not in RUN->PAUSE state before... process normally.
			if ( m_Clock_timestamp_streamStart_isSet == YES )		//Was stream started already?
			{
				m_Clock_samplesLost += clock_samplesLost * m_nChannels;

				//DbgPrint( "AV: account for lost samples (%d)\n", clock_samplesLost );

			//As we'll first try to resync the lost samples we need to simulate like we are really in the past and that everything is
			//working as it should... i.e. no lost samples.
				clock_masterCard -= ( m_Clock_samplesLost/m_nChannels );
			}
		}

	//Get the buffer.
		PKSSTREAM_POINTER	pStreamPointer = KsPinGetLeadingEdgeStreamPointer( m_pPin, KSSTREAM_POINTER_STATE_LOCKED );

		//Validate.
		//
		//If current pStreamPointer doesn't contain data move to the next one and unlock it.
		//
		//... some applications send "NULL" pointer at first... legacy stuff...
		//
		if ( pStreamPointer != NULL )
		{
			if (
			  ( pStreamPointer->StreamHeader->Data == NULL )
			  ||
			  ( pStreamPointer->Offset->Data == NULL )
			  ||
			  ( pStreamPointer->Offset->Remaining == 0 )
			)
			{
				KsStreamPointerAdvanceOffsetsAndUnlock(
				  pStreamPointer,
				  0,
				  0,
				  TRUE
				);
				pStreamPointer = NULL;

			//We lost another buffer of samples regarding to master clock.
				if ( m_Clock_timestamp_streamStart_isSet == YES )	//Was stream started already?
				{
					m_Clock_samplesLost += pDriver->m_BufferSize;
				}
			}
		}
		if ( pStreamPointer != NULL )
		{
		//Check how much data is still in pStreamPointer and copy it.
		//If it's less than bufferSize than we will loop it until we will copy everything.
			unsigned long		alreadyProcessedSamples = 0;
			unsigned long		dataSize;
			unsigned short		loopIt = NO;
			unsigned short		newStreamPointer = NO;
			unsigned long*		pNumberOfSamplesToCopy;

			do
			{
			//#ifdef GL_LOGGER
			//	pLogger->LogFast( "TR: loopStart\n" );
			//#endif

			//Check if we will have to copy buffer in multi parts.
				dataSize = pStreamPointer->Offset->Remaining;
				switch ( m_BitSize )
				{
				case CHANNEL_BIT_SIZE_32 :
					bytesToSamples32( dataSize );
					break;

				case CHANNEL_BIT_SIZE_24 :
					bytesToSamples24( dataSize );
					break;

				case CHANNEL_BIT_SIZE_16 :
				default :
					bytesToSamples16( dataSize );
					break;
				}

				//Do we have to sync to master clock first?
				//pLogger->LogFast( "TR: samplesLost: 0x%X\n", m_Clock_samplesLost );
				if ( m_Clock_samplesLost > 0 )
				{
				//Sycn first.
					pNumberOfSamplesToCopy = &m_Clock_samplesLost;

					//pLogger->LogFast( "TR: Sync clock\n" );
				}
				else
				{
				//Already in sync.
					pNumberOfSamplesToCopy = &m_BufferSize_nChannels_inSamples;

					//pLogger->LogFast( "TR: Transfer Data\n" );
				}

				if ( dataSize >= *pNumberOfSamplesToCopy )
				{
				//Client buffer is bigger or just the same size as driver's one.
					if ( loopIt == NO )
					{
					//This is the first time we are here and buffer is big enough -> copy everything in one pass.
						m_BufferSize_copyInCurrentPass = *pNumberOfSamplesToCopy;
					}
					else
					{
					//Client's buffer is big enough -> last copy pass.
					//
					//How much more to do?
						dataSize = *pNumberOfSamplesToCopy;
						dataSize -= alreadyProcessedSamples;
						m_BufferSize_copyInCurrentPass = dataSize;
					}
					loopIt = NO;
				}
				else
				{
				//Client buffer is smaller than driver's one.
					if ( loopIt == NO )
					{
					//This is the first time we are here.
					//
					//Not enough data->we will have to do a multi pass copy.
						m_BufferSize_copyInCurrentPass = dataSize;
						loopIt = YES;
					}
					else
					{
					//This is the n-th time we are here.
					//Client's buffer is not big enough->we will do a multi pass copy.
					//
					//How much more to do?
						register unsigned long	advanceOffset;
						advanceOffset  = *pNumberOfSamplesToCopy;
						advanceOffset -= alreadyProcessedSamples;

						if ( advanceOffset <= dataSize )
						{
						//There is more or just right number of samples in client's buffer than we need to do -> stop looping.
							m_BufferSize_copyInCurrentPass = advanceOffset;
							dataSize = advanceOffset;
							loopIt = NO;
						}
						else
						{
						//There is not enough samples in client's buffer than we need to do -> continue looping.
							m_BufferSize_copyInCurrentPass = dataSize;
						}
					}
				}
				//pLogger->LogFast( "TR: CopyInCurrentPass: 0x%X\n", m_BufferSize_copyInCurrentPass );

			//Advance current StreamPointer.
				unsigned long	bufferSize_copyInCurrentPass_bytes = m_BufferSize_copyInCurrentPass;

				switch ( m_BitSize )
				{
				case CHANNEL_BIT_SIZE_32 :
					samplesToBytes32( bufferSize_copyInCurrentPass_bytes );
					break;

				case CHANNEL_BIT_SIZE_24 :
					samplesToBytes24( bufferSize_copyInCurrentPass_bytes );
					break;

				case CHANNEL_BIT_SIZE_16 :
				default :
					samplesToBytes16( bufferSize_copyInCurrentPass_bytes );
					break;
				}

			//Set data pointer.
			//
			//DX buffers are packed, so it's enough to set only one pointer.
				Set_Buffer_AVStream( (PULONG)pStreamPointer->Offset->Data );

			//Do the honky tonk.
			//
			//Notice1:
			//I had to protect it (don't remove this) -> this can happen if buffer is not nBlockAlign-ed.. ex. we only get 1 byte of a 2 byte sample.
			//
			//Notice:
			//If we are out of sync with the master clock we'll simulate copying and will try to sync the system buffer back with the card one.
				if ( m_BufferSize_copyInCurrentPass > 0 )
				{
					if ( m_Clock_samplesLost == 0 )
					{
					//Update addresses.
						Set_HardwareAddress( offset_inSamples + alreadyProcessedSamples/m_nChannels );		//offset is counted per 1 channels

					//Copy buffers.
						if ( m_pCopyBuffersProcedure != NULL )
						{
							//m_pCard->WriteWord( m_pCard->m_IoWave, SC4_BUS_WAIT_TIMER, 0 );

							m_pCopyBuffersProcedure( this );
						}
					}
					else if ( m_Type == CHANNEL_TYPE_IN )
					{
					//Clear streamPointer data, so that we wont get some random sounds on recording.
						RtlZeroMemory( pStreamPointer->Offset->Data, bufferSize_copyInCurrentPass_bytes );
					}
				}

			//Update writeOffset position in KSAUDIO_POSITION structure.
				if ( m_Clock_writeOffset_newStreamPointer == YES )
				{
					m_Clock_writeOffset.clock += pStreamPointer->Offset->Remaining;
					m_Clock_writeOffset_newStreamPointer = NO;

					newStreamPointer = YES;		//for timestamping
				}

			//Update clock... if buffer pointer will be moved to the end of the buffer signal that we'll get a new streamPointer.
				if ( pStreamPointer->Offset->Remaining == bufferSize_copyInCurrentPass_bytes )
				{
				//Signal that new stream pointer is on the way.
					m_Clock_writeOffset_newStreamPointer = YES;
				}

				//pLogger->LogFast( "TR: ClockMaster: 0x%.8I64X\n", clock_masterCard );

			//Advance buffer.
				if ( m_Type == CHANNEL_TYPE_OUT )
				{
				//Output.
					KsStreamPointerAdvanceOffsetsAndUnlock(
					  pStreamPointer,
					  bufferSize_copyInCurrentPass_bytes,
					  0,
					  FALSE
					);

				//Update clock -> playOffset is updated realtime in PropertyItem_Wave_Audio_Position_Handler_Get.
				}
				else
				{
				//Input.
				//
				//Set timeStamp in samples.
				//Notice:
				//Only set timestamp at the begining of the system buffer.
					if ( newStreamPointer == YES )
					{
					//Get when were this samples acquired by the card.
						PKSSTREAM_HEADER	pStreamHeader = pStreamPointer->StreamHeader;
						pStreamHeader->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_TIMEVALID | KSSTREAM_HEADER_OPTIONSF_DURATIONVALID;

						LONGLONG			clock_input = (LONGLONG)( clock_masterCard - pDriver->m_BufferSize );
						pStreamHeader->PresentationTime.Time = clock_input;
						pStreamHeader->PresentationTime.Numerator = 10000000;	// 1/100e-9
						pStreamHeader->PresentationTime.Denominator = sampleRate;

						unsigned long		durationInSamples = pStreamPointer->Offset->Remaining;
						switch ( m_BitSize )
						{
						case CHANNEL_BIT_SIZE_32 :
							bytesToSamples32( durationInSamples );
							break;

						case CHANNEL_BIT_SIZE_24 :
							bytesToSamples24( durationInSamples );
							break;

						case CHANNEL_BIT_SIZE_16 :
						default :
							bytesToSamples16( durationInSamples );
							break;
						}
						pStreamHeader->Duration = durationInSamples;

					//Reset flag...
						newStreamPointer = NO;
					}

				//Check if we need to update the masterTime.
					if ( loopIt == YES )
					{
					//We have to update it for the next loop.
						clock_masterCard += m_BufferSize_copyInCurrentPass;
					}

				//Update clock.
					m_Clock_playOffset.clock += bufferSize_copyInCurrentPass_bytes;

				//Advanced streampointer.
					KsStreamPointerAdvanceOffsetsAndUnlock(
					  pStreamPointer,
					  0,
					  bufferSize_copyInCurrentPass_bytes,
					  FALSE
					);
				}
				pStreamPointer = NULL;

			//Are we doing a multipart transfer?
				if ( loopIt == YES )
				{
					if ( m_BufferSize_copyInCurrentPass > 0 )
					{
					//Proceed.
						alreadyProcessedSamples += m_BufferSize_copyInCurrentPass;

					//If we came here than previous frame was finished->open next one.
						pStreamPointer = KsPinGetLeadingEdgeStreamPointer(
						  m_pPin,
						  KSSTREAM_POINTER_STATE_LOCKED
						);
						if ( pStreamPointer == NULL )
						{
						//We have just lost a bufferSize of samples... the ones that should be transfered to the card.
						//
						//Notice:
						//The code for real trasnfer is the same the only difference is if m_Clock_samplesLost is already 0 or not.
						//
							m_Clock_samplesLost -= alreadyProcessedSamples;				//we have done this ammount of samples already
							m_Clock_samplesLost += m_BufferSize_nChannels_inSamples;	//and the whole "real" buffer still missing
						}
					}
					//else
					//{
					////System supplied non nBlockAligned buffer, so if we lend here it's a system error and we can't do much about it.
					//}
				}
				else
				{
				//All data was transfered... now.. we're we copying resync data?
					if ( pNumberOfSamplesToCopy == &m_Clock_samplesLost )
					{
					//Yes -> prepare for real data transfer.
						m_Clock_samplesLost = 0;
						alreadyProcessedSamples = 0;

					//Get the "real" streamPointer.
						pStreamPointer = KsPinGetLeadingEdgeStreamPointer(
						  m_pPin,
						  KSSTREAM_POINTER_STATE_LOCKED
						);
						if ( pStreamPointer == NULL )
						{
						//We have just lost a bufferSize of samples... the ones that should be trasnfered to the card.
							m_Clock_samplesLost += pDriver->m_BufferSize;
						}
					}
				}
			} while ( pStreamPointer != NULL );

			//pLogger->LogFast( "TR: loop end\n" );

		//Set first buffer timeStamp.
			if ( m_Clock_timestamp_streamStart_isSet == NO )
			{
			//Set stream start timestamp.
			//
			//firstSampleInBuffer =
			// - input: 1 buffer size back
			// - output: 1 buffer size forward
			//
				if ( m_Type == CHANNEL_TYPE_IN )
				{
					clock_masterCard -= pDriver->m_BufferSize;
				}
				else
				{
					clock_masterCard += pDriver->m_BufferSize;
				}
				m_Clock_timestamp_streamStart.clock = clock_masterCard;
				//pLogger->LogFast( "TR: streamStart: 0x%.8I64X\n", clock_masterCard );

				m_Clock_timestamp_streamStart_isSet = YES;
			}
		}
		else if ( m_Type == CHANNEL_TYPE_OUT )
		{
		//Failed to get pStreamPointer->copy zeros to card buffer.
		//
		//We have to do this because some programs don't pause/stop the channel when they are not using it ->
		//they just stop suplying the pStreamPointer-> the card would loop the channel card buffer indefinitely-> well until pause/stop is pressed.
			//Setup variables.
			m_BufferSize_copyInCurrentPass = pDriver->m_BufferSize * m_nChannels;
			Set_HardwareAddress( offset_inSamples );

			//Zero out memory.
			ZeroChannelMemory();
		}

		//pLogger->LogFast( "\n" );
	}
	else if ( m_State == KSSTATE_PAUSE )
	{
	//If channel just moved to pause state get current timestamp.
		if ( m_Clock_timestamp_streamPause.clock == 1 )
		{
		//Yes current channel was just moved to pause state...
		//
		//Account for lost samples.
			if ( m_Clock_timestamp_streamStart_isSet == YES )		//Was stream started already?
			{
				m_Clock_samplesLost += clock_samplesLost * m_nChannels;

				//DbgPrint( "AV: account for lost samples (%d)\n", clock_samplesLost );

			//As we'll first try to resync the lost samples we need to simulate like we are really in the past and that everything is
			//working as it should... i.e. no lost samples.
				clock_masterCard -= ( m_Clock_samplesLost/m_nChannels );

			//Save master clock.
				m_Clock_timestamp_streamPause.clock = clock_masterCard;
			}
		}
	}
}

#pragma CODE_LOCKED
void channel_base_c::TransferData_ASIO
(
	unsigned long		offset_inSamples
)
{
//Is it an ASIO channel and in Running state?
	if (
	  ( m_Type_client == CHANNEL_TYPE_CLIENT_ASIO )
	  &&
	  ( m_State == KSSTATE_RUN )
	)
	{
	//Update addresses.
		Set_HardwareAddress( offset_inSamples );

	//Copy buffers.
		if ( m_pCopyBuffersProcedure != NULL )
		{
			m_pCopyBuffersProcedure( this );
		}
	}
}
