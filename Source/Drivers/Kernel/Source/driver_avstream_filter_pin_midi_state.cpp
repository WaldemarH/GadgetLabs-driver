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
#include "driver_avstream_filter_register.h"

#pragma DATA_LOCKED

//****************************************************************************
// ProcedureName:
// 		Pin_Process_Midi
//
// Explanation:
//		An AVStream minidriver's AVStrMiniPinProcess routine is called when
//		there is data available for a KSPIN structure to process.
//
//		Driver will use this function only for output pins where it will initiate the
//		data transfer.
//		If UART will be full before all the data will be transfered the output timer function
//		will take care of that.
//
//		For input we must define Pin_Process even if it isn't used.
//		...
//		"Further, if you specify KSPIN_FLAG_DO_NOT_INITIATE_PROCESSING and the pin
//		uses the standard transport mechanism, you must have a processing object.
//		This means there must be some process dispatch provided (either at
//		the filter level or at the pin level); even if this function is
//		never called, it must be provided in this circumstance."

// IRQ Level:
// 		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// Pin: IN
//		Pointer to the KSPIN structure for which state is changing.
//
// --------------------------------------------------------------------------
// Return:
// 		Should return STATUS_SUCCESS, STATUS_PENDING or an error message.
//
#pragma CODE_LOCKED
NTSTATUS driver_c::Pin_Process_Midi
(
	IN PKSPIN			pPin
)
{
	NTSTATUS			Status;

//Initialize variables.
	//Cards...
	card_c*				pCard = NULL;

	Status = pDriver->GetCard_FromPin( pPin, &pCard );

	//Channel...
	channel_midi_c*		pChannel_midi = NULL;

	if ( SUCCESS( Status ) )
	{
		Status = pCard->GetChannel_FromPin_Midi( pPin, &pChannel_midi );
	}

//Process input/output.
	if ( SUCCESS( Status ) )
	{
		if ( pChannel_midi->m_Type == channel_midi_c::CHANNEL_TYPE_IN )
		{
		//Input -> just return pending state -> ISR/DPC combination will handle all the input data.
			Status = STATUS_PENDING;
		}
		else
		{
		//Output -> request data trasnfer.
			//If something is wrong with the driver then we'll make sure that system wont freeze... that it can cancel the MIDI channels.
			Status = STATUS_PENDING;

			//Acquire channelInfo spinLock.
			KIRQL		spinLock_oldLevel;

			KeAcquireSpinLock( &pCard->m_SpinLock_channel_midi_out, &spinLock_oldLevel );

				if (
				  ( pChannel_midi->m_Type_client != channel_midi_c::CHANNEL_TYPE_CLIENT_NON )
				  &&
				  ( pChannel_midi->m_State == KSSTATE_RUN )
				)
				{
					if ( pChannel_midi->m_pCopyBuffersProcedure != NULL )
					{
						Status = (pChannel_midi->*pChannel_midi->m_pCopyBuffersProcedure)( 0 );
					}
				}

			//Release SPinLock.
			KeReleaseSpinLock( &pCard->m_SpinLock_channel_midi_out, spinLock_oldLevel );
		}
	}

	return Status;
}


