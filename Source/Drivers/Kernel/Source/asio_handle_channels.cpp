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
#include "driver_ioctl_data.h"
#include "notify_event.h"

#pragma DATA_LOCKED


#pragma CODE_LOCKED
void asioHandle_c::Channels_CloseAll()
{
	card_c*		pCard = NULL;

//Stop and release ASIO channels.
	KIRQL		spinLock_oldLevel;

	KeAcquireSpinLock( &pDriver->m_GUI_notifyEvent_spinLock, &spinLock_oldLevel );

	//INS...
		channel_stereo_c*		pChannel;
		channel_stereo_c**		ppChannels = &m_pChannels_Stereo_INs[0];

		while ( ( pChannel = *ppChannels ) != NULL )
		{
		//Save card for later.
			pCard = pChannel->m_pCard;

		//Stop channel.
			if ( pChannel->m_State == KSSTATE_RUN )
			{
			//Stop channel.
				pChannel->m_State = KSSTATE_STOP;

			//Lower the number of channels in run state.
				--pChannel->m_pCard->m_Channels_nInUse_INs_ASIO;
			}

		//Release channel.
			pChannel->m_Type_client = channel_base_c::CHANNEL_TYPE_CLIENT_NON;
			pChannel->m_pCopyBuffersProcedure = NULL;

			pChannel->m_hAsio = 0;

			--pCard->m_Channels_opened_INs_ASIO;

		//Disable monitoring checkboxes in GUI (for both channels left and right).
			if ( pDriver->m_GUI_notifyEvent != NULL )
			{
				pDriver->m_GUI_notifyEvent_master->SetEvent_ChannelProperty_ControlEnabled( pCard->m_SerialNumber, 2*pChannel->m_Index, PROPERTY_CONTROL_ENABLED );
				pDriver->m_GUI_notifyEvent_master->SetEvent_ChannelProperty_ControlEnabled( pCard->m_SerialNumber, 2*pChannel->m_Index+1, PROPERTY_CONTROL_ENABLED );
			}

		//Remove reference.
			*ppChannels = NULL;

		//Next...
			++ppChannels;
		}

	//OUTs...
		ppChannels = &m_pChannels_Stereo_OUTs[0];

		while ( ( pChannel = *ppChannels ) != NULL )
		{
		//Save card for later.
			pCard = pChannel->m_pCard;

		//Stop channel.
			if ( pChannel->m_State == KSSTATE_RUN )
			{
			//Stop channel.
				pChannel->m_State = KSSTATE_STOP;

			//Mute output channel.
				//Clear samples in DAC card buffer.
				pChannel->ZeroChannelMemory_Full();

			//Lower the number of channels in run state.
				--pChannel->m_pCard->m_Channels_nInUse_OUTs_ASIO;
			}

		//Release channel.
			pChannel->m_Type_client = channel_base_c::CHANNEL_TYPE_CLIENT_NON;
			pChannel->m_pCopyBuffersProcedure = NULL;

			pChannel->m_hAsio = 0;

			--pCard->m_Channels_opened_OUTs_ASIO;

		//Remove reference.
			*ppChannels = NULL;

		//Next...
			++ppChannels;
		}

	//Get master card.
		card_c*			pCard_master = NULL;

		if ( pCard != NULL )
		{
			pCard_master = pCard->GetMasterCard();
		}

	//Send number of channels opened notification.
		if ( pDriver->m_GUI_notifyEvent != NULL )
		{
			card_c*		pCard_temp = pCard_master;
			while ( pCard_temp != NULL )
			{
				pDriver->m_GUI_notifyEvent_master->SetEvent_ChannelsOpened(
				  pCard_temp->m_SerialNumber,
				  pCard_temp->m_Channels_opened_INs_ASIO,
				  pCard_temp->m_Channels_opened_OUTs_ASIO,
				  pCard_temp->m_Channels_opened_INs_AVStream,
				  pCard_temp->m_Channels_opened_OUTs_AVStream
				);

			//Next...
				pCard_temp = pCard_temp->m_pCard_slave;
			}
		}

	//Fire notification.
		if ( pDriver->m_GUI_notifyEvent != NULL )
		{
			KeSetEvent( pDriver->m_GUI_notifyEvent, EVENT_INCREMENT, FALSE );
		}

		KeReleaseSpinLock( &pDriver->m_GUI_notifyEvent_spinLock, spinLock_oldLevel );

//Stop card interrupts if no more channels are playing.
	if ( pCard_master != NULL )
	{
		pCard_master->Streaming_Wave_Stop();
	}

//Unlock sampleRate.
	if ( pCard_master != NULL )
	{
		--pCard_master->m_SampleRate_owned;
	}
}

