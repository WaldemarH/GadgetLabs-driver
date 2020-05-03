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
#include "driver_avstream_filter_register.h"


#pragma DATA_LOCKED

//***************************************************************************
// ProcedureName:
// 		SetChannels
//
// Explanation:
//		This procedure will create channels.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return:
//		STATUS_SUCCESS/STATUS_INSUFFICIENT_RESOURCES
//
#pragma CODE_PAGED
NTSTATUS card_c::SetChannels
(
	void
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	NTSTATUS		Status = STATUS_SUCCESS;

	unsigned long	channelSize;
	unsigned long	nStereoPairs = 0;
	PULONG			pAddressOnCard_INs[8];
	PULONG			pAddressOnCard_OUTs[8];

//Get number of stereo pairs.
	if ( m_Type != CARD_TYPE_824 )
	{
	//4xx cards have 2 stereo pairs
		nStereoPairs += 2;

		//Clear the not used channels in pAddressOnCard array.
		pAddressOnCard_INs[4] = NULL;
		pAddressOnCard_INs[5] = NULL;
		pAddressOnCard_INs[6] = NULL;
		pAddressOnCard_INs[7] = NULL;

		pAddressOnCard_OUTs[4] = NULL;
		pAddressOnCard_OUTs[5] = NULL;
		pAddressOnCard_OUTs[6] = NULL;
		pAddressOnCard_OUTs[7] = NULL;

		channelSize = SC4_MONO_CHANNEL_SIZE;
	}
	else
	{
	//824 card has 4 stereo pairs
		nStereoPairs += 4;

		channelSize = SC8_MONO_CHANNEL_SIZE;
	}

//Set stereo channels.
	for ( unsigned long i = 0; i < nStereoPairs; i++ )
	{
	//Get address of current channel on card.
		PULONG		pChannelAddress_IN_left;
		PULONG		pChannelAddress_IN_right;
		PULONG		pChannelAddress_OUT_left;
		PULONG		pChannelAddress_OUT_right;

		if ( m_Type != CARD_TYPE_824 )
		{
		//4xx
			pChannelAddress_IN_left = m_Mem_Analog + SC4_MONO_CHANNEL_SIZE * 2*i;
			pChannelAddress_IN_right = pChannelAddress_IN_left + SC4_MONO_CHANNEL_SIZE;

			pChannelAddress_OUT_left = m_Mem_Analog + SC4_OUTPUT_CH1 + SC4_MONO_CHANNEL_SIZE * 2*i;
			pChannelAddress_OUT_right = pChannelAddress_OUT_left + SC4_MONO_CHANNEL_SIZE;
		}
		else
		{
		//824
			pChannelAddress_IN_left = m_Mem_Analog + SC8_MONO_CHANNEL_SIZE * 2*i;
			pChannelAddress_IN_right = pChannelAddress_IN_left + SC8_MONO_CHANNEL_SIZE;

			pChannelAddress_OUT_left = m_Mem_Analog + SC8_OUTPUT_CH1 + SC8_MONO_CHANNEL_SIZE * 2*i;
			pChannelAddress_OUT_right = pChannelAddress_OUT_left + SC8_MONO_CHANNEL_SIZE;
		}

	//Save addresses for multichannel class.
		pAddressOnCard_INs[2*i] = pChannelAddress_IN_left;
		pAddressOnCard_INs[2*i+1] = pChannelAddress_IN_right;

		pAddressOnCard_OUTs[2*i] = pChannelAddress_OUT_left;
		pAddressOnCard_OUTs[2*i+1] = pChannelAddress_OUT_right;

	//Set IN stereo channels.
		channel_stereo_c*		pChannel = new channel_stereo_c( this, true, channelSize, i, pChannelAddress_IN_left, pChannelAddress_IN_right );
		if ( pChannel != NULL )
		{
			m_pChannels_Stereo_INs[i] = pChannel;
		}
		else
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

	//Set OUT stereo channels.
		pChannel = new channel_stereo_c( this, false, channelSize, i, pChannelAddress_OUT_left, pChannelAddress_OUT_right );
		if ( pChannel != NULL )
		{
			m_pChannels_Stereo_OUTs[i] = pChannel;
		}
		else
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
	}

//Set all channels.
	//Input...
	if ( SUCCESS( Status ) )
	{
		channel_all_c*		pChannel = new channel_all_c( this, true, channelSize, pAddressOnCard_INs );
		if ( pChannel != NULL )
		{
			m_pChannels_All_INs = pChannel;
		}
		else
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
		}

	}

	//Output...
	if ( SUCCESS( Status ) )
	{
		channel_all_c*		pChannel = new channel_all_c( this, false, channelSize, pAddressOnCard_OUTs );
		if ( pChannel != NULL )
		{
			m_pChannels_All_OUTs = pChannel;
		}
		else
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
		}
	}