//****************************************************************************
// ProcedureName:
// 		Pin_SetDeviceState_Midi
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
NTSTATUS driver_c::Pin_SetDeviceState_Midi
(
	IN PKSPIN	pPin,
	IN KSSTATE	ToState,
	IN KSSTATE	FromState
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
	pLogger->Log( "Pin->Id: %d\n", pPin->Id );
#endif

	NTSTATUS			Status;

//Initialize variables.
	//Cards...
	card_c*				pCard = NULL;

	Status = pDriver->GetCard_FromPin( pPin, &pCard );

	//Channel...
	channel_midi_c*		pChannel_midi = NULL;

	if ( SUCCESS( Status ) )
	{
		Status = pCard->GetChannel_FromPin_Midi( pPin, &pChannel_midi );
	}
#ifdef DBG_AND_LOGGER
	pLogger->Log(
	  logger_c::LOGGER_SEND_TO_BOTH,
	  logger_c::LS_Driver_AVStream_Pin_SetDeviceState,
	  &ToState,
	  &FromState,
	  NULL	//pChannel_midi
	);
#endif

	//Spinlock...
	PKSPIN_LOCK			pSpinLock = NULL;

	if ( SUCCESS( Status ) )
	{
		if ( pChannel_midi->m_Type == channel_midi_c::CHANNEL_TYPE_IN )
		{
		//Input...
			pSpinLock = &pCard->m_SpinLock_channel_midi_in;
		}
		else
		{
		//Output...
			pSpinLock = &pCard->m_SpinLock_channel_midi_out;
		}
	}

//Set State.
	if ( SUCCESS( Status ) )
	{
		BOOL		timerWasCanceled = FALSE;

	//Acquire channelInfo spinLock.
		KIRQL		spinLock_oldLevel;

		KeAcquireSpinLock( pSpinLock, &spinLock_oldLevel );

			switch (ToState)
			{
			case KSSTATE_STOP:
			{
			//Set channel state.
				pChannel_midi->m_State = KSSTATE_STOP;
				break;
			}
			case KSSTATE_ACQUIRE:
			{
			//Set channel state.
				pChannel_midi->m_State = KSSTATE_ACQUIRE;

			//Update syncStart class.
				if ( pChannel_midi->m_State == KSSTATE_STOP )
				{
				//From STOP state.
				//
				//Reset clock.
					//pChannel_midi->m_Timestamp_streamPause = 0;

					//pChannel_midi->m_Timestamp_streamStart_isSet = NO;
				}
				else
				{
				//From PAUSE state.
				//
				//Notice:
				//- ACQUIRE is in this driver STOP.
				//- We will stop every channel for it self.
				//
					if ( pChannel_midi->m_Type == channel_midi_c::CHANNEL_TYPE_IN )
					{
					//Stop input.
						pCard->Streaming_Midi_Stop();
					}
					else
					{
					//Stop output -> cancel timer.
						KeCancelTimer( &pCard->m_Timer_midi );
						timerWasCanceled = TRUE;

					//#ifdef GL_LOGGER
					//	pLogger->LogFast( "  Timer was canceled and stopped.\n" );
					//#endif
					}
				}
			//#ifdef GL_LOGGER
			//	pLogger->m_LogFast_save = YES;
			//#endif
				break;
			}
			case KSSTATE_PAUSE:
			{
			//Signal to DPC to record the pause timestamp.
				//if ( pChannel_midi->m_State == KSSTATE_RUN )
				//{
				////From RUN state.
				////
				////Signal to transfer routine to record the PAUSE timestamp.
				//	pChannel_midi->m_Clock_timestamp_streamPause.clock = 1;
				//}

			//Set channel state.
				pChannel_midi->m_State = KSSTATE_PAUSE;
				break;
			}
			case KSSTATE_RUN:
			{
			//Reset fast logger.
			//#ifdef GL_LOGGER
			//	pLogger->LogFast_Reset();
			//#endif

			//Set channel state.
				pChannel_midi->m_State = KSSTATE_RUN;

			//Initialize variables and start streaming.
				//pChannel_midi->m_Timestamp_streamStart = KeQueryPerformanceCounter( NULL ).QuadPart;

				if ( pChannel_midi->m_Type == channel_midi_c::CHANNEL_TYPE_IN )
				{
				//Input...
					pChannel_midi->m_Data_In.m_Data_size = 0;
					pChannel_midi->m_Data_In.m_Command = 0;
					pChannel_midi->m_Data_In.m_Command_old = 0;

				//Reset input FIFO.
					pCard->UART_Reset_FIFO_Input();

				//Start master card -> enable midi interrupts.
					pCard->Streaming_Midi_Start();
				}
				else
				{
				//Output...
					pChannel_midi->m_Data_Out_lastOffset = 0;

				//Reset output FIFO.
					pCard->UART_Reset_FIFO_Output();

				//Start timer/DPC thread.
				//
				//NOTICE!!!!!
				//MIDI output is handled at 2 different locations:
				//1. Pin_Process_Midi -> try to send data to UART as fast as it's received by the driver.
				//2. Timer function   -> send all data that was remained after the Pin_Process_Midi call
				//
					LARGE_INTEGER	tempInt;
					tempInt.QuadPart = 0;

					KeSetTimerEx( &pCard->m_Timer_midi, tempInt, 1, &pCard->m_DPCObject_midi_out );
				}
				break;
			}
			}

	//Release SPinLock.
		KeReleaseSpinLock( pSpinLock, spinLock_oldLevel );

	//Wait until timer DPC is finished... wait for 5ms... by that time it should be finished by a long time.
		if ( timerWasCanceled == TRUE )
		{
			pDriver->DelayThreadLong_PassiveLevel( delay_c::DELAY_THREAD_LONG_5ms );
		}
	}

#ifdef GL_LOGGER
	//pLogger->LogFast_Save();
	Procedure_Exit( Status );
#endif
	return Status;
}