#pragma CODE_LOCKED
NTSTATUS asioHandle_c::Channels_OpenAll
(
	ioctl_asio_channels_header_s*	pHeader
)
{
	NTSTATUS	Status = STATUS_SUCCESS;

	card_c*		pCard = NULL;

//Validate...
	//Are any channels already opened by current ASIO handle?
	for ( int i = 0; i < ELEMENTS_IN_ARRAY( m_pChannels_Stereo_INs ); i++ )
	{
		if ( m_pChannels_Stereo_INs[i] != NULL )
		{
			Status = STATUS_UNSUCCESSFUL;
			break;
		}
	}
	if ( SUCCESS( Status ) )
	{
		for ( int i = 0; i < ELEMENTS_IN_ARRAY( m_pChannels_Stereo_OUTs ); i++ )
		{
			if ( m_pChannels_Stereo_OUTs[i] != NULL )
			{
				Status = STATUS_UNSUCCESSFUL;
				break;
			}
		}
	}

//Are requested channels available?
	if ( SUCCESS( Status ) )
	{
		for ( int i = 0; i < ELEMENTS_IN_ARRAY( pHeader->m_Channels_Opened ); i++ )
		{
		//Is card defined?
			if ( pHeader->m_Channels_Opened[i].m_Type == ioctl_cardInfo_s::CARD_TYPE_NOT_SET )
			{
			//No -> continue with next one...
				continue;
			}

		//Get the card.
			card_c*		pCard_temp = pDriver->m_Cards[pHeader->m_Channels_Opened[i].m_SerialNumber];
			if ( pCard_temp == NULL )
			{
				continue;
			}

		//Number of channels...
			int		nChannels_stereo;

			if ( pCard_temp->m_Type != card_c::CARD_TYPE_824 )
			{
			//4xx
				nChannels_stereo = 2;
			}
			else
			{
			//824
				nChannels_stereo = 4;
			}

		//Are input channels available?
			//Stereo...
			unsigned char	bit = 0x01;

			for ( int j = 0; j < nChannels_stereo; j++ )
			{
				if ( ( pHeader->m_Channels_Opened[i].m_Channels_IN & bit ) != 0 )
				{
				//Get requested channel...
					channel_stereo_c*	pChannel = pCard_temp->m_pChannels_Stereo_INs[j];
					if ( pChannel == NULL )
					{
						Status = STATUS_UNSUCCESSFUL;
						break;
					}

				//Is it already opened?
					if ( pChannel->m_Type_client != channel_base_c::CHANNEL_TYPE_CLIENT_NON )
					{
						Status = STATUS_UNSUCCESSFUL;
						break;
					}

				//Save card for later.
					pCard = pChannel->m_pCard;
				}

			//Next...
				bit <<= 1;
			}
			if ( FAILED( Status ) )
			{
				break;
			}

			//All...
			if ( pCard_temp->m_pChannels_All_INs != NULL )
			{
				if ( pCard_temp->m_pChannels_All_INs->m_Type_client != channel_base_c::CHANNEL_TYPE_CLIENT_NON )
				{
					Status = STATUS_UNSUCCESSFUL;
				}
			}
			else
			{
				Status = STATUS_UNSUCCESSFUL;
			}
			if ( FAILED( Status ) )
			{
				break;
			}

		//Are output channels available?
			//Stereo...
			bit = 0x01;

			for ( int j = 0; j < nChannels_stereo; j++ )
			{
				if ( ( pHeader->m_Channels_Opened[i].m_Channels_OUT & bit ) != 0 )
				{
				//Get requested channel...
					channel_stereo_c*	pChannel = pCard_temp->m_pChannels_Stereo_OUTs[j];
					if ( pChannel == NULL )
					{
						Status = STATUS_UNSUCCESSFUL;
						break;
					}

				//Is it already opened?
					if ( pChannel->m_Type_client != channel_base_c::CHANNEL_TYPE_CLIENT_NON )
					{
						Status = STATUS_UNSUCCESSFUL;
						break;
					}

				//Save card for later.
					pCard = pChannel->m_pCard;
				}

			//Next...
				bit <<= 1;
			}
			if ( FAILED( Status ) )
			{
				break;
			}

			//All...
			if ( pCard_temp->m_pChannels_All_OUTs != NULL )
			{
				if ( pCard_temp->m_pChannels_All_OUTs->m_Type_client != channel_base_c::CHANNEL_TYPE_CLIENT_NON )
				{
					Status = STATUS_UNSUCCESSFUL;
				}
			}
			else
			{
				Status = STATUS_UNSUCCESSFUL;
			}
			if ( FAILED( Status ) )
			{
				break;
			}
		}
	}

//Get master card.
	card_c*			pCard_master = NULL;

	if ( SUCCESS( Status ) )
	{
		if ( pCard != NULL )
		{
			pCard_master = pCard->GetMasterCard();
		}
		else
		{
			Status = STATUS_UNSUCCESSFUL;
		}
	}

//Open channels and send GUI notification events.
	if ( SUCCESS( Status ) )
	{
		KIRQL		spinLock_oldLevel;

		KeAcquireSpinLock( &pDriver->m_GUI_notifyEvent_spinLock, &spinLock_oldLevel );

			unsigned long		nChannels_Stereo_INs = 0;
			unsigned long		nChannels_Stereo_OUTs = 0;

			for ( int i = 0; i < ELEMENTS_IN_ARRAY( pHeader->m_Channels_Opened ); i++ )
			{
			//Is card defined?
				if ( pHeader->m_Channels_Opened[i].m_Type == ioctl_cardInfo_s::CARD_TYPE_NOT_SET )
				{
				//No -> continue with next one...
					continue;
				}

			//Get the card.
				card_c*		pCard = pDriver->m_Cards[pHeader->m_Channels_Opened[i].m_SerialNumber];
				if ( pCard == NULL )
				{
					continue;
				}

			//Number of channels...
				int		nChannels_stereo;

				if ( pCard->m_Type != card_c::CARD_TYPE_824 )
				{
				//4xx
					nChannels_stereo = 2;
				}
				else
				{
				//824
					nChannels_stereo = 4;
				}

			//Open input channels.
				unsigned char	bit = 0x01;

				for ( int j = 0; j < nChannels_stereo; j++ )
				{
					if ( ( pHeader->m_Channels_Opened[i].m_Channels_IN & bit ) != 0 )
					{
					//Open requested channel...
						channel_stereo_c*	pChannel = pCard->m_pChannels_Stereo_INs[j];

						pChannel->m_hAsio = m_HandleID;

						pChannel->m_Type_client = channel_base_c::CHANNEL_TYPE_CLIENT_ASIO;
						pChannel->Set_CopyBuffersProcedure_ASIO( pHeader->m_SampleType );

						++pCard->m_Channels_opened_INs_ASIO;

					//Save pointer to ASIO handle.
						m_pChannels_Stereo_INs[nChannels_Stereo_INs] = pChannel;
						++nChannels_Stereo_INs;

					//Disable monitoring checkboxes in GUI (for both channels left and right).
						if ( pDriver->m_GUI_notifyEvent != NULL )
						{
							pDriver->m_GUI_notifyEvent_master->SetEvent_ChannelProperty_ControlEnabled( pCard->m_SerialNumber, 2*j, PROPERTY_CONTROL_DISABLED );
							pDriver->m_GUI_notifyEvent_master->SetEvent_ChannelProperty_ControlEnabled( pCard->m_SerialNumber, 2*j+1, PROPERTY_CONTROL_DISABLED );
						}
					}

				//Next...
					bit <<= 1;
				}

			//Open output channels.
				bit = 0x01;

				for ( int j = 0; j < nChannels_stereo; j++ )
				{
					if ( ( pHeader->m_Channels_Opened[i].m_Channels_OUT & bit ) != 0 )
					{
					//Open requested channel...
						channel_stereo_c*	pChannel = pCard->m_pChannels_Stereo_OUTs[j];

						pChannel->m_hAsio = m_HandleID;

						pChannel->m_Type_client = channel_base_c::CHANNEL_TYPE_CLIENT_ASIO;
						pChannel->Set_CopyBuffersProcedure_ASIO( pHeader->m_SampleType );

						++pCard->m_Channels_opened_OUTs_ASIO;

					//Save pointer to ASIO handle.
						m_pChannels_Stereo_OUTs[nChannels_Stereo_OUTs] = pChannel;
						++nChannels_Stereo_OUTs;
					}

				//Next...
					bit <<= 1;
				}
			}

		//Send number of channels opened notification.
			if ( pDriver->m_GUI_notifyEvent != NULL )
			{
				card_c*		pCard_temp = pCard_master;
				while ( pCard_temp != NULL )
				{
					pDriver->m_GUI_notifyEvent_master->SetEvent_ChannelsOpened(
					  pCard_temp->m_SerialNumber,
					  pCard_temp->m_Channels_opened_INs_ASIO,
					  pCard_temp->m_Channels_opened_OUTs_ASIO,
					  pCard_temp->m_Channels_opened_INs_AVStream,
					  pCard_temp->m_Channels_opened_OUTs_AVStream
					);

				//Next...
					pCard_temp = pCard_temp->m_pCard_slave;
				}
			}

		//Lock samplerate.
			if ( pCard_master != NULL )
			{
			//Lock sampleRate.
				++pCard_master->m_SampleRate_owned;
			}

		//Fire notification.
			if ( pDriver->m_GUI_notifyEvent != NULL )
			{
				KeSetEvent( pDriver->m_GUI_notifyEvent, EVENT_INCREMENT, FALSE );
			}

		KeReleaseSpinLock( &pDriver->m_GUI_notifyEvent_spinLock, spinLock_oldLevel );
	}

	return Status;
}

