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
// PURPOSE:
//		File contains IOCTL codes for GUI<->driver interaction.
//
//***************************************************************************
//***************************************************************************
#pragma once

#define IOCTL_MAX_CARDS			4			//should be same as MAX_CARDS

//-----------------------------------------------------------------------------
//Engine version
#define IOCTL_DATA_GET_ENGINE_VERSION				CTL_CODE( FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS )
// in: nothing
//out: unsigned long -> version number
#define IOCTL_DATA_ENGINE_VERSION					21

//-----------------------------------------------------------------------------
//Cards Information
#define IOCTL_DATA_GET_CARDS_INFORMATION			CTL_CODE( FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS )
// in: nothing
//out: ioctl_cardInfo_s[IOCTL_MAX_CARDS]

__declspec(align(1)) struct ioctl_cardInfo_s
{
//-------------------------------------
//Card type
	enum CARD_TYPE
	{
		CARD_TYPE_NOT_SET = 0,
		CARD_TYPE_424,
		CARD_TYPE_496,
		CARD_TYPE_824
	};
	CARD_TYPE				m_Type;		//test for proper value if card is present
	unsigned long			m_Type_subVersion;

//-------------------------------------
//Sample Rate.
	unsigned long			m_SampleRate;

	enum CLOCK_SOURCE
	{
		CLOCK_SOURCE_NOT_SET = 0,
		CLOCK_SOURCE_CARD,			//Local card clock.
		CLOCK_SOURCE_EXTERN			//Card is synced with the syncCable.
	};
	CLOCK_SOURCE			m_ClockSource;

//-------------------------------------
//Card order.
//
//Last two define the order of the sync cable (one based number).
//If any of those is 0 than the card doesn't have
//a master or slave card (it doesn't have the sync cable connection).
	unsigned char			m_SerialNumber;					//zero based
	unsigned char			m_SerialNumber_masterCard;		//one based
	unsigned char			m_SerialNumber_slaveCard;		//one based

	unsigned char			m_Align;

//-------------------------------------
//Number of channels opened.
	unsigned char			m_Channels_opened_INs_ASIO;
	unsigned char			m_Channels_opened_OUTs_ASIO;
	unsigned char			m_Channels_opened_INs_AVStream;
	unsigned char			m_Channels_opened_OUTs_AVStream;
};

//-----------------------------------------------------------------------------
//Channel properties
#define IOCTL_DATA_GET_SET_CHANNEL_PROPERTIES		CTL_CODE( FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS )
//Get:
// in: ioctl_channelProperty_header_s+n*ioctl_channelProperty_s -> define what kind of information do you want
//out: ioctl_channelProperty_header_s+n*ioctl_channelProperty_s -> requested information is returned
//
//Set:
// in: ioctl_channelProperty_header_s+n*ioctl_channelProperty_s -> define what kind of information you want to set
//out: ioctl_channelProperty_header_s+n*ioctl_channelProperty_s -> same data is returned
//
//Notice:
//You have to define which chanels you want data for.
//So if you want the data for 4th channel of MUTE property the ioctl_channelProperty_s structure should be defined as:
//
//property.type = TYPE_MUTE;
//property.channelNumber = 3;
//property.position = 0,
//property.controlEnabled = 0;
//
//If any data of the channel will not be set correctly the data will be ignored.

__declspec(align(1)) struct ioctl_channelProperty_header_s
{
	unsigned char 				m_Request;
#define PROPERTY_GET_PROPERTY			0
#define PROPERTY_SET_PROPERTY			1

	unsigned char				m_RequestBy;
#define PROPERTY_REQUEST_BY_GUI		0
#define PROPERTY_REQUEST_BY_ASIO	1

//Card information.
	unsigned char				m_SerialNumber;				//cards zero based serial number

//Channels information.
	unsigned char				m_nPropertyDefinitions;
};
__declspec(align(1)) struct ioctl_channelProperty_s
{
//Property type.
	unsigned char			m_Property;
#define PROPERTY_TYPE_UNDEFINED			0		//this structure should be ignored
#define PROPERTY_TYPE_GAIN_ADC			1
#define PROPERTY_TYPE_GAIN_DAC			2
#define PROPERTY_TYPE_MONITORING		3
#define PROPERTY_TYPE_MUTE				4

//Channel info.
	unsigned char			m_SerialNumber;		//mono channel zero based serial number
	unsigned char			m_Position;
#define PROPERTY_GAIN_10dBv				0		//don't change the value it's set the same way as SetCheck()
#define PROPERTY_GAIN_4dBu				1

#define PROPERTY_MONITORING_DISABLE		0
#define PROPERTY_MONITORING_ENABLE		1

#define PROPERTY_MUTE_OFF				0
#define PROPERTY_MUTE_ON				1

	//Is control enabled or disabled? If disabled, GUI has to disable the monitoring of current channel.
	//
	//Notice:
	//This one is set only on PROPERTY_GET_PROPERTY and it's ignored on PROPERTY_SET_PROPERTY.
	//
	//Notice2:
	//This flag will only be updated when m_Property == PROPERTY_TYPE_MONITORING.
	//
	unsigned char			m_ControlEnabled;
#define PROPERTY_CONTROL_DISABLED		0		//don't change the value it's set the same way as EnableWindow()
#define PROPERTY_CONTROL_ENABLED		1
};

//-----------------------------------------------------------------------------
//Buffer size
#define IOCTL_DATA_GET_SET_BUFFER_SIZE				CTL_CODE( FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS )
// in: unsigned long -> if you want to set the bufferSize, set this value else set to 0
//out: unsigned long -> bufferSize(0x80, 0x100, 0x200, 0x400 <- in samples)
//
//Get:
//in <= 0
//out <= current bufferSize (0x40, 0x80, 0x100, 0x200, 0x400 <- in samples)
//
//Set:
//in <= new bufferSize (0x40, 0x80, 0x100, 0x200, 0x400 <- in samples)
//out <= 0 = failed; 'same value as in' = success
//
//Notice:
//If you want to set the bufferSize no channels should be running (or even acquired) at this time!

//-----------------------------------------------------------------------------
//Sample rate
#define IOCTL_DATA_GET_SET_SAMPLE_RATE				CTL_CODE( FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS )
// in: ioctl_asio_setSampleRate_s
//out: ioctl_asio_setSampleRate_s
//
//Get:
//in <= m_SampleRate = 0
//out => m_SampleRate = sampleRate of master card
//
//Set (reserved for ASIO driver):
//in <= m_SampleRate = new sampleRate
//out <= 0 = failed; 'same value as in' = success
//
//  Notice:
//  If card is not a master card or sampleRate is locked by other user programs the driver will FAIL the request.
//
//Supported sample rates: 44.1kHz, 48kHz, 88.2kHz, 96kHz (last two only on 4xx cards)
//
__declspec(align(1)) struct ioctl_setSampleRate_s
{
	unsigned long			m_SerialNumber;			//card's zero based serial number
	unsigned long			m_SampleRate;
};

//-----------------------------------------------------------------------------
//Cards synhronization
#define IOCTL_DATA_SET_SYNC_CARDS					CTL_CODE( FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS )
// in: unsigned long ( 0 - desync, 1 - sync )
//out: nothing
//
//Notice:
//No channel can be opened when doing this!!! If they are the command will FAIL!
//