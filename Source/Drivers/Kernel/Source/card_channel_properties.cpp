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
#include "card_codec.h"
#include "driver_ioctl_data.h"
#include "notify_event.h"


#pragma DATA_LOCKED


#pragma CODE_LOCKED
void card_c::GetChannelProperty
(
	IN ioctl_channelProperty_s*		pChannel
)
{
	switch ( pChannel->m_Property )
	{
	case PROPERTY_TYPE_GAIN_ADC:
	{
		switch ( m_Type )
		{
		case CARD_TYPE_424:
		{
			if ( pChannel->m_SerialNumber == 0 )
			{
				WORD	data = ReadWord( m_IoWave, SC4_GAIN_MON );
				if ( ( data & SC424_INPUT_GAIN ) == 0 )
				{
					pChannel->m_Position = PROPERTY_GAIN_10dBv;
				}
				else
				{
					pChannel->m_Position = PROPERTY_GAIN_4dBu;
				}
			}
			break;
		}
		case CARD_TYPE_496:
		{
			if ( pChannel->m_SerialNumber == 0 )
			{
				WORD	data = ReadWord( m_IoWave, SC4_GAIN_MON );
				if ( ( data & SC496_INPUT_GAIN ) == 0 )
				{
					pChannel->m_Position = PROPERTY_GAIN_10dBv;
				}
				else
				{
					pChannel->m_Position = PROPERTY_GAIN_4dBu;
				}
			}
			break;
		}
		case CARD_TYPE_824:
		{
		//824 card
		//
		//SC_GSR:
		//InGain (0=-10dBV, 1=+4dBu)
			WORD	channelBit;

			switch ( pChannel->m_SerialNumber )
			{
			case 0: channelBit = SC8_INPUT_GAIN_CH_1; break;
			case 1: channelBit = SC8_INPUT_GAIN_CH_2; break;
			case 2: channelBit = SC8_INPUT_GAIN_CH_3; break;
			case 3: channelBit = SC8_INPUT_GAIN_CH_4; break;
			case 4: channelBit = SC8_INPUT_GAIN_CH_5; break;
			case 5: channelBit = SC8_INPUT_GAIN_CH_6; break;
			case 6: channelBit = SC8_INPUT_GAIN_CH_7; break;
			case 7: channelBit = SC8_INPUT_GAIN_CH_8; break;
			default: channelBit = 0; break;
			}
			if ( channelBit != 0 )
			{
				WORD	data = ReadWord( m_IoWave, SC8_GAIN );

				//if ( m_Type_subVersion == 3 )
				//{
				////Version 3
					if ( ( data & channelBit ) == 0 )
					{
						pChannel->m_Position = PROPERTY_GAIN_10dBv;
					}
					else
					{
						pChannel->m_Position = PROPERTY_GAIN_4dBu;
					}
				//}
				//else
				//{
				////Version 4
				//	if ( ( data & channelBit ) == 0 )
				//	{
				//		pChannel->m_Position = PROPERTY_GAIN_4dBu;
				//	}
				//	else
				//	{
				//		pChannel->m_Position = PROPERTY_GAIN_10dBv;
				//	}
				//}
			}
			break;
		}
		default:
			break;
		}
		break;
	}
	case PROPERTY_TYPE_GAIN_DAC:
	{
		switch ( m_Type )
		{
		case CARD_TYPE_496:
		{
			if ( pChannel->m_SerialNumber == 0 )
			{
				WORD	data = ReadWord( m_IoWave, SC4_GAIN_MON );
				if ( ( data & SC496_OUTPUT_GAIN ) == 0 )
				{
					pChannel->m_Position = PROPERTY_GAIN_10dBv;
				}
				else
				{
					pChannel->m_Position = PROPERTY_GAIN_4dBu;
				}
			}
			break;
		}
		case CARD_TYPE_824:
		{
		//824 card
		//
		//SC_GSR:
		//InGain (0=-10dBV, 1=+4dBu)
			WORD	channelBit;

			switch ( pChannel->m_SerialNumber )
			{
			case 0: channelBit = SC8_OUTPUT_GAIN_CH_1; break;
			case 1: channelBit = SC8_OUTPUT_GAIN_CH_2; break;
			case 2: channelBit = SC8_OUTPUT_GAIN_CH_3; break;
			case 3: channelBit = SC8_OUTPUT_GAIN_CH_4; break;
			case 4: channelBit = SC8_OUTPUT_GAIN_CH_5; break;
			case 5: channelBit = SC8_OUTPUT_GAIN_CH_6; break;
			case 6: channelBit = SC8_OUTPUT_GAIN_CH_7; break;
			case 7: channelBit = SC8_OUTPUT_GAIN_CH_8; break;
			default: channelBit = 0; break;
			}
			if ( channelBit != 0 )
			{
				WORD	data = ReadWord( m_IoWave, SC8_GAIN );
				if ( ( data & channelBit ) == 0 )
				{
					pChannel->m_Position = PROPERTY_GAIN_10dBv;
				}
				else
				{
					pChannel->m_Position = PROPERTY_GAIN_4dBu;
				}
			}
			break;
		}
		default:
			break;
		}
		break;
	}
	case PROPERTY_TYPE_MONITORING:
	{
		switch ( m_Type )
		{
		case CARD_TYPE_424:
		case CARD_TYPE_496:
		{
			WORD	channelBit;

			switch ( pChannel->m_SerialNumber )
			{
			case 0: channelBit = SC4_MONITOR_CHANNEL_1; break;
			case 1: channelBit = SC4_MONITOR_CHANNEL_2; break;
			case 2: channelBit = SC4_MONITOR_CHANNEL_3; break;
			case 3: channelBit = SC4_MONITOR_CHANNEL_4; break;
			default: channelBit = 0; break;
			}
			if ( channelBit != 0 )
			{
				WORD	data = ReadWord( m_IoWave, SC4_GAIN_MON );
				if ( ( data & channelBit ) == 0 )
				{
					pChannel->m_Position = PROPERTY_MONITORING_DISABLE;
				}
				else
				{
					pChannel->m_Position = PROPERTY_MONITORING_ENABLE;
				}
			}
			break;
		}
		case CARD_TYPE_824:
		{
			WORD	channelBit;

			switch ( pChannel->m_SerialNumber )
			{
			case 0: channelBit = SC8_MONITOR_CH_1; break;
			case 1: channelBit = SC8_MONITOR_CH_2; break;
			case 2: channelBit = SC8_MONITOR_CH_3; break;
			case 3: channelBit = SC8_MONITOR_CH_4; break;
			case 4: channelBit = SC8_MONITOR_CH_5; break;
			case 5: channelBit = SC8_MONITOR_CH_6; break;
			case 6: channelBit = SC8_MONITOR_CH_7; break;
			case 7: channelBit = SC8_MONITOR_CH_8; break;
			default: channelBit = 0; break;
			}
			if ( channelBit != 0 )
			{
				WORD	data = ReadWord( m_IoWave, SC8_MON_MUTE );
				if ( ( data & channelBit ) == 0 )
				{
					pChannel->m_Position = PROPERTY_MONITORING_DISABLE;
				}
				else
				{
					pChannel->m_Position = PROPERTY_MONITORING_ENABLE;
				}
			}
			break;
		}
		default:
			break;
		}
		break;
	}
	case PROPERTY_TYPE_MUTE:
	{
		switch ( m_Type )
		{
		case CARD_TYPE_424:
		case CARD_TYPE_496:
		{
		//Use CODEC for mute function.
		//
		// 0x7F&(0(mute off)-1) = 0x7F
		// 0x7F&(1(mute on)-1) = 0x0
		//
		//Notice:
		//CODEC doesn't support read operation, so we can only read status variables that track mute status on 4xx cards.
			if ( pChannel->m_SerialNumber < 4 )
			{
				pChannel->m_Position = m_Property_4xxCards_Mute[pChannel->m_SerialNumber];
			}
			break;
		}
		case CARD_TYPE_824:
		{
			WORD	channelBit;

			switch ( pChannel->m_SerialNumber )
			{
			case 0: channelBit = SC8_MUTE_CH_1; break;
			case 1: channelBit = SC8_MUTE_CH_2; break;
			case 2: channelBit = SC8_MUTE_CH_3; break;
			case 3: channelBit = SC8_MUTE_CH_4; break;
			case 4: channelBit = SC8_MUTE_CH_5; break;
			case 5: channelBit = SC8_MUTE_CH_6; break;
			case 6: channelBit = SC8_MUTE_CH_7; break;
			case 7: channelBit = SC8_MUTE_CH_8; break;
			default: channelBit = 0; break;
			}
			if ( channelBit != 0 )
			{
				WORD	data = ReadWord( m_IoWave, SC8_MON_MUTE );
				if ( ( data & channelBit ) == 0 )
				{
					pChannel->m_Position = PROPERTY_MUTE_OFF;
				}
				else
				{
					pChannel->m_Position = PROPERTY_MUTE_ON;
				}
			}
			break;
		}
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
}

#pragma CODE_LOCKED
void card_c::SetChannelProperty
(
	IN ioctl_channelProperty_s*		pChannel
)
{
	switch ( pChannel->m_Property )
	{
	case PROPERTY_TYPE_GAIN_ADC:
	{
		switch ( m_Type )
		{
		case CARD_TYPE_424:
		{
			if ( pChannel->m_SerialNumber == 0 )
			{
				WORD	data = ReadWord( m_IoWave, SC4_GAIN_MON );
				if ( pChannel->m_Position == PROPERTY_GAIN_10dBv )
				{
				//-10dBv
					data &= ~( SC424_INPUT_GAIN );
				}
				else
				{
				//+4dBu
					data |= SC424_INPUT_GAIN;
				}
				WriteWord( m_IoWave, SC4_GAIN_MON, data );
			}
			break;
		}
		case CARD_TYPE_496:
		{
			if ( pChannel->m_SerialNumber == 0 )
			{
				WORD	data = ReadWord( m_IoWave, SC4_GAIN_MON );
				if ( pChannel->m_Position == PROPERTY_GAIN_10dBv )
				{
				//-10dBv
					data &= ~( SC496_INPUT_GAIN );
				}
				else
				{
				//+4dBu
					data |= SC496_INPUT_GAIN;
				}
				WriteWord( m_IoWave, SC4_GAIN_MON, data );
			}
			break;
		}
		case CARD_TYPE_824:
		{
		//824 card
		//
		//SC_GSR:
		//InGain (0=-10dBV, 1=+4dBu)
			WORD	channelBit;

			switch ( pChannel->m_SerialNumber )
			{
			case 0: channelBit = SC8_INPUT_GAIN_CH_1; break;
			case 1: channelBit = SC8_INPUT_GAIN_CH_2; break;
			case 2: channelBit = SC8_INPUT_GAIN_CH_3; break;
			case 3: channelBit = SC8_INPUT_GAIN_CH_4; break;
			case 4: channelBit = SC8_INPUT_GAIN_CH_5; break;
			case 5: channelBit = SC8_INPUT_GAIN_CH_6; break;
			case 6: channelBit = SC8_INPUT_GAIN_CH_7; break;
			case 7: channelBit = SC8_INPUT_GAIN_CH_8; break;
			default: channelBit = 0; break;
			}
			if ( channelBit != 0 )
			{
				WORD	data = ReadWord( m_IoWave, SC8_GAIN );
				if ( pChannel->m_Position == PROPERTY_GAIN_10dBv )
				{
				//-10dBv
					data &= ~( channelBit );
				}
				else
				{
				//+4dBu
					data |= channelBit;
				}
				WriteWord( m_IoWave, SC8_GAIN, data );
			}
			break;
		}
		default:
			break;
		}
		break;
	}
	case PROPERTY_TYPE_GAIN_DAC:
	{
		switch ( m_Type )
		{
		case CARD_TYPE_496:
		{
			if ( pChannel->m_SerialNumber == 0 )
			{
				WORD	data = ReadWord( m_IoWave, SC4_GAIN_MON );
				if ( pChannel->m_Position == PROPERTY_GAIN_10dBv )
				{
				//-10dBv
					data &= ~( SC496_OUTPUT_GAIN );
				}
				else
				{
				//+4dBu
					data |= SC496_OUTPUT_GAIN;
				}
				WriteWord( m_IoWave, SC4_GAIN_MON, data );
			}
			break;
		}
		case CARD_TYPE_824:
		{
		//824 card
		//
		//SC_GSR:
		//OutGain (0=-10dBV, 1=+4dBu)
			WORD	channelBit;

			switch ( pChannel->m_SerialNumber )
			{
			case 0: channelBit = SC8_OUTPUT_GAIN_CH_1; break;
			case 1: channelBit = SC8_OUTPUT_GAIN_CH_2; break;
			case 2: channelBit = SC8_OUTPUT_GAIN_CH_3; break;
			case 3: channelBit = SC8_OUTPUT_GAIN_CH_4; break;
			case 4: channelBit = SC8_OUTPUT_GAIN_CH_5; break;
			case 5: channelBit = SC8_OUTPUT_GAIN_CH_6; break;
			case 6: channelBit = SC8_OUTPUT_GAIN_CH_7; break;
			case 7: channelBit = SC8_OUTPUT_GAIN_CH_8; break;
			default: channelBit = 0; break;
			}
			if ( channelBit != 0 )
			{
				WORD	data = ReadWord( m_IoWave, SC8_GAIN );
				if ( pChannel->m_Position == PROPERTY_GAIN_10dBv )
				{
				//-10dBv
					data &= ~( channelBit );
				}
				else
				{
				//+4dBu
					data |= channelBit;
				}
				WriteWord( m_IoWave, SC8_GAIN, data );
			}
			break;
		}
		default:
			break;
		}
		break;
	}
	case PROPERTY_TYPE_MONITORING:
	{
		switch ( m_Type )
		{
		case CARD_TYPE_424:
		case CARD_TYPE_496:
		{
			WORD	channelBit;

			switch ( pChannel->m_SerialNumber )
			{
			case 0: channelBit = SC4_MONITOR_CHANNEL_1; break;
			case 1: channelBit = SC4_MONITOR_CHANNEL_2; break;
			case 2: channelBit = SC4_MONITOR_CHANNEL_3; break;
			case 3: channelBit = SC4_MONITOR_CHANNEL_4; break;
			default: channelBit = 0; break;
			}
			if ( channelBit != 0 )
			{
			//Evaluate current state...
				WORD	data = ReadWord( m_IoWave, SC4_GAIN_MON );

				//Is it different then current state?
				bool	different = false;
				if ( pChannel->m_Position == PROPERTY_MONITORING_DISABLE )
				{
				//Disable monitoring.
					if ( ( data & channelBit ) != 0 )
					{
					//Currently enabled -> disable it.
						different = true;
					}
				}
				else
				{
				//Enable monitoring.
					if ( ( data & channelBit ) == 0 )
					{
					//Currently disabled -> enable it.
						different = true;
					}
				}

			//Change state...
				if ( different == true )
				{
					if ( pChannel->m_Position == PROPERTY_MONITORING_DISABLE )
					{
					//Disable monitoring.
						data &= ~( channelBit );
					}
					else
					{
					//Enable monitoring.
						data |= channelBit;
					}
					WriteWord( m_IoWave, SC4_GAIN_MON, data );

				//Send notification to GUI.
					KIRQL		spinLock_oldLevel;

					KeAcquireSpinLock( &pDriver->m_GUI_notifyEvent_spinLock, &spinLock_oldLevel );

					//Is GUI notification set?
						if ( pDriver->m_GUI_notifyEvent != NULL )
						{
						//Yes -> set new event.
							pDriver->m_GUI_notifyEvent_master->SetEvent_ChannelProperty_Monitor( m_SerialNumber, pChannel->m_SerialNumber, pChannel->m_Position );

						//Fire notification.
							KeSetEvent( pDriver->m_GUI_notifyEvent, EVENT_INCREMENT, FALSE );
						}

					KeReleaseSpinLock( &pDriver->m_GUI_notifyEvent_spinLock, spinLock_oldLevel );
				}
			}
			break;
		}
		case CARD_TYPE_824:
		{
			WORD	channelBit;

			switch ( pChannel->m_SerialNumber )
			{
			case 0: channelBit = SC8_MONITOR_CH_1; break;
			case 1: channelBit = SC8_MONITOR_CH_2; break;
			case 2: channelBit = SC8_MONITOR_CH_3; break;
			case 3: channelBit = SC8_MONITOR_CH_4; break;
			case 4: channelBit = SC8_MONITOR_CH_5; break;
			case 5: channelBit = SC8_MONITOR_CH_6; break;
			case 6: channelBit = SC8_MONITOR_CH_7; break;
			case 7: channelBit = SC8_MONITOR_CH_8; break;
			default: channelBit = 0; break;
			}
			if ( channelBit != 0 )
			{
			//Evaluate current state...
				WORD	data = ReadWord( m_IoWave, SC8_MON_MUTE );

				//Is it different then current state?
				bool	different = false;
				if ( pChannel->m_Position == PROPERTY_MONITORING_DISABLE )
				{
				//Disable monitoring.
					if ( ( data & channelBit ) != 0 )
					{
					//Currently enabled -> disable it.
						different = true;
					}
				}
				else
				{
				//Enable monitoring.
					if ( ( data & channelBit ) == 0 )
					{
					//Currently disabled -> enable it.
						different = true;
					}
				}

			//Change state...
				if ( different == true )
				{
					if ( pChannel->m_Position == PROPERTY_MONITORING_DISABLE )
					{
					//Disable monitoring..
						data &= ~( channelBit );
					}
					else
					{
					//Enable monitoring.
						data |= channelBit;
					}
					WriteWord( m_IoWave, SC8_MON_MUTE, data );

				//Send notification to GUI.
					KIRQL		spinLock_oldLevel;

					KeAcquireSpinLock( &pDriver->m_GUI_notifyEvent_spinLock, &spinLock_oldLevel );

					//Is GUI notification set?
						if ( pDriver->m_GUI_notifyEvent != NULL )
						{
						//Yes -> set new event.
							pDriver->m_GUI_notifyEvent_master->SetEvent_ChannelProperty_Monitor( m_SerialNumber, pChannel->m_SerialNumber, pChannel->m_Position );

						//Fire notification.
							KeSetEvent( pDriver->m_GUI_notifyEvent, EVENT_INCREMENT, FALSE );
						}

					KeReleaseSpinLock( &pDriver->m_GUI_notifyEvent_spinLock, spinLock_oldLevel );
				}
			}
			break;
		}
		default:
			break;
		}
		break;
	}
	case PROPERTY_TYPE_MUTE:
	{
		switch ( m_Type )
		{
		case CARD_TYPE_424:
		case CARD_TYPE_496:
		{
		//Use CODEC for mute function.
		//
		// 0x7F&(0(mute off)-1) = 0x7F
		// 0x7F&(1(mute on)-1) = 0x0
			WORD	data = ( pChannel->m_Position == PROPERTY_MUTE_OFF ) ? 0x7F : 0x00;		//this is really Volumen is...

			switch ( pChannel->m_SerialNumber )
			{
			case 0:
				CODEC_WriteCodecsRegister( SC4_CODEC_CS1, AK_OGL, data );
				m_Property_4xxCards_Mute[0] = pChannel->m_Position;
				break;
			case 1:
				CODEC_WriteCodecsRegister( SC4_CODEC_CS1, AK_OGR, data );
				m_Property_4xxCards_Mute[1] = pChannel->m_Position;
				break;
			case 2:
				CODEC_WriteCodecsRegister( SC4_CODEC_CS2, AK_OGL, data );
				m_Property_4xxCards_Mute[2] = pChannel->m_Position;
				break;
			case 3:
				CODEC_WriteCodecsRegister( SC4_CODEC_CS2, AK_OGR, data );
				m_Property_4xxCards_Mute[3] = pChannel->m_Position;
				break;
			default:
				break;
			}
			break;
		}
		case CARD_TYPE_824:
		{
			WORD	channelBit;

			switch ( pChannel->m_SerialNumber )
			{
			case 0: channelBit = SC8_MUTE_CH_1; break;
			case 1: channelBit = SC8_MUTE_CH_2; break;
			case 2: channelBit = SC8_MUTE_CH_3; break;
			case 3: channelBit = SC8_MUTE_CH_4; break;
			case 4: channelBit = SC8_MUTE_CH_5; break;
			case 5: channelBit = SC8_MUTE_CH_6; break;
			case 6: channelBit = SC8_MUTE_CH_7; break;
			case 7: channelBit = SC8_MUTE_CH_8; break;
			default: channelBit = 0; break;
			}
			if ( channelBit != 0 )
			{
				WORD	data = ReadWord( m_IoWave, SC8_MON_MUTE );
				if ( pChannel->m_Position == PROPERTY_MUTE_OFF )
				{
				//Mute off...
					data &= ~( channelBit );
				}
				else
				{
				//Mute on...
					data |= channelBit;
				}
				WriteWord( m_IoWave, SC8_MON_MUTE, data );
			}
			break;
		}
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
}