#pragma CODE_LOCKED
void asioHandle_c::Channels_SetBuffers
(
	unsigned long*		pBuffers
)
{
//INS...
	channel_stereo_c*		pChannel;
	channel_stereo_c**		ppChannels = &m_pChannels_Stereo_INs[0];

	while ( ( pChannel = *ppChannels ) != NULL )
	{
	//Get addresses.
		PULONG		pBuffer_left_A = pBuffers; pBuffers += pDriver->m_BufferSize;
		PULONG		pBuffer_left_B = pBuffers; pBuffers += pDriver->m_BufferSize;
		PULONG		pBuffer_right_A = pBuffers; pBuffers += pDriver->m_BufferSize;
		PULONG		pBuffer_right_B = pBuffers; pBuffers += pDriver->m_BufferSize;

	//Set buffer addresses.
		pChannel->Set_Buffers_ASIO( pBuffer_left_A, pBuffer_left_B, pBuffer_right_A, pBuffer_right_B );

	//Next...
		++ppChannels;
	}

//OUTs...
	ppChannels = &m_pChannels_Stereo_OUTs[0];

	while ( ( pChannel = *ppChannels ) != NULL )
	{
	//Get addresses.
		PULONG		pBuffer_left_A = pBuffers; pBuffers += pDriver->m_BufferSize;
		PULONG		pBuffer_left_B = pBuffers; pBuffers += pDriver->m_BufferSize;
		PULONG		pBuffer_right_A = pBuffers; pBuffers += pDriver->m_BufferSize;
		PULONG		pBuffer_right_B = pBuffers; pBuffers += pDriver->m_BufferSize;

	//Set buffer addresses.
		pChannel->Set_Buffers_ASIO( pBuffer_left_A, pBuffer_left_B, pBuffer_right_A, pBuffer_right_B );

	//Next...
		++ppChannels;
	}
}