//Set Midi.
	//Input...
	if ( SUCCESS( Status ) )
	{
		channel_midi_c*		pChannel = new channel_midi_c( this, true );
		if ( pChannel != NULL )
		{
			m_pChannels_Midi_INs = pChannel;
		}
		else
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
		}

	}

	//Output...
	if ( SUCCESS( Status ) )
	{
		channel_midi_c*		pChannel = new channel_midi_c( this, false );
		if ( pChannel != NULL )
		{
			m_pChannels_Midi_OUTs = pChannel;
		}
		else
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
		}
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

//***************************************************************************
// ProcedureName: GetChannel_FromPin_Wave
//
// Explanation:
//		Procedure will try to retrieve the channel class.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// Pin: IN
//		Pointer of a pin to retrieve the channel structure.
//
// ppChannelInfo: IN OUT
//		Pointer to variable that will receive channelInfo pointer.
//
// --------------------------------------------------------------------------
// Return: STATUS_SUCCESS or STATUS_UNSUCCESSFUL.
//
NTSTATUS card_c::GetChannel_FromPin_Wave
(
	IN PKSPIN					pPin,
	OUT channel_base_c**		ppChannel
)
{
//#ifdef DBG_PRINT
//	DbgPrint( "%s Start\n", __FUNCTION__ );
//#endif

	NTSTATUS				Status = STATUS_SUCCESS;

//Get FilterFactory.
	PKSFILTERFACTORY		pFilterFactory = KsFilterGetParentFilterFactory( KsPinGetParentFilter( pPin ) );
	if ( pFilterFactory == NULL )
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//Get channel from FilterFactory.
	if ( SUCCESS( Status ) )
	{
	//Does it belong to All channel?
		if ( m_pFilterFactories_All == pFilterFactory )
		{
		//Yes -> is it input or output?
			channel_all_c*		pChannel = NULL;

			switch ( pPin->Id )
			{
			case WAVEIN_12:
			{
			//Input...
				pChannel = m_pChannels_All_INs;
				break;
			}
			case WAVEOUT_12:
			{
			//Output...
				pChannel = m_pChannels_All_OUTs;
				break;
			}
			default:
			{
				Status = STATUS_UNSUCCESSFUL;
				break;
			}
			}

		//Return channel class.
			if ( SUCCESS( Status ) )
			{
				if ( ppChannel != NULL )
				{
					*ppChannel = pChannel;
				}
				else
				{
					Status = STATUS_UNSUCCESSFUL;
				}
			}
		}
		else
		{
		//No -> check stereo channels.
			Status = STATUS_UNSUCCESSFUL;

			for ( unsigned long i = 0; i < ELEMENTS_IN_ARRAY(m_pFilterFactories_Stereo); i++ )
			{
				if ( m_pFilterFactories_Stereo[i] == pFilterFactory )
				{
					Status = STATUS_SUCCESS;

				//Is it input or output?
				//
				//Pin IDs:
				//"IN" SINK pin (0), BRIDGE pin (1), "OUT" SINK pin (2), BRIDGE pin(3)
					channel_stereo_c**		ppChannels;

					switch ( pPin->Id )
					{
					case WAVEIN_12:
					{
					//Input...
						ppChannels = (channel_stereo_c**)m_pChannels_Stereo_INs;
						break;
					}
					case WAVEOUT_12:
					{
					//Output...
						ppChannels = (channel_stereo_c**)m_pChannels_Stereo_OUTs;
						break;
					}
					default:
					{
						Status = STATUS_UNSUCCESSFUL;
						break;
					}
					}

				//Get and set the pointer.
					if ( SUCCESS( Status ) )
					{
					//Get current channelInfo structure.
					//
					//i(filterFactories) = i(channelStereoPair_xxx)
						ppChannels += i;

					//Return the pointer.
						if ( ppChannel != NULL )
						{
							*ppChannel = *ppChannels;
						}
						else
						{
							Status = STATUS_UNSUCCESSFUL;
						}
					}
					break;
				}
			}
		}
	}

//#ifdef DBG_PRINT
//	DbgPrint( "%s Exit -> Return: %d\n", __FUNCTION__, Status );
//#endif
	return Status;
}

