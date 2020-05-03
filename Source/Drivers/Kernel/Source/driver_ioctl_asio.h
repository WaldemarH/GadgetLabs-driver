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

//NOTICE:
//ALL (except IOCTL_ASIO_SET_PROTECTION_IRP) commands MUST be called by 1 DeviceIoControl handle.
//Or kernel driver can run in serious problems.

//-----------------------------------------------------------------------------
//Engine version
#define IOCTL_ASIO_GET_ENGINE_VERSION				CTL_CODE( FILE_DEVICE_UNKNOWN, 0x820, METHOD_BUFFERED, FILE_ANY_ACCESS )
// in: nothing
//out: unsigned long -> version number
#define IOCTL_ASIO_ENGINE_VERSION					20

//-----------------------------------------------------------------------------
//Kernel's ASIO handle.
#define IOCTL_ASIO_GET_CLOSE_HANDLE					CTL_CODE( FILE_DEVICE_UNKNOWN, 0x821, METHOD_BUFFERED, FILE_ANY_ACCESS )
//Get:
// in: unsigned long -> 0
//out: unsigned long -> hAsio (asio handle)
//
//Close:
// in: unsigned long -> hAsio (asio handle)
//out: unsigned long -> 0
//
//Notice:
//GET: This IOCTL must be called before any other.
//CLOSE: This IOCTL must be called last.
//

//-----------------------------------------------------------------------------
//Protection agains ASIO crash.
#define IOCTL_ASIO_SET_PROTECTION_IRP				CTL_CODE( FILE_DEVICE_UNKNOWN, 0x822, METHOD_BUFFERED, FILE_ANY_ACCESS )
// in: unsigned long -> hAsio
//out: nothing
//
//This IRP will be set to protect driver agains ASIO crash ->
//if the ASIO will crash the IRP will signal that, so the driver
//can clean the already set variables.
//
//This IOCTL must be called with OVERLAPPED flag set, because the IRP will be blocked (STATUS_PENDING).
//
//This must be called after IOCTL_ASIO_CLOSE_HANDLE command.

#define IOCTL_ASIO_REMOVE_PROTECTION_IRP			CTL_CODE( FILE_DEVICE_UNKNOWN, 0x823, METHOD_BUFFERED, FILE_ANY_ACCESS )
// in: unsigned long -> hAsio
//out: nothing
//
//This IOCTL should be called before IOCTL_ASIO_CLOSE_HANDLE command.

//-----------------------------------------------------------------------------
//Open/Close channels.
#define IOCTL_ASIO_OPEN_CLOSE_CHANNELS				CTL_CODE( FILE_DEVICE_UNKNOWN, 0x824, METHOD_BUFFERED, FILE_ANY_ACCESS )
//Open:
// in: ioctl_asio_channels_header_s
//out: nothing
//
//Close:
// in: ioctl_asio_channels_header_s
//out: nothing
//
//Notice:
//On close all the channels info is ignored... every channel that is opened is also closed.
//

__declspec(align(1)) struct ioctl_asio_channels_s
{
//Card...
	unsigned char		m_Type;					//same value as ioctl_cardInfo_s::CARD_TYPE
	unsigned char		m_SerialNumber;			//card's zero based serial number

//Every bit sets one stereo channel -> to enable channel 1-2 set bit0 to 1.
	unsigned char		m_Channels_IN;
	unsigned char		m_Channels_OUT;
};
__declspec(align(1)) struct ioctl_asio_channels_header_s
{
//ASIO handle.
	unsigned long					m_hAsio;

//Request type.
	unsigned short					m_Request;
#define ASIO_CHANNELS_OPEN				0
#define ASIO_CHANNELS_CLOSE				1

//Channel sample type.
	unsigned short					m_SampleType;
#define SAMPLETYPE_32BIT_LSB24			0
#define SAMPLETYPE_32BIT_LSB			1
#define SAMPLETYPE_32BIT_MSB24			2

//Which channels are opened.
	ioctl_asio_channels_s			m_Channels_Opened[IOCTL_MAX_CARDS];
};

//-----------------------------------------------------------------------------
//Lock/Unlock memory.
#define IOCTL_ASIO_LOCK_MEMORY						CTL_CODE( FILE_DEVICE_UNKNOWN, 0x825, METHOD_BUFFERED, FILE_ANY_ACCESS )
// in: ioctl_asio_memory_s
//out: nothing
//

__declspec(align(1)) struct ioctl_asio_memory_s
{
//ASIO handle.
	unsigned long					m_hAsio;

//Memory stuff.
	unsigned long					m_Memory_Size;
	__int64							m_Memory_Address;	//this is OK even for 32 bit pointers
};