#pragma CODE_LOCKED
NTSTATUS asioHandle_c::Channels_Start
(
	ioctl_asio_start_header_s*		pHeader
)
{
	NTSTATUS	Status = STATUS_SUCCESS;

	card_c*		pCard = NULL;

//Are requested channels opened by current ASIO handle?
	for ( int i = 0; i < ELEMENTS_IN_ARRAY( pHeader->m_Channels_Started ); i++ )
	{
	//Is card set?
		if ( pHeader->m_Channels_Started[i].m_Type == ioctl_cardInfo_s::CARD_TYPE_NOT_SET )
		{
			continue;
		}

	//Get the card.
		card_c*		pCard_temp = pDriver->m_Cards[pHeader->m_Channels_Started[i].m_SerialNumber];
		if ( pCard_temp == NULL )
		{
			continue;
		}

	//Number of channels...
		int		nChannels_mono;

		if ( pCard_temp->m_Type != card_c::CARD_TYPE_824 )
		{
		//4xx
			nChannels_mono = 4;
		}
		else
		{
		//824
			nChannels_mono = 8;
		}

	//Input channels...
		unsigned char	bit = 0x01;		//every bit is 1 mono channel

		for ( int j = 0; j < nChannels_mono; j++ )
		{
			if ( ( pHeader->m_Channels_Started[i].m_Channels_IN & bit ) != 0 )
			{
			//Get requested channel...
				channel_stereo_c*	pChannel = pCard_temp->m_pChannels_Stereo_INs[j/2];
				if ( pChannel == NULL )
				{
					Status = STATUS_UNSUCCESSFUL;
					break;
				}

			//Is opened by current ASIO handle?
				if ( pChannel->m_hAsio != m_HandleID )
				{
				//No -> error.
					Status = STATUS_UNSUCCESSFUL;
					break;
				}

			//Set card for later.
				pCard = pChannel->m_pCard;
			}

		//Next...
			bit <<= 1;
		}
		if ( FAILED( Status ) )
		{
			break;
		}

	//Output channels...
		bit = 0x01;			//every bit is 1 mono channel

		for ( int j = 0; j < nChannels_mono; j++ )
		{
			if ( ( pHeader->m_Channels_Started[i].m_Channels_OUT & bit ) != 0 )
			{
			//Get requested channel...
				channel_stereo_c*	pChannel = pCard_temp->m_pChannels_Stereo_OUTs[j/2];
				if ( pChannel == NULL )
				{
					Status = STATUS_UNSUCCESSFUL;
					break;
				}

			//Is opened by current ASIO handle?
				if ( pChannel->m_hAsio != m_HandleID )
				{
				//No -> error.
					Status = STATUS_UNSUCCESSFUL;
					break;
				}

			//Set card for later.
				pCard = pChannel->m_pCard;
			}

		//Next...
			bit <<= 1;
		}
		if ( FAILED( Status ) )
		{
			break;
		}
	}

//Reset switch structure variables.
	if ( SUCCESS( Status ) )
	{
	//Get switch structure.
		ioctl_asio_switch*		pSwitch = (ioctl_asio_switch*)m_pMemory_locked;

	//Reset variables.
		//Buffer select...
		pSwitch->m_BufferSelect = ASIO_SWITCH_USE_BUFFER_B;		//in DPC this will be converted to A
		
		//Synhronization...
		pSwitch->m_Synhronize = FALSE;

		//Clock and timestamp.
		pSwitch->m_Clock_samples.clock = 0;
		pSwitch->m_Clock_samples_first.clock = 0;
	}

//Get master card.
	card_c*			pCard_master = NULL;

	if ( SUCCESS( Status ) )
	{
		if ( pCard != NULL )
		{
			pCard_master = pCard->GetMasterCard();
		}
		else
		{
			Status = STATUS_UNSUCCESSFUL;
		}
	}

//Start channels.
	if ( SUCCESS( Status ) )
	{
		for ( int i = 0; i < ELEMENTS_IN_ARRAY( pHeader->m_Channels_Started ); i++ )
		{
		//Is card set?
			if ( pHeader->m_Channels_Started[i].m_Type == ioctl_cardInfo_s::CARD_TYPE_NOT_SET )
			{
				continue;
			}

		//Get the card.
			card_c*		pCard = pDriver->m_Cards[pHeader->m_Channels_Started[i].m_SerialNumber];
			if ( pCard == NULL )
			{
				continue;
			}

		//Number of channels...
			int		nChannels_mono;

			if ( pCard->m_Type != card_c::CARD_TYPE_824 )
			{
			//4xx
				nChannels_mono = 4;
			}
			else
			{
			//824
				nChannels_mono = 8;
			}

		//Start input channels.
			unsigned char	bit = 0x01;

			for ( int j = 0; j < nChannels_mono; j++ )
			{
				if ( ( pHeader->m_Channels_Started[i].m_Channels_IN & bit ) != 0 )
				{
				//Start requested channel...
					channel_stereo_c*	pChannel = pCard->m_pChannels_Stereo_INs[j/2];

					if ( pChannel->m_State == KSSTATE_STOP )
					{
					//Stop channel.
						pChannel->m_State = KSSTATE_RUN;

					//Rise the number of channels in run state.
						++pCard->m_Channels_nInUse_INs_ASIO;
					}
				}

			//Next...
				bit <<= 1;
			}

		//Are output channels available?
			bit = 0x01;

			for ( int j = 0; j < nChannels_mono; j++ )
			{
				if ( ( pHeader->m_Channels_Started[i].m_Channels_OUT & bit ) != 0 )
				{
				//Start requested channel...
					channel_stereo_c*	pChannel = pCard->m_pChannels_Stereo_OUTs[j/2];
					
					if ( pChannel->m_State == KSSTATE_STOP )
					{
					//Stop channel.
						pChannel->m_State = KSSTATE_RUN;

					//Rise the number of channels in run state.
						++pCard->m_Channels_nInUse_OUTs_ASIO;
					}
				}

			//Next...
				bit <<= 1;
			}
		}

	//Start master cards -> enable wave interrupts.
		if ( pCard_master != NULL )
		{
			pCard_master->Streaming_Wave_Start();
		}
	}

	return Status;
}