//***************************************************************************
// ProcedureName:
//		IsChannelInUse_Wave
//
//		Notice:
//		Must be protected with channel spinlock.
//
// Explanation:
//		This procedure will check if any channel in current card is in use
//
// IRQ Level:
//		ANY
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return:
//		TRUE/FALSE
//
#pragma CODE_LOCKED
BOOL card_c::IsChannelInUse_Wave()
{
	channel_stereo_c*		pChannel;
	channel_stereo_c**		ppChannel;

//Check stereo inputs...
	ppChannel = (channel_stereo_c**)m_pChannels_Stereo_INs;
	while ( ( pChannel = *ppChannel ) != NULL )
	{
		if ( pChannel->m_Type_client != channel_base_c::CHANNEL_TYPE_CLIENT_NON )
		{
		//Channel is in use...
			return TRUE;
		}

	//Next...
		++ppChannel;
	}

//Check stereo outputs...
	ppChannel = (channel_stereo_c**)m_pChannels_Stereo_OUTs;
	while ( ( pChannel = *ppChannel ) != NULL )
	{
		if ( pChannel->m_Type_client != channel_base_c::CHANNEL_TYPE_CLIENT_NON )
		{
		//Channel is in use...
			return TRUE;
		}

	//Next...
		++ppChannel;
	}

//Check all input...
	if ( m_pChannels_All_INs->m_Type_client != channel_base_c::CHANNEL_TYPE_CLIENT_NON )
	{
	//Channel is in use...
		return TRUE;
	}

//Check all output...
	if ( m_pChannels_All_OUTs->m_Type_client != channel_base_c::CHANNEL_TYPE_CLIENT_NON )
	{
	//Channel is in use...
		return TRUE;
	}

	return FALSE;
}

//***************************************************************************
// ProcedureName:
//		IsChannelInUse_Wave
//
//		Notice:
//		Must be protected with channel spinlock.
//
// Explanation:
//		This procedure will check if any channel related to current channel
//		is ni use.
//
// IRQ Level:
//		ANY
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return:
//		STATUS_SUCCESS/STATUS_INSUFFICIENT_RESOURCES
//
#pragma CODE_LOCKED
NTSTATUS card_c::IsChannelInUse_Wave
(
	channel_base_c*		pChannel
)
{
//#ifdef DBG_PRINT
//	DbgPrint( "%s Start\n", __FUNCTION__ );
//#endif

	NTSTATUS		Status = STATUS_SUCCESS;

//Is channel in use?
	if ( pChannel->m_Type_client != channel_base_c::CHANNEL_TYPE_CLIENT_NON )
	{
	//Channel is in use...
		Status = STATUS_UNSUCCESSFUL;
	}
	else
	{
	//Channel is not in use... check also All or Stereo channel is in use.
		if ( pChannel->m_Format == channel_base_c::CHANNEL_FORMAT_STEREO )
		{
		//Stereo channel has been requested.
			//Is All channel opened?
			if ( pChannel->m_Type == channel_base_c::CHANNEL_TYPE_IN )
			{
			//Input...
				if ( m_pChannels_All_INs->m_Type_client != channel_base_c::CHANNEL_TYPE_CLIENT_NON )
				{
				//All channel is in use...
					Status = STATUS_UNSUCCESSFUL;
				}
			}
			else
			{
			//Output...
				if ( m_pChannels_All_OUTs->m_Type_client != channel_base_c::CHANNEL_TYPE_CLIENT_NON )
				{
				//All channel is in use...
					Status = STATUS_UNSUCCESSFUL;
				}
			}
		}
		else
		{
		//Surround channel has been requested.
			channel_stereo_c*		pChannel_temp;
			channel_stereo_c**		ppChannel_temp;

			//Is input or output?
			if ( pChannel->m_Type == channel_base_c::CHANNEL_TYPE_IN )
			{
			//Input...
				ppChannel_temp = (channel_stereo_c**)m_pChannels_Stereo_INs;
			}
			else
			{
			//Output...
				ppChannel_temp = (channel_stereo_c**)m_pChannels_Stereo_OUTs;
			}

			//Is any of the stereo channels opened?
			while ( ( pChannel_temp = *ppChannel_temp ) != NULL )
			{
				if ( pChannel_temp->m_Type_client != channel_base_c::CHANNEL_TYPE_CLIENT_NON )
				{
				//Stereo channel is in use...
					Status = STATUS_UNSUCCESSFUL;
					break;
				}

			//Next...
				++ppChannel_temp;
			}
		}
	}

//#ifdef DBG_PRINT
//	DbgPrint( "%s Exit -> Return: %d\n", __FUNCTION__, Status );
//#endif
	return Status;
}

