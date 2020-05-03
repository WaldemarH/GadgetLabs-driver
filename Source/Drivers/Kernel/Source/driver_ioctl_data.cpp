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
//		driver.cpp
//
// PURPOSE:
//		File contains the class of all classes.
//
// STATUS:
//		Unfinished/Untested.
//
//***************************************************************************
//***************************************************************************
#include "main.h"
#include "driver_ioctl_data.h"
#include "notify_event.h"
#include "card_altera_firmware.h"


#pragma DATA_LOCKED

#pragma CODE_LOCKED
NTSTATUS driver_c::Data_BufferSize
(
	IN OUT unsigned long*		pSystemBuffer,
	IN unsigned long			bufferLength_input,
	IN unsigned long			bufferLength_output,
	IN OUT unsigned long*		pInformationSize
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
#endif

	NTSTATUS		Status = STATUS_SUCCESS;
	NTSTATUS		Status_locked = STATUS_UNSUCCESSFUL;

//Validate.
	if (
	  ( bufferLength_input != sizeof( unsigned long ) )
	  ||
	  ( bufferLength_output != sizeof( unsigned long ) )
	)
	{
		Status = STATUS_INVALID_BUFFER_SIZE;
	}

	if ( SUCCESS( Status ) )
	{
		switch ( *pSystemBuffer )
		{
		case 0:
		case 0x20:
		case 0x40:
		case 0x80:
		case 0x100:
		case 0x200:
		case 0x400:
			break;
		default:
			Status = STATUS_INVALID_PARAMETER;
			break;
		}
	}

//Do the honky tonk.
	if ( SUCCESS( Status ) )
	{
	//Lock driver.
		Status = Status_locked = LockDriver();
	}

	if ( SUCCESS( Status ) )
	{
		if ( *pSystemBuffer == 0 )
		{
		//Get buffer size.
			*pSystemBuffer = m_BufferSize;
		}
		else
		{
		//Set buffer size.
			KIRQL		spinLock_oldLevel;

			KeAcquireSpinLock( &m_SpinLock_channel_wave, &spinLock_oldLevel );

			//Are any channels opened?
				BOOL	channelsInUse = FALSE;
				for ( int i = 0; i < ELEMENTS_IN_ARRAY(m_Cards); i++ )
				{
					card_c*		pCard = m_Cards[i];
					if ( pCard != NULL )
					{
						channelsInUse = pCard->IsChannelInUse_Wave();
						if ( channelsInUse != FALSE )
						{
							break;
						}
					}
				}

			//Set new buffer size.
				if ( channelsInUse == FALSE )
				{
					SetBufferSize( *pSystemBuffer );
				}
				else
				{
				//Failed...
					*pSystemBuffer = 0;
				}

			KeReleaseSpinLock( &m_SpinLock_channel_wave, spinLock_oldLevel );
		}

		*pInformationSize = sizeof( unsigned long );
	}

	if ( SUCCESS( Status_locked ) )
	{
		UnlockDriver();
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

#pragma CODE_LOCKED
NTSTATUS driver_c::Data_EngineVersion
(
	IN OUT unsigned long*		pSystemBuffer,
	IN unsigned long			bufferLength_input,
	IN unsigned long			bufferLength_output,
	IN OUT unsigned long*		pInformationSize
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
#endif

	NTSTATUS		Status = STATUS_SUCCESS;

//Validate.
	if (
	  ( bufferLength_input != 0 )
	  ||
	  ( bufferLength_output != sizeof( unsigned long ) )
	)
	{
		Status = STATUS_INVALID_BUFFER_SIZE;
	}

//Do the honky tonk.
	if ( SUCCESS( Status ) )
	{
		*pSystemBuffer = IOCTL_DATA_ENGINE_VERSION;
		*pInformationSize = sizeof( unsigned long );
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

#pragma CODE_LOCKED
NTSTATUS driver_c::Data_CardsInformation
(
	IN OUT void*				pSystemBuffer,
	IN unsigned long			bufferLength_input,
	IN unsigned long			bufferLength_output,
	IN OUT unsigned long*		pInformationSize
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
#endif

	NTSTATUS		Status = STATUS_SUCCESS;
	NTSTATUS		Status_locked = STATUS_UNSUCCESSFUL;

//Validate.
	if (
	  ( bufferLength_input != 0 )
	  ||
	  ( bufferLength_output != sizeof( ioctl_cardInfo_s[MAX_CARDS] ) )
	)
	{
		Status = STATUS_INVALID_BUFFER_SIZE;
	}

//Do the honky tonk.
	if ( SUCCESS( Status ) )
	{
	//Lock driver.
		Status = Status_locked = LockDriver();
	}

	if ( SUCCESS( Status ) )
	{
		for ( unsigned long i = 0; i < ELEMENTS_IN_ARRAY( m_Cards ); i++ )
		{
			card_c*					pCard = m_Cards[i];
			ioctl_cardInfo_s*		pCardInfo = &( (ioctl_cardInfo_s*)pSystemBuffer )[i];

			if ( pCard != NULL )
			{
			//Copy data.
				pCardInfo->m_Type = (ioctl_cardInfo_s::CARD_TYPE)pCard->m_Type;
				pCardInfo->m_Type_subVersion = pCard->m_Type_subVersion;
				pCardInfo->m_SampleRate = pCard->m_SampleRate;
				pCardInfo->m_ClockSource = (ioctl_cardInfo_s::CLOCK_SOURCE)pCard->m_ClockSource;
				pCardInfo->m_SerialNumber = pCard->m_SerialNumber;
				pCardInfo->m_SerialNumber_masterCard = pCard->m_SerialNumber_masterCard;
				pCardInfo->m_SerialNumber_slaveCard = pCard->m_SerialNumber_slaveCard;
				pCardInfo->m_Channels_opened_INs_ASIO = pCard->m_Channels_opened_INs_ASIO;
				pCardInfo->m_Channels_opened_OUTs_ASIO = pCard->m_Channels_opened_OUTs_ASIO;
				pCardInfo->m_Channels_opened_INs_AVStream = pCard->m_Channels_opened_INs_AVStream;
				pCardInfo->m_Channels_opened_OUTs_AVStream = pCard->m_Channels_opened_OUTs_AVStream;
			}
			else
			{
			//Card doesn't exist -> clear the card ioctl_cardInfo_s struct.
				pCardInfo->m_Type = ioctl_cardInfo_s::CARD_TYPE_NOT_SET;
				pCardInfo->m_Type_subVersion = 0;
				pCardInfo->m_SampleRate = 0;
				pCardInfo->m_ClockSource = ioctl_cardInfo_s::CLOCK_SOURCE_NOT_SET;
				pCardInfo->m_SerialNumber = (unsigned char)-1;
				pCardInfo->m_SerialNumber_masterCard = 0;
				pCardInfo->m_SerialNumber_slaveCard = 0;
				pCardInfo->m_Channels_opened_INs_ASIO = 0;
				pCardInfo->m_Channels_opened_OUTs_ASIO = 0;
				pCardInfo->m_Channels_opened_INs_AVStream = 0;
				pCardInfo->m_Channels_opened_OUTs_AVStream = 0;
			}
		}

		*pInformationSize = sizeof( ioctl_cardInfo_s[MAX_CARDS] );
	}

	if ( SUCCESS( Status_locked ) )
	{
		UnlockDriver();
	}

#ifdef DBG_AND_LOGGER
	pLogger->Log(
	  logger_c::LOGGER_SEND_TO_LOG_FILE,
	  logger_c::LS_IOCTL_DATA_CardsInformation,
	  pSystemBuffer
	);
	Procedure_Exit( Status );
#endif
	return Status;
}

#pragma CODE_LOCKED
NTSTATUS driver_c::Data_ChannelProperty
(
	IN OUT void*				pSystemBuffer,
	IN unsigned long			bufferLength_input,
	IN unsigned long			bufferLength_output,
	IN OUT unsigned long*		pInformationSize
)
{
//#ifdef DBG_AND_LOGGER
//	Procedure_Start_Global();
//#endif

	NTSTATUS							Status = STATUS_SUCCESS;

	ioctl_channelProperty_header_s*		pHeader = (ioctl_channelProperty_header_s*)pSystemBuffer;
	card_c*								pCard;
	unsigned long						size;

//Validate.
	if (
	  ( bufferLength_input < sizeof( ioctl_channelProperty_header_s ) )
	  ||
	  ( bufferLength_output < sizeof( ioctl_channelProperty_header_s ) )
	)
	{
		Status = STATUS_INVALID_BUFFER_SIZE;
	}
	if ( SUCCESS( Status ) )
	{
		size = sizeof( ioctl_channelProperty_header_s ) + pHeader->m_nPropertyDefinitions*sizeof( ioctl_channelProperty_s );

		if (
		  ( pHeader->m_nPropertyDefinitions > 32 )
		  ||
		  ( bufferLength_input != size )
		  ||
		  ( bufferLength_output != size )
		)
		{
			Status = STATUS_INVALID_BUFFER_SIZE;
		}
	}

	if ( SUCCESS( Status ) )
	{
		if ( pHeader->m_SerialNumber > MAX_CARDS )
		{
			Status = STATUS_INVALID_PARAMETER;
		}
	}
	if ( SUCCESS( Status ) )
	{
		pCard = m_Cards[pHeader->m_SerialNumber];
		if ( pCard == NULL )
		{
			Status = STATUS_INVALID_PARAMETER;
		}
	}

//Do the honky tonk.
	if ( SUCCESS( Status ) )
	{
		KIRQL				spinLock_oldLevel;

		KeAcquireSpinLock( &m_SpinLock_channel_wave, &spinLock_oldLevel );

			ioctl_channelProperty_s*		pProperty = (ioctl_channelProperty_s*)( pHeader+1 );

			if ( pHeader->m_Request == PROPERTY_GET_PROPERTY )
			{
			//Get property settings.
				for ( int i = 0; i < pHeader->m_nPropertyDefinitions; i++ )
				{
					if ( pProperty->m_Property != PROPERTY_TYPE_UNDEFINED )
					{
						pCard->GetChannelProperty( pProperty );

					//Do we have to handle m_ControlEnabled also?
					//
					//Notice:
					//This is done only on monitoring requests.
						if ( pProperty->m_Property == PROPERTY_TYPE_MONITORING )
						{
						//Set default value.
							pProperty->m_ControlEnabled = PROPERTY_CONTROL_ENABLED;

							if ( pProperty->m_SerialNumber < 2*MAX_STEREO_PAIRS_PER_CARD )
							{
							//Is this an ASIO channel?
								channel_stereo_c*		pChannel = pCard->m_pChannels_Stereo_INs[pProperty->m_SerialNumber/2];
								if ( pChannel != NULL )
								{
									if ( pChannel->m_Type_client == channel_base_c::CHANNEL_TYPE_CLIENT_ASIO )
									{
									//It's an ASIO channel -> disable checkBox.
										pProperty->m_ControlEnabled = PROPERTY_CONTROL_DISABLED;
									}
									else
									{
									//It's not an ASIO channel -> enable checkBox.
										pProperty->m_ControlEnabled = PROPERTY_CONTROL_ENABLED;
									}
								}
							}
						}
					}

				//Next...
					++pProperty;
				}
			}
			else
			{
			//Set property settings.
				for ( int i = 0; i < pHeader->m_nPropertyDefinitions; i++ )
				{
					if ( pProperty->m_Property != PROPERTY_TYPE_UNDEFINED )
					{
						if ( pProperty->m_Property != PROPERTY_TYPE_MONITORING )
						{
						//All property requests exept monitoring.
							pCard->SetChannelProperty( pProperty );
						}
						else
						{
						//It's a monitoring request.
						//
						//If this is an ASIO channel monitoring requests can only come from ASIO driver... GUI requests are ignored.
							if ( pProperty->m_SerialNumber < 2*MAX_STEREO_PAIRS_PER_CARD )
							{
							//Is this an ASIO channel?
								channel_stereo_c*		pChannel = pCard->m_pChannels_Stereo_INs[pProperty->m_SerialNumber/2];
								if ( pChannel != NULL )
								{
									if ( pChannel->m_Type_client == channel_base_c::CHANNEL_TYPE_CLIENT_ASIO )
									{
									//It's an ASIO channel... monitoring request can only come from ASIO driver.
										if ( pHeader->m_RequestBy == PROPERTY_REQUEST_BY_ASIO )
										{
											pCard->SetChannelProperty( pProperty );
										}
									}
									else
									{
									//It's not an ASIO channel -> monitoring request allowed.
										pCard->SetChannelProperty( pProperty );
									}
								}
							}
						}
					}

				//Next...
					++pProperty;
				}
			}

		KeReleaseSpinLock( &m_SpinLock_channel_wave, spinLock_oldLevel );

		*pInformationSize = size;
	}

//#ifdef DBG_AND_LOGGER
//	Procedure_Exit( Status );
//#endif
	return Status;
}

#pragma CODE_LOCKED
NTSTATUS driver_c::Data_SampleRate
(
	IN OUT void*			pSystemBuffer,
	IN unsigned long		bufferLength_input,
	IN unsigned long		bufferLength_output,
	IN OUT unsigned long*	pInformationSize
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
#endif

	NTSTATUS					Status = STATUS_SUCCESS;
	NTSTATUS					Status_locked = STATUS_UNSUCCESSFUL;

	card_c*						pCard;
	card_c*						pCard_master;
	ioctl_setSampleRate_s*		pSampleRate = (ioctl_setSampleRate_s*)pSystemBuffer;

//Validate.
	if (
	  ( bufferLength_input != sizeof( ioctl_setSampleRate_s ) )
	  ||
	  ( bufferLength_output != sizeof( ioctl_setSampleRate_s ) )
	)
	{
		Status = STATUS_INVALID_BUFFER_SIZE;
	}

	if ( SUCCESS( Status ) )
	{
		if ( pSampleRate->m_SerialNumber > MAX_CARDS )
		{
			Status = STATUS_INVALID_PARAMETER;
		}
	}
	if ( SUCCESS( Status ) )
	{
		pCard = m_Cards[pSampleRate->m_SerialNumber];
		if ( pCard == NULL )
		{
			Status = STATUS_INVALID_PARAMETER;
		}
	}
	if ( SUCCESS( Status ) )
	{
		pCard_master = pCard->GetMasterCard();

		switch ( pSampleRate->m_SampleRate )
		{
		case 0:		//get sample rate...
		case 44100:
		case 48000:
		{
			break;
		}
		case 88200:
		case 96000:
		{
		//Check if master and slave cards can handle this sample rate.
			Status = pCard_master->CanMasterAndSlaveCardsHandleRisedSampleRate();
			if ( FAILED( Status ) )
			{
				Status = STATUS_INVALID_PARAMETER;
			}
			break;
		}
		default:
		{
			Status = STATUS_INVALID_PARAMETER;
			break;
		}
		}
	}

//Do the honky tonk.
	if ( SUCCESS( Status ) )
	{
	//Lock driver.
		Status = Status_locked = LockDriver();
	}

	if ( SUCCESS( Status ) )
	{

		if ( pSampleRate->m_SampleRate == 0 )
		{
		//Get sampleRate.
			pSampleRate->m_SampleRate = pCard->m_SampleRate;
		#ifdef DBG_AND_LOGGER
			pLogger->Log( "  Get sampleRate: %d\n", pSampleRate->m_SampleRate );
		#endif
		}
		else
		{
		//Set sampleRate.
		#ifdef DBG_AND_LOGGER
			pLogger->Log( "  Set sampleRate: %d (old: %d)\n", pSampleRate->m_SampleRate, pCard_master->m_SampleRate );
		#endif

			if ( pCard_master->m_SampleRate != pSampleRate->m_SampleRate )
			{
			#ifdef DBG_AND_LOGGER
				pLogger->Log( "  New sampleRate: %d\n", pSampleRate->m_SampleRate );
			#endif

			//Is this sample rate owned?
			//
			//As soon as ASIO will acquire a group of channels sampleRate will get locked to that group.
			//So if only 1 owns the sampleRate it means it's current ASIO channel group.. so we can change the sampleRate.
				if ( pCard_master->m_SampleRate_owned == 1 )
				{
				//It's owned by ASIO channel group.
				#ifdef DBG_AND_LOGGER
					pLogger->Log( "  New sampleRate will be set.\n" );
				#endif

				//Unlock sampleRate.
					--pCard_master->m_SampleRate_owned;

				//Set sampleRate.
					Status = pCard_master->SetSampleRate( pSampleRate->m_SampleRate );

				//Lock sampleRate.
					++pCard_master->m_SampleRate_owned;

				//Send new sampleRate notification to ControlPanel.
					if ( SUCCESS( Status ) )
					{
					#ifdef DBG_AND_LOGGER
						pLogger->Log( "  pDriver->m_GUI_notifyEvent: 0x%p\n", pDriver->m_GUI_notifyEvent );
					#endif

						KIRQL		spinLock_oldLevel;

						KeAcquireSpinLock( &m_GUI_notifyEvent_spinLock, &spinLock_oldLevel );

						//Is GUI notification set?
							if ( pDriver->m_GUI_notifyEvent != NULL )
							{
							//Yes -> set new event.
								m_GUI_notifyEvent_master->SetEvent_SampleRate( pCard_master->m_SerialNumber, pSampleRate->m_SampleRate );

							//Fire notification.
								KeSetEvent( m_GUI_notifyEvent, EVENT_INCREMENT, FALSE );
							}

						KeReleaseSpinLock( &m_GUI_notifyEvent_spinLock, spinLock_oldLevel );
					}
					else
					{
						pSampleRate->m_SampleRate = 0;	//error
					}
				}
				else
				{
				//The samplerate is locked to more than 1 channel.
				#ifdef DBG_AND_LOGGER
					pLogger->Log( "  SampleRate locked to more then 1 channel/group.\n" );
				#endif

					pSampleRate->m_SampleRate = 0;	//error
				}
			}
		}

		*pInformationSize = sizeof( ioctl_setSampleRate_s );
	}

	if ( SUCCESS( Status_locked ) )
	{
		UnlockDriver();
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

#pragma CODE_LOCKED
NTSTATUS driver_c::Data_SyncCards
(
	IN OUT unsigned long*		pSystemBuffer,
	IN unsigned long			bufferLength_input,
	IN unsigned long			bufferLength_output
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
#endif

	NTSTATUS					Status = STATUS_SUCCESS;
	NTSTATUS					Status_locked = STATUS_UNSUCCESSFUL;

//Validate.
	if (
	  ( bufferLength_input != sizeof( unsigned long ) )
	  ||
	  ( bufferLength_output != 0 )
	)
	{
		Status = STATUS_INVALID_BUFFER_SIZE;
	}

//Do the honky tonk.
	if ( SUCCESS( Status ) )
	{
	//Lock driver.
		Status = Status_locked = LockDriver();
	}

	if ( SUCCESS( Status ) )
	{
	//Are any channels in use?
		for ( int i = 0; i < ELEMENTS_IN_ARRAY(m_Cards); i++ )
		{
			card_c*		pCard = m_Cards[i];
			if ( pCard != NULL )
			{
				if ( FALSE != pCard->IsChannelInUse_Wave() )
				{
				#ifdef DBG_AND_LOGGER
					pLogger->Log( "  SYNC: At least 1 channel in use... no more honky tonk.\n" );
				#endif

				//There is at least 1 channel in use... no more honky tonk.
					Status = STATUS_UNSUCCESSFUL;
					break;
				}
			}
		}
	}
	if ( SUCCESS( Status ) )
	{
	//Check card sync.
		Data_SetCardsOrder();

	//Is it sync?
		if ( *pSystemBuffer != 0 )
		{
		//Sync...
		//
		//Notice:
		//We wont be sending any sampleRate notifications to GUI as when this function will return it will refresh all cards data anyway.
		#ifdef DBG_AND_LOGGER
			pLogger->Log( "  Sync\n" );
		#endif

		//Software sync...
			BOOL	wasThereASync = FALSE;

			for ( int i = 0; i < ELEMENTS_IN_ARRAY( m_Cards ); i++ )
			{
				card_c*		pCard = m_Cards[i];
				if ( pCard != NULL )
				{
					if ( pCard->m_SerialNumber_masterCard > 0 )
					{
						pCard->m_pCard_master = m_Cards[pCard->m_SerialNumber_masterCard-1];

						wasThereASync = TRUE;
					}
					if ( pCard->m_SerialNumber_slaveCard > 0 )
					{
						pCard->m_pCard_slave = m_Cards[pCard->m_SerialNumber_slaveCard-1];

						wasThereASync = TRUE;
					}
				}
			}
			if ( wasThereASync == FALSE )
			{
				Status = STATUS_UNSUCCESSFUL;
			}

		//Hardware sync...
			if ( SUCCESS( Status ) )
			{
			//Set master card's sampleRate to 44.1kHz and sync slave cards.
				for ( int i = 0; i < ELEMENTS_IN_ARRAY( m_Cards ); i++ )
				{
					card_c*		pCard = m_Cards[i];
					if ( pCard != NULL )
					{
					//Is it a master card?
						if ( pCard->m_pCard_master == NULL )
						{
						//Found a master card -> change the sample rate.
						//
						//Notice:
						//We don't have to send notification as GUI will refresh all cards info after the sync exits.
							Status = pCard->SetSampleRate( 44100 );

						//Sync slave cards.
							if ( SUCCESS( Status ) )
							{
								card_c*		pCard_slave = pCard->m_pCard_slave;
								while ( pCard_slave != NULL )
								{
									Status = pCard_slave->SetClockSource( card_c::CLOCK_SOURCE_EXTERN );
									if ( FAILED( Status ) )
									{
									//Failed to set extern clock...
										break;
									}

								//Next...
									pCard_slave = pCard_slave->m_pCard_slave;
								}
							}
						}
					}
				}
			}
		}

	//Is it desync?
		if (
		  ( *pSystemBuffer == 0 )		//Unsync command...
		  ||
		  ( FAILED( Status ) )			//Saync failed...
		)
		{
		//Desync...
		//
		//Notice:
		//We wont be sending any sampleRate notifications to GUI as when this function will return it will refresh all cards data anyway.
		#ifdef DBG_AND_LOGGER
			pLogger->Log( "  Desync\n" );
		#endif

		//Hardware unsync...
			for ( int i = 0; i < ELEMENTS_IN_ARRAY( m_Cards ); i++ )
			{
				card_c*		pCard = m_Cards[i];
				if ( pCard != NULL )
				{
				//Is it a master card?
					if ( pCard->m_pCard_master == NULL )
					{
					//Unsync slave cards.
					//
					//Notice:
					//Samplerate of all unsync cards will be set to 44.1kHz.
					//
						card_c*		pCard_slave = pCard->m_pCard_slave;
						while ( pCard_slave != NULL )
						{
							pCard_slave->SetClockSource( card_c::CLOCK_SOURCE_CARD );

						//Next...
							pCard_slave = pCard_slave->m_pCard_slave;
						}
					}
				}
			}

		//Software uncync...
			for ( int i = 0; i < ELEMENTS_IN_ARRAY( m_Cards ); i++ )
			{
				card_c*		pCard = m_Cards[i];
				if ( pCard != NULL )
				{
					pCard->m_pCard_master = NULL;
					pCard->m_pCard_slave = NULL;
				}
			}
		}

		//*pInformationSize = 0;
	}

	if ( SUCCESS( Status_locked ) )
	{
		UnlockDriver();
	}

#ifdef DBG_AND_LOGGER
	for ( int i = 0; i < ELEMENTS_IN_ARRAY(m_Cards); i++ )
	{
		card_c*		pCard = m_Cards[i];
		if ( pCard != NULL )
		{
			pLogger->Log( "  Card(%d; 0x%p): pMasterCard = 0x%p, pSlaveCard = 0x%p\n", i, pCard, pCard->m_pCard_master, pCard->m_pCard_slave );
		}
		else
		{
			pLogger->Log( "  Card(%d; 0x%p): not present\n", i, NULL );
		}
	}

	Procedure_Exit( Status );
#endif
	return Status;
}

//*****************************************************************************
// ProcedureName:
//		SetCardsOrder
//
// Explanation:
//		Procedure will define how the cards are synced together.
//
//		NOTICE:
//		Function will only determine which card is the master and which are it's
//		slaves. This is as the order of slaves is not important.
//		This also means that function can handle master with parallel slaves.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
// Return:
//
#pragma CODE_LOCKED
void driver_c::Data_SetCardsOrder
(
	void
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

//Get number of cards in the system.
	unsigned long		nCardInSystem = 0;

	for ( long i = 0; i < ELEMENTS_IN_ARRAY( pDriver->m_Cards ); i++ )
	{
		if ( pDriver->m_Cards[i] != NULL )
		{
			++nCardInSystem;
		}
	}

//Was this already done?
	if ( m_Data_SyncChecked != nCardInSystem )
	{
	#ifdef DBG_AND_LOGGER
		pLogger->Log( "  Check Cards Sync Order\n" );
	#endif

	//Separate master and slave cards.
		bool				masterCardPresent = false;
		unsigned long		nSlaveCards = 0;
		card_c*				pCards_master[MAX_CARDS][MAX_CARDS];
		card_c*				pCards_slave[MAX_CARDS];

		RtlZeroMemory( pCards_master, sizeof( pCards_master ) );
		RtlZeroMemory( pCards_slave, sizeof( pCards_slave ) );

		for ( int i = 0; i < MAX_CARDS; i++ )
		{
			card_c*		pCard = m_Cards[i];
			if ( pCard != NULL )
			{
			//Is this a master or a slave card... reset sync detector and read the status.
				//Reset sync...
				WORD	data = ReadWord( pCard->m_IoWave, SC_ENGINE );
				WriteWord( pCard->m_IoWave, SC_ENGINE, data | SC_RESET_CLOCK_DETECTOR );
				DelayThreadShort( DELAY_THREAD_SHORT_1us );

				WriteWord( pCard->m_IoWave, SC_ENGINE, data );
				DelayThreadShort( DELAY_THREAD_SHORT_10us );

				//Read status...
				data = ReadWord( pCard->m_IoWave, SC_ENGINE );
				if ( ( data & SC_SYNC_IN_CLOCK_PRESENT ) == 0 )
				{
				//There is no sync_in clock -> it's a master card...
					pCards_master[i][0] = pCard;

					masterCardPresent = true;
				}
				else
				{
				//Sync_in clock present -> it's a slave card...
					pCards_slave[i] = pCard;
					++nSlaveCards;
				}
			}
		}

	//Is there a master card?
		if ( masterCardPresent == false )
		{
		//There is no master card... select last slave card as master(it doesn't really matter which one...).
			if ( nSlaveCards > 0 )
			{
			//Get the last slave card.
				pCards_master[0][0] = pCards_slave[nSlaveCards-1];

			//Remove it from the array.
				pCards_slave[nSlaveCards-1] = NULL;
			}
		}

	#ifdef DBG_AND_LOGGER
		pLogger->Log( "  Master cards: \n" );
		for ( int i = 0; i < MAX_CARDS; i++ )
		{
			card_c*		pCard = pCards_master[i][0];
			if ( pCard != NULL )
			{
				pLogger->Log( "    Card %d is master.\n", pCard->m_SerialNumber );
			}
		}
		pLogger->Log( "  Slave cards: \n" );
		for ( int i = 0; i < MAX_CARDS; i++ )
		{
			card_c*		pCard = pCards_slave[i];
			if ( pCard != NULL )
			{
				pLogger->Log( "    Card %d is slave.\n", pCard->m_SerialNumber );
			}
		}
	#endif

	//Which slave cards belong to which master card...
		for ( int i = 0; i < MAX_CARDS; i++ )
		{
			card_c*		pCard_master = pCards_master[i][0];
			if ( pCard_master != NULL )
			{
			//Stop master clock and check which slave cards lost their sync.
				//Stop master card.
				WORD	data_master = ReadWord( pCard_master->m_IoWave, SC_ENGINE );
				WriteWord( pCard_master->m_IoWave, SC_ENGINE, data_master & ~( SC_START_AUDIO_ENGINE ) );
				DelayThreadShort( DELAY_THREAD_SHORT_1us );

				//Reset slave card sync...
				for ( int j = 0; j < MAX_CARDS; j++ )
				{
					card_c*		pCard_slave = pCards_slave[j];
					if ( pCard_slave != NULL )
					{
					//Reset sync...
						WORD	data_slave = ReadWord( pCard_slave->m_IoWave, SC_ENGINE );
						WriteWord( pCard_slave->m_IoWave, SC_ENGINE, data_slave | SC_RESET_CLOCK_DETECTOR );
						DelayThreadShort( DELAY_THREAD_SHORT_1us );

						WriteWord( pCard_slave->m_IoWave, SC_ENGINE, data_slave );
						DelayThreadShort( DELAY_THREAD_SHORT_10us );

					//Read status...
						data_slave = ReadWord( pCard_slave->m_IoWave, SC_ENGINE );
						if ( ( data_slave & SC_SYNC_IN_CLOCK_PRESENT ) == 0 )
						{
						//No sync_in clock -> we got a slave card.
							//Get first empty space...
							for ( int k = 1; k < MAX_CARDS; k++ )
							{
								if ( pCards_master[i][k] == NULL )
								{
									pCards_master[i][k] = pCard_slave;
									break;
								}
							}

						//Remove from slave cards.
							pCards_slave[j] = NULL;
						}
					}
				}

				//Start master card.
				WriteWord( pCard_master->m_IoWave, SC_ENGINE, data_master );
				DelayThreadShort( DELAY_THREAD_SHORT_1us );
			}
		}

	//Set cards order.
		for ( int i = 0; i < MAX_CARDS; i++ )
		{
		//Is there a master card?
			if ( pCards_master[i][0] != NULL )
			{
			//Yes -> set cards order.
				for ( int j = 0; j < MAX_CARDS; j++ )
				{
					card_c*		pCard = pCards_master[i][j];
					if ( pCard != NULL )
					{
					//Set current card master and slave card.
						//Master card...
						if ( j > 0 )
						{
							card_c*		pCard_master = pCards_master[i][j-1];

							pCard->m_SerialNumber_masterCard = pCard_master->m_SerialNumber + 1;	//one based
						}

						//Slave card...
						if ( ( j+1 ) < MAX_CARDS )
						{
							card_c*		pCard_slave = pCards_master[i][j+1];
							if ( pCard_slave != NULL )
							{
								pCard->m_SerialNumber_slaveCard = pCard_slave->m_SerialNumber + 1;	//one based
							}
							else
							{
							//There is no next card available.. we can stop the j loop.
								break;
							}
						}
					}
				}
			}
		}

	//Set the flag that sync check has already been done.
		m_Data_SyncChecked = nCardInSystem;
	}

#ifdef DBG_AND_LOGGER
	for ( int i = 0; i < MAX_CARDS; i++ )
	{
		card_c*		pCard = m_Cards[i];
		if ( pCard != NULL )
		{
			pLogger->Log( "  Card: %d -> Master Card: %d, Slave Card: %d\n", pCard->m_SerialNumber, pCard->m_SerialNumber_masterCard, pCard->m_SerialNumber_slaveCard );
		}
	}
	Procedure_Exit( STATUS_SUCCESS );
#endif
	return;
}