#pragma CODE_LOCKED
void asioHandle_c::Channels_Stop()
{
	card_c*					pCard = NULL;

//Stop ASIO channels.
	//INS...
	channel_stereo_c*		pChannel;
	channel_stereo_c**		ppChannels = &m_pChannels_Stereo_INs[0];

	while ( ( pChannel = *ppChannels ) != NULL )
	{
	//Stop channel.
		if ( pChannel->m_State == KSSTATE_RUN )
		{
		//Stop channel.
			pChannel->m_State = KSSTATE_STOP;

		//Lower the number of channels in run state.
			--pChannel->m_pCard->m_Channels_nInUse_INs_ASIO;
		}

	//Save card for later.
		pCard = pChannel->m_pCard;

	//Next...
		++ppChannels;
	}

	//OUTs...
	ppChannels = &m_pChannels_Stereo_OUTs[0];

	while ( ( pChannel = *ppChannels ) != NULL )
	{
	//Stop channel.
		if ( pChannel->m_State == KSSTATE_RUN )
		{
		//Stop channel.
			pChannel->m_State = KSSTATE_STOP;

		//Mute output channel.
			//Clear samples in DAC card buffer.
			pChannel->ZeroChannelMemory_Full();

		//Lower the number of channels in run state.
			--pChannel->m_pCard->m_Channels_nInUse_OUTs_ASIO;
		}

	//Save card for later.
		pCard = pChannel->m_pCard;

	//Next...
		++ppChannels;
	}

//Stop card interrupts if no more channels are playing.
	if ( pCard != NULL )
	{
		card_c*		pCard_master = pCard->GetMasterCard();
		if ( pCard_master != NULL )
		{
			pCard_master->Streaming_Wave_Stop();
		}
	}
}