//***************************************************************************
// ProcedureName: GetChannel_FromPin_Midi
//
// Explanation:
//		Procedure will try to retrieve the channel class.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// Pin: IN
//		Pointer of a pin to retrieve the channel structure.
//
// ppChannelInfo: IN OUT
//		Pointer to variable that will receive channelInfo pointer.
//
// --------------------------------------------------------------------------
// Return: STATUS_SUCCESS or STATUS_UNSUCCESSFUL.
//
NTSTATUS card_c::GetChannel_FromPin_Midi
(
	IN PKSPIN					pPin,
	OUT channel_midi_c**		ppChannel
)
{
//#ifdef DBG_PRINT
//	DbgPrint( "%s Start\n", __FUNCTION__ );
//#endif

	NTSTATUS			Status = STATUS_SUCCESS;

//Get channel.
	//Is it input or output?
	channel_midi_c*		pChannel = NULL;

	switch ( pPin->Id )
	{
	//case MIDI_IN_DX:
	case MIDI_IN_MME:
	{
	//Input...
		pChannel = m_pChannels_Midi_INs;
		break;
	}
	//case MIDI_OUT_DX:
	case MIDI_OUT_MME:
	{
	//Output...
		pChannel = m_pChannels_Midi_OUTs;
		break;
	}
	default:
	{
		Status = STATUS_UNSUCCESSFUL;
		break;
	}
	}

//Return channel class.
	if ( SUCCESS( Status ) )
	{
		if ( ppChannel != NULL )
		{
			*ppChannel = pChannel;
		}
		else
		{
			Status = STATUS_UNSUCCESSFUL;
		}
	}

//#ifdef DBG_PRINT
//	DbgPrint( "%s Exit -> Return: %d\n", __FUNCTION__, Status );
//#endif
	return Status;
}

//***************************************************************************
// ProcedureName:
//		IsChannelInUse_Midi
//
//		Notice:
//		Must be protected with channel spinlock.
//
// Explanation:
//		This procedure will check if any channel in current card is in use.
//
// IRQ Level:
//		ANY
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return:
//		TRUE/FALSE
//
#pragma CODE_LOCKED
BOOL card_c::IsChannelInUse_Midi()
{
//Check if any Midi channel is opened.
	if (
	  ( m_pChannels_Midi_INs->m_Type_client !=  channel_midi_c::CHANNEL_TYPE_CLIENT_NON )
	  ||
	  ( m_pChannels_Midi_OUTs->m_Type_client !=  channel_midi_c::CHANNEL_TYPE_CLIENT_NON )
	)
	{
		return TRUE;
	}

	return FALSE;
}

//***************************************************************************
// ProcedureName:
//		IsChannelInUse_Midi
//
//		Notice:
//		Must be protected with channel spinlock.
//
// Explanation:
//		This procedure will check if any channel related to current channel
//		is ni use.
//
// IRQ Level:
//		ANY
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return:
//		STATUS_SUCCESS/STATUS_INSUFFICIENT_RESOURCES
//
#pragma CODE_LOCKED
NTSTATUS card_c::IsChannelInUse_Midi
(
	channel_midi_c*		pChannel
)
{
//#ifdef DBG_PRINT
//	DbgPrint( "%s Start\n", __FUNCTION__ );
//#endif

	NTSTATUS		Status = STATUS_SUCCESS;

//Is channel in use?
	if ( pChannel->m_Type_client != channel_midi_c::CHANNEL_TYPE_CLIENT_NON )
	{
	//Channel is in use...
		Status = STATUS_UNSUCCESSFUL;
	}

//#ifdef DBG_PRINT
//	DbgPrint( "%s Exit -> Return: %d\n", __FUNCTION__, Status );
//#endif
	return Status;
}
