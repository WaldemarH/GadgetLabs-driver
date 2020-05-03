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
#include "notify_event.h"

#pragma DATA_LOCKED

//****************************************************************************
// ProcedureName:
// 		Pin_Create_Wave
//
// Explanation:
//		An AVStream minidriver's AVStrMiniPinCreate routine is called when a
//		pin is created. Typically, this routine is used by minidrivers that
//		want to initialize the context and resources associated with the pin.
//
// IRQ Level:
// 		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// Pin: IN
//		Pointer to the KSPIN that was just created.
//
// Irp: IN
//		Pointer to the IRP_MJ_CREATE for Pin.
//
// --------------------------------------------------------------------------
// Return:
// 		Should return STATUS_SUCCESS or an error message.
//
#pragma CODE_LOCKED
NTSTATUS driver_c::Pin_Create_Wave
(
	IN PKSPIN	pPin,
	IN PIRP		pIrp
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
#endif
//#ifdef GL_LOGGER
//	Logger_Print( LS_Pin_Create_1, Pin->ConnectionFormat );
//#endif

	NTSTATUS			Status;
	NTSTATUS			Status_locked = STATUS_UNSUCCESSFUL;

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

//#ifdef GL_LOGGER
//	if ( SUCCESS( Status ) )
//	{
//		Logger_Print( LS_Pin_Create_2, pChannelInfo );
//	}
//#endif

//Lock the driver (protect against deviceIoControl).
	if ( SUCCESS( Status ) )
	{
		Status = Status_locked = pDriver->LockDriver();
	}

//Is this channel available?
//
//If All channel is used then all the stereo channels are prohibited.
//Also if a stereo channel is used All channel is prohibited.
	if ( SUCCESS( Status ) )
	{
	//Acquire channel spinLock.
		KIRQL		spinLock_oldLevel;

		KeAcquireSpinLock( &pDriver->m_SpinLock_channel_wave, &spinLock_oldLevel );

		//Is channel in use?
			Status = pCard->IsChannelInUse_Wave( pChannel_base );
			if ( SUCCESS( Status ) )
			{
				pChannel_base->m_Type_client = channel_base_c::CHANNEL_TYPE_CLIENT_AVSTREAM;

				if ( pChannel_base->m_Type == channel_base_c::CHANNEL_TYPE_IN )
				{
					++pCard->m_Channels_opened_INs_AVStream;
				}
				else
				{
					++pCard->m_Channels_opened_OUTs_AVStream;
				}
			}

	//Release SPinLock.
		KeReleaseSpinLock( &pDriver->m_SpinLock_channel_wave, spinLock_oldLevel );
	}

//Initialize pin's pChannelInfo structure.
	PKSDATAFORMAT_WAVEFORMATEXTENSIBLE		pConnectionFormat = (PKSDATAFORMAT_WAVEFORMATEXTENSIBLE)pPin->ConnectionFormat;

	if ( SUCCESS( Status ) )
	{
	//Lock down the sampleRate.
		unsigned long		nSamplesPerSec = pConnectionFormat->WaveFormatEx.Format.nSamplesPerSec;

		switch ( nSamplesPerSec )
		{
		case 44100 :
		case 48000 :
			break;

		case 88200 :
		case 96000 :
		//Check if master and slave cards can handle this sample rate.
			Status = pCard_master->CanMasterAndSlaveCardsHandleRisedSampleRate();
			break;

		default:
			Status = STATUS_NO_MATCH;
			break;
		}

	//Protect sampleRate manipulation process -> this is already done by LockDriver.
	//
	//ASIO driver could lock the sampleRate until now.
	//If this has happened check if the sampleRates are the same.
		if ( SUCCESS( Status ) )
		{
			if ( pCard_master->m_SampleRate_owned == 0 )
			{
			//SampleRate is not locked yet -> set the new sampleRate.
				Status = pCard_master->SetSampleRate( nSamplesPerSec );
			}
			else
			{
			//SampleRate was already locked.
				if ( (unsigned long)pCard_master->m_SampleRate != nSamplesPerSec )
				{
				//The sampleRates are different -> fail.
					Status = STATUS_UNSUCCESSFUL;
				}
			}
			if ( SUCCESS( Status ) )
			{
			//Lock sampleRate to this channel also.
				++pCard_master->m_SampleRate_owned;

			//Send new sampleRate notification to ControlPanel.
			#ifdef DBG_AND_LOGGER
				pLogger->Log( "  pDriver->m_GUI_notifyEvent: 0x%p\n", pDriver->m_GUI_notifyEvent );
			#endif
				KIRQL		spinLock_oldLevel;

				KeAcquireSpinLock( &pDriver->m_GUI_notifyEvent_spinLock, &spinLock_oldLevel );

				//Is GUI notification set?
					if ( pDriver->m_GUI_notifyEvent != NULL )
					{
					//Yes -> set new event.
						pDriver->m_GUI_notifyEvent_master->SetEvent_SampleRate( pCard_master->m_SerialNumber, nSamplesPerSec );

					//Fire notification.
						KeSetEvent( pDriver->m_GUI_notifyEvent, EVENT_INCREMENT, FALSE );
					}

				KeReleaseSpinLock( &pDriver->m_GUI_notifyEvent_spinLock, spinLock_oldLevel );
			}
		}
	}

//Set channel.
	if ( SUCCESS( Status ) )
	{
		WORD		cbSize = pConnectionFormat->WaveFormatEx.Format.cbSize;
		WORD		nBlockAlign = pConnectionFormat->WaveFormatEx.Format.nBlockAlign;
		WORD		nChannels = pConnectionFormat->WaveFormatEx.Format.nChannels;

		switch ( nBlockAlign/nChannels )
		{
		case 2:
		{
		//2 byte sample container => sampleSize = 16 bit
		//
		//WAVEFORMATEX -> DX_MME_16bit
		//WAVEFORMATEXTENSIBLE -> DX_MME_16bit
		//
		//wBitsPerSample
			if ( pConnectionFormat->WaveFormatEx.Format.wBitsPerSample != 16 )
			{
				Status = STATUS_NO_MATCH;
			}

			if ( cbSize >= ( sizeof( WAVEFORMATEXTENSIBLE ) - sizeof( WAVEFORMATEX ) ) )
			{
			//WAVEFORMATEXTENSIBLE -> DX_MME_16bit
			//
			//wValidBitesPerSample
				if ( pConnectionFormat->WaveFormatEx.Samples.wValidBitsPerSample != 16 )
				{
					Status = STATUS_NO_MATCH;
				}
			}
			else if ( cbSize != 0 )
			{
				Status = STATUS_NO_MATCH;
			}

		//Update channelInfo
			if ( SUCCESS( Status ) )
			{
			//Acquire channelInfo spinLock.
				KIRQL		spinLock_oldLevel;

				KeAcquireSpinLock( &pDriver->m_SpinLock_channel_wave, &spinLock_oldLevel );

					Status = pChannel_base->Set_CopyBuffersProcedure_AVStream( nChannels, channel_base_c::CHANNEL_BIT_SIZE_16 );
					if ( SUCCESS( Status ) )
					{
					//Pin
						Status = pChannel_base->Set_AVStream_Pin( pPin );
					}

			//Release channelInfo spinLock.
				KeReleaseSpinLock( &pDriver->m_SpinLock_channel_wave, spinLock_oldLevel );
			}
			break;
		}
		case 3 :
		{
		//3 byte sample container => sampleSize = 24 bit
		//
		//WAVEFORMATEX -> DX_MME_24bit
		//WAVEFORMATEXTENSIBLE -> DX_MME_24bit
		//
		//wBitsPerSample
			if ( pConnectionFormat->WaveFormatEx.Format.wBitsPerSample != 24 )
			{
				Status = STATUS_NO_MATCH;
			}
		
			if ( cbSize >= ( sizeof( WAVEFORMATEXTENSIBLE ) - sizeof( WAVEFORMATEX ) ) )
			{
			//WAVEFORMATEXTENSIBLE -> DX_MME_24bit
			//
			//wValidBitesPerSample
				if ( pConnectionFormat->WaveFormatEx.Samples.wValidBitsPerSample != 24 )
				{
					Status = STATUS_NO_MATCH;
				}
			}
			else if ( cbSize != 0 )
			{
				Status = STATUS_NO_MATCH;
			}
		
		//Update channelInfo
			if ( SUCCESS( Status ) )
			{
			//Acquire channelInfo spinLock.
				KIRQL		spinLock_oldLevel;

				KeAcquireSpinLock( &pDriver->m_SpinLock_channel_wave, &spinLock_oldLevel );

					Status = pChannel_base->Set_CopyBuffersProcedure_AVStream( nChannels, channel_base_c::CHANNEL_BIT_SIZE_24 );
					if ( SUCCESS( Status ) )
					{
					//Pin
						Status = pChannel_base->Set_AVStream_Pin( pPin );
					}

			//Release channelInfo spinLock.
				KeReleaseSpinLock( &pDriver->m_SpinLock_channel_wave, spinLock_oldLevel );
			}
			break;
		}
		case 4 :
		{
		//4 byte sample container => sampleSize = 8,16,24 bit
		//
		//WAVEFORMATEX -> DX_MME_32bit
		//WAVEFORMATEXTENSIBLE -> DX_MME_32bit
		//
		//wBitsPerSample
			if ( pConnectionFormat->WaveFormatEx.Format.wBitsPerSample != 32 )
			{
				Status = STATUS_NO_MATCH;
			}

			if ( cbSize >= ( sizeof( WAVEFORMATEXTENSIBLE ) - sizeof( WAVEFORMATEX ) ) )
			{
			//WAVEFORMATEXTENSIBLE -> DX_MME_32bit
			//
			//wValidBitesPerSample
				if ( pConnectionFormat->WaveFormatEx.Samples.wValidBitsPerSample > 24 )
				{
					Status = STATUS_NO_MATCH;
				}
			}
			else if ( cbSize != 0 )
			{
				Status = STATUS_NO_MATCH;
			}

		//Update channelInfo
			if ( SUCCESS( Status ) )
			{
			//Acquire channelInfo spinLock.
				KIRQL		spinLock_oldLevel;

				KeAcquireSpinLock( &pDriver->m_SpinLock_channel_wave, &spinLock_oldLevel );

					Status = pChannel_base->Set_CopyBuffersProcedure_AVStream( nChannels, channel_base_c::CHANNEL_BIT_SIZE_32 );
					if ( SUCCESS( Status ) )
					{
					//Pin
						Status = pChannel_base->Set_AVStream_Pin( pPin );
					}

			//Release channelInfo spinLock.
				KeReleaseSpinLock( &pDriver->m_SpinLock_channel_wave, spinLock_oldLevel );
			}
			break;
		}
		default:
			Status = STATUS_NO_MATCH;
			break;
		}
	}

//Was all ok? If not we need to release the channel.
	if ( FAILED( Status ) )
	{
	//Acquire channelInfo spinLock.
		KIRQL		spinLock_oldLevel;

		KeAcquireSpinLock( &pDriver->m_SpinLock_channel_wave, &spinLock_oldLevel );

			if ( pChannel_base->m_Type_client == channel_base_c::CHANNEL_TYPE_CLIENT_AVSTREAM )
			{
				pChannel_base->m_Type_client = channel_base_c::CHANNEL_TYPE_CLIENT_NON;

				if ( pChannel_base->m_Type == channel_base_c::CHANNEL_TYPE_IN )
				{
					--pCard->m_Channels_opened_INs_AVStream;
				}
				else
				{
					--pCard->m_Channels_opened_OUTs_AVStream;
				}
			}

	//Release SPinLock.
		KeReleaseSpinLock( &pDriver->m_SpinLock_channel_wave, spinLock_oldLevel );
	}

//Send number of channels opened notification.
	if ( SUCCESS( Status ) )
	{
		KIRQL		spinLock_oldLevel;

		KeAcquireSpinLock( &pDriver->m_GUI_notifyEvent_spinLock, &spinLock_oldLevel );

		//Set event.
			pDriver->m_GUI_notifyEvent_master->SetEvent_ChannelsOpened(
				pCard->m_SerialNumber,
				pCard->m_Channels_opened_INs_ASIO,
				pCard->m_Channels_opened_OUTs_ASIO,
				pCard->m_Channels_opened_INs_AVStream,
				pCard->m_Channels_opened_OUTs_AVStream
			);

		//Fire notification.
			if ( pDriver->m_GUI_notifyEvent != NULL )
			{
				KeSetEvent( pDriver->m_GUI_notifyEvent, EVENT_INCREMENT, FALSE );
			}

		KeReleaseSpinLock( &pDriver->m_GUI_notifyEvent_spinLock, spinLock_oldLevel );
	}

//Unlock the card.
	if ( SUCCESS( Status_locked ) )
	{
		pDriver->UnlockDriver();
	}

#ifdef DBG_AND_LOGGER
//	if ( SUCCESS( Status ) )
//	{
//		Logger_Print( LS_Pin_Create_3, pChannelInfo	);
//	}
	Procedure_Exit( Status );
#endif
	return Status;
}

