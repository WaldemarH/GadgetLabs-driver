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
#include "main.h"
#include "sync_start.h"


#pragma DATA_LOCKED

//We must define Pin_Process even if it isn't used.
//
//"Further, if you specify KSPIN_FLAG_DO_NOT_INITIATE_PROCESSING and the pin
//uses the standard transport mechanism, you must have a processing object.
//This means there must be some process dispatch provided (either at
//the filter level or at the pin level); even if this function is
//never called, it must be provided in this circumstance."
#pragma CODE_LOCKED
NTSTATUS driver_c::Pin_Process_Wave
(
	IN PKSPIN  Pin
)
{
	return STATUS_PENDING;
}

//****************************************************************************
// ProcedureName:
// 		Pin_SetDeviceState_Wave
//
// Explanation:
//		An AVStream minidriver's AVStrMiniPinSetDeviceState routine is called
//		when the state of a KSPIN structure is changed due to the arrival of
//		a connection state property ‘set’ IOCTL. Typically, this will be
//		provided by minidrivers that need to change the state of hardware.
//
// IRQ Level:
// 		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// Pin: IN
//		Pointer to the KSPIN structure for which state is changing.
//
// ToState: IN
//		The target KSSTATE after receipt of the IOCTL.
//
// FromState: IN
//		The previous KSSTATE.
//
// --------------------------------------------------------------------------
// Return:
// 		Should return STATUS_SUCCESS or an error message.
//
#pragma CODE_LOCKED
NTSTATUS driver_c::Pin_SetDeviceState_Wave
(
	IN PKSPIN	pPin,
	IN KSSTATE	ToState,
	IN KSSTATE	FromState
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
#endif

	NTSTATUS				Status;

//Initialize variables.
	//Cards...
	card_c*				pCard = NULL;
	card_c*				pCard_master = NULL;

	Status = pDriver->GetCard_FromPin( pPin, &pCard );
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
#ifdef DBG_AND_LOGGER
	pLogger->Log(
	  logger_c::LOGGER_SEND_TO_BOTH,
	  logger_c::LS_Driver_AVStream_Pin_SetDeviceState,
	  &ToState,
	  &FromState,
	  NULL	//pChannel_base
	);
#endif

	//Get pin's syncStart_s structure.
	syncStart_c*		pSyncStart = NULL;

	if ( SUCCESS( Status ) )
	{
		Status = pDriver->m_pSyncStart_master->GetSyncStart( &pSyncStart, pChannel_base );
	}

//Set State.
	if ( SUCCESS( Status ) )
	{
	//Acquire channelInfo spinLock.
		KIRQL		spinLock_oldLevel;

		KeAcquireSpinLock( &pDriver->m_SpinLock_channel_wave, &spinLock_oldLevel );

			switch (ToState) 
			{
			case KSSTATE_STOP:
			{
			//#ifdef GL_LOGGER
			//	pLogger->LogFast( "driver_c::Pin_SetDeviceState_Wave: KSSTATE_STOP\n" );
			//#endif

			//Update syncStart class.
				if ( pChannel_base->m_State == KSSTATE_ACQUIRE )
				{
				//From ACQUIRE state -> remove channel reference.
					pSyncStart->RemoveReference( pChannel_base );
				}

			//Set channel state.
				pChannel_base->m_State = KSSTATE_STOP;
				break;
			}
			case KSSTATE_ACQUIRE:
			{
			//#ifdef GL_LOGGER
			//	pLogger->LogFast( "driver_c::Pin_SetDeviceState_Wave: KSSTATE_ACQUIRE\n" );
			//#endif

			//Update syncStart class.
				if ( pChannel_base->m_State == KSSTATE_STOP )
				{
				//From STOP state.
				//
				//Update syncStart class.
					pSyncStart->AddReference( pChannel_base );

				//Reset clock.
					pChannel_base->m_Clock_playOffset.clock = 0;
					pChannel_base->m_Clock_writeOffset.clock = 0;
					pChannel_base->m_Clock_timestamp_streamPause.clock = 0;

					pChannel_base->m_Clock_timestamp_streamStart_isSet = NO;
					pChannel_base->m_Clock_writeOffset_newStreamPointer = YES;

					pChannel_base->m_Clock_samplesLost = 0;
				}
				else
				{
				//From PAUSE state.
				//
				//Notice:
				//- ACQUIRE is in this driver STOP.
				//- We will stop every channel for it self.
				//
				//Update syncStart class.
					pSyncStart->Channel_FromRunState();

				//Update numberOfChannelsInUse counter.
					if ( pChannel_base->m_Type == channel_base_c::CHANNEL_TYPE_IN )
					{
					//In
						--pCard->m_Channels_nInUse_INs_AVStream;
					}
					else
					{
					//Out
						--pCard->m_Channels_nInUse_OUTs_AVStream;
					}

				//Stop card interrupt if no more channels are playing.
					pCard_master->Streaming_Wave_Stop();
				}

			//Set channel state.
				pChannel_base->m_State = KSSTATE_ACQUIRE;
				break;
			}
			case KSSTATE_PAUSE:
			{
			#ifdef GL_LOGGER
				//pLogger->LogFast( "driver_c::Pin_SetDeviceState_Wave: KSSTATE_PAUSE\n" );
				pLogger->Log( logger_c::LOGGER_SEND_TO_DEBUG_PRINT, "KSSTATE_PAUSE\n" );
			#endif

				if ( pChannel_base->m_State == KSSTATE_RUN )
				{
				//From RUN state.
				//
				//Signal to transfer routine to record the PAUSE timestamp.
					pChannel_base->m_Clock_timestamp_streamPause.clock = 1;

				//Channel will not copy any stream any more -> mute output channel (clear samples in DAC card buffer).
					if ( pChannel_base->m_Type == channel_base_c::CHANNEL_TYPE_OUT )
					{
						pChannel_base->ZeroChannelMemory_Full();
					}
				}

			//Set channel state.
				pChannel_base->m_State = KSSTATE_PAUSE;
				break;
			}
			case KSSTATE_RUN:
			{
			//Update syncStart class.
				pSyncStart->Channel_ToRunState();

			////Reset max timer delay value.
			//#ifdef DBG_AND_LOGGER
			//	pCardClass->timerDelay = 0;
			//	pCardClass->timerDelay_performanceConter = 0;
			//#endif

			//Start streaming?
				if ( true == pSyncStart->Channels_StartStreaming() )
				{
				//Enable channels
				//
				//The next two variables will signal DPC to start processing this PIN.
				//The last one will enable copyBuffers part.
				//
				//Be careful as if channel goes RUN->PAUSE->RUN other channels will be already running ->
				//so enable only those that aren't enabled yet.
					channel_base_c*			pChannel_temp;
					channel_base_c**		ppChannel_temp = pSyncStart->m_ppChannels;

					while ( ( pChannel_temp = *ppChannel_temp ) != NULL )
					{
					//Signal DPC to start processing this PIN.
					//
					//Notice:
					//Be careful about RUN->PAUSE->RUN situation -> start only those pins which are in PAUSE state.
						if ( pChannel_temp->m_State == KSSTATE_PAUSE )
						{
							if ( pChannel_temp->m_Type == channel_base_c::CHANNEL_TYPE_IN )
							{
							//In
								++pChannel_temp->m_pCard->m_Channels_nInUse_INs_AVStream;
							}
							else
							{
							//Out
								++pChannel_temp->m_pCard->m_Channels_nInUse_OUTs_AVStream;
							}

						//Set channel state.
							pChannel_temp->m_State = KSSTATE_RUN;
						}

					//Next...
						++ppChannel_temp;
					}

				//Start master card -> enable wave interrupts.
					pCard_master->Streaming_Wave_Start();
				}
				break;
			}
			default:
				Status = STATUS_UNSUCCESSFUL;
				break;
			}

	//Release SPinLock.
		KeReleaseSpinLock( &pDriver->m_SpinLock_channel_wave, spinLock_oldLevel );
	}

//Should we release syncStart structure?
//
//We will relase every syncStart structure that doesn't include
//channels that are in pause or up state.
	if ( pSyncStart != NULL )
	{
		Status = pDriver->m_pSyncStart_master->FreeSyncStart( &pSyncStart );
	}

#ifdef GL_LOGGER
	//pLogger->LogFast_Save();
	Procedure_Exit( Status );
#endif
	return Status;
}