//Memory layout:
//offset							size										name
//0									sizeof( ioctl_asio_switch )					bufferSwitchData
//sizeof( ioctl_asio_switch )		4*nChannels*bufferSize*sizeof(SAMPLE)		channel_buffers
//
//Notice:
//Buffers are organized in next manner:
// card 1 INs... A_left, B_left, A_right, B_right
// card 2 INs... A_left, B_left, A_right, B_right
// ...
// card 1 OUTs... A_left, B_left, A_right, B_right
// card 2 OUTs... A_left, B_left, A_right, B_right
// ...
//
#ifndef SAMPLE_CLOCKS
__declspec(align(1)) struct clock_WORDS_s
{
	WORD			word_loLo;
	WORD			word_loHi;
	WORD			word_hiLo;
	WORD			word_hiHi;
};
__declspec(align(1)) struct clock_DWORDS_s
{
	DWORD			dword_lo;
	unsigned long	dword_hi;
};
__declspec(align(1)) union sampleClock_s
{
	ULONGLONG			clock;
	clock_WORDS_s		words;
	clock_DWORDS_s		dwords;
};
#endif //SAMPLE_CLOCKS
__declspec(align(1)) struct ioctl_asio_switch
{
//Buffer select.
//
//Selection is set regarding ASIO driver ASIOCallback->bufferSwitch function.
//So after kernel driver will fire the event m_BufferSelect will be set to the value for bufferSwitch function.
//
//This means that:
//- input buffer copy routine must copy to the other buffer
//- output buffer copy routine must use the same buffer.
	unsigned long					m_BufferSelect;
#define ASIO_SWITCH_USE_BUFFER_A		0
#define ASIO_SWITCH_USE_BUFFER_B		1

//Synhronization.
//
//If set to TRUE some buffers were lost... resynhronize ASIO driver.
//
//Notice:
//This flag must be reset by ASIO driver... it means the value was acknowledged.
	unsigned long					m_Synhronize;

//Clock and timestamp.
	sampleClock_s					m_Clock_samples;
	sampleClock_s					m_Clock_samples_first;		//timestamp: first sample
};


#define IOCTL_ASIO_UNLOCK_MEMORY					CTL_CODE( FILE_DEVICE_UNKNOWN, 0x826, METHOD_BUFFERED, FILE_ANY_ACCESS )
// in: unsigned long -> hAsio
//out: nothing

//-----------------------------------------------------------------------------
//Switch event.
#define IOCTL_ASIO_SET_SWITCH_EVENT					CTL_CODE( FILE_DEVICE_UNKNOWN, 0x827, METHOD_BUFFERED, FILE_ANY_ACCESS )
// in: ioctl_asio_switch_event
//out: nothing

__declspec(align(1)) struct ioctl_asio_switch_event
{
//ASIO handle.
	unsigned long					m_hAsio;

	unsigned long					m_Align;

//Switch event handle.
	__int64							m_SwitchEvent;		//this is OK even for 32 bit pointers
};

#define IOCTL_ASIO_RELEASE_SWITCH_EVENT				CTL_CODE( FILE_DEVICE_UNKNOWN, 0x828, METHOD_BUFFERED, FILE_ANY_ACCESS )
// in: unsigned long -> hAsio
//out: nothing


//-----------------------------------------------------------------------------
//Start/Stop..
#define IOCTL_ASIO_START							CTL_CODE( FILE_DEVICE_UNKNOWN, 0x829, METHOD_BUFFERED, FILE_ANY_ACCESS )
// in: ioctl_asio_start_header_s
//out: nothing
//
//Notice:
//Start command can only start channels that were previously opened by the IOCTL_ASIO_OPEN_CLOSE_CHANNELS.
//If a channel will be set that is not part of the Opened channels, function will fail the request.
//
__declspec(align(1)) struct ioctl_asio_start_channels_s
{
//Card...
	unsigned char		m_Type;					//same value as ioctl_cardInfo_s::CARD_TYPE
	unsigned char		m_SerialNumber;			//card's zero based serial number

//Every bit sets one MONO channel -> to enable channel 1 set bit0 to 1.
	unsigned char		m_Channels_IN;
	unsigned char		m_Channels_OUT;
};

__declspec(align(1)) struct ioctl_asio_start_header_s
{
//ASIO handle.
	unsigned long					m_hAsio;

//Channels...
	ioctl_asio_start_channels_s		m_Channels_Started[IOCTL_MAX_CARDS];
};


#define IOCTL_ASIO_STOP								CTL_CODE( FILE_DEVICE_UNKNOWN, 0x82A, METHOD_BUFFERED, FILE_ANY_ACCESS )
// in: unsigned long -> hAsio
//out: nothing


//-----------------------------------------------------------------------------
//Copy output buffer.
#define IOCTL_ASIO_COPY_OUTPUT_BUFFERS				CTL_CODE( FILE_DEVICE_UNKNOWN, 0x82B, METHOD_BUFFERED, FILE_ANY_ACCESS )
// in: unsigned long -> hAsio
//out: nothing