//****************************************************************************
// ProcedureName:
// 		Pin_Close_Wave
//
// Explanation:
//		An AVStream minidriver's AVStrMiniPinClose routine is called when a
//		pin is closed. It usually is provided by minidrivers that want to free
//		the context and resources associated with the pin.
//
// IRQ Level:
// 		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// Pin: IN
//		Pointer to the KSPIN that was just closed.
//
// Irp: IN
//		Pointer to the IRP_MJ_CLOSE for Pin.
//
// --------------------------------------------------------------------------
// Return:
// 		Should return STATUS_SUCCESS or an error message.
//
#pragma CODE_LOCKED
NTSTATUS driver_c::Pin_Close_Wave
(
	IN PKSPIN	pPin,
	IN PIRP		pIrp
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
#endif

	NTSTATUS			Status;
	NTSTATUS			Status_locked = STATUS_UNSUCCESSFUL;

//	card_c*				pCardClass;
//	card_c*				pCardClass_master;
//	channelInfo_s*		pChannelInfo;
//	KIRQL				spinLock_oldLevel;

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

//#ifdef GL_LOGGER
//	if ( SUCCESS( Status ) )
//	{
//		Logger_Print( LS_Pin_Close_1, pChannelInfo );
//	}
//#endif

//Lock the driver (protect against deviceIoControl).
	if ( SUCCESS( Status ) )
	{
		Status = Status_locked = pDriver->LockDriver();
	}

//Test if it's really a WDM channel and free it's resources.
	if ( SUCCESS( Status ) )
	{
	//Acquire channelInfo spinLock.
		KIRQL		spinLock_oldLevel;

		KeAcquireSpinLock( &pDriver->m_SpinLock_channel_wave, &spinLock_oldLevel );

		//Is this channel really a WDM channel?
			if ( pChannel_base->m_Type_client == channel_base_c::CHANNEL_TYPE_CLIENT_AVSTREAM )
			{
			//Free pin if it's in KSSTATE_STOP state only.
				if ( pChannel_base->m_State == KSSTATE_STOP )
				{
				//It is -> free resources.
					//copyBufferProc
					pChannel_base->m_pCopyBuffersProcedure = NULL;

					//pin
					pChannel_base->Set_AVStream_Pin( NULL );

					//typeClient -> release it.
					pChannel_base->m_Type_client = channel_base_c::CHANNEL_TYPE_CLIENT_NON;

					if ( pChannel_base->m_Type == channel_base_c::CHANNEL_TYPE_IN )
					{
						--pCard->m_Channels_opened_INs_AVStream;
					}
					else
					{
						--pCard->m_Channels_opened_OUTs_AVStream;
					}

					//Unlock sample rate.
					--pCard_master->m_SampleRate_owned;
				}
				else
				{
					Status = STATUS_INVALID_DEVICE_STATE;
				}
			}
			else
			{
				Status = STATUS_UNSUCCESSFUL;
			}

	//Release SPinLock.
		KeReleaseSpinLock( &pDriver->m_SpinLock_channel_wave, spinLock_oldLevel );
	}

//Send number of channels opened notification.
	if ( SUCCESS( Status ) )
	{
		KIRQL		spinLock_oldLevel;

		KeAcquireSpinLock( &pDriver->m_GUI_notifyEvent_spinLock, &spinLock_oldLevel );

		//Set event.
			pDriver->m_GUI_notifyEvent_master->SetEvent_ChannelsOpened(
				pCard->m_SerialNumber,
				pCard->m_Channels_opened_INs_ASIO,
				pCard->m_Channels_opened_OUTs_ASIO,
				pCard->m_Channels_opened_INs_AVStream,
				pCard->m_Channels_opened_OUTs_AVStream
			);

		//Fire notification.
			if ( pDriver->m_GUI_notifyEvent != NULL )
			{
				KeSetEvent( pDriver->m_GUI_notifyEvent, EVENT_INCREMENT, FALSE );
			}

		KeReleaseSpinLock( &pDriver->m_GUI_notifyEvent_spinLock, spinLock_oldLevel );
	}

//Unlock the card.
	if ( SUCCESS( Status_locked ) )
	{
		pDriver->UnlockDriver();
	}

#ifdef DBG_AND_LOGGER
//	if ( SUCCESS( Status ) )
//	{
//		Logger_Print( LS_Pin_Close_2, pChannelInfo );
//	}
	Procedure_Exit( Status );
#endif
	return Status;
}