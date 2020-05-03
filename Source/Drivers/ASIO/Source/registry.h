#pragma once

//Includes...
#include "..\..\Kernel\Source\driver_ioctl_data.h"


#define REGISTRY_GL_ASIO_PATH		"SOFTWARE\\Gadgetlabs\\ASIO"


__declspec(align(1)) struct registry_data_channels_s
{
//Card...
	unsigned char		m_Type;					//same value as ioctl_cardInfo_s::CARD_TYPE
	unsigned char		m_SerialNumber;			//card's zero based serial number

//Every bit sets one stereo channel -> to enable channel 1-2 set bit0 to 1.
	unsigned char		m_Channels_IN;
	unsigned char		m_Channels_OUT;
};
__declspec(align(1)) struct registry_data_s
{
//Standard defines.
	unsigned long					m_SampleType;

//Which channels are opened.
	registry_data_channels_s		m_Channels_Opened[IOCTL_MAX_CARDS];
};


class registry_c
{
	public: registry_c();

//Read/Write
	public: BOOL Read( registry_data_s* pData );
	public: BOOL Save( registry_data_s* pData );

	private: BOOL GetHostPath( char hostPath[1024] );
};