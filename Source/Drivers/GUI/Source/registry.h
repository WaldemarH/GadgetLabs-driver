#pragma once
#include "..\..\Kernel\Source\driver_ioctl_data.h"


#define REGISTRY_GL_ROOT_PATH		"SOFTWARE\\Gadgetlabs"
#define REGISTRY_DATA				"GUI_Data"


__declspec(align(1)) struct registry_data_card_s
{
//Card...
	unsigned short		m_SerialNumber;			//card's zero based serial number
	unsigned short		m_Type;					//same value as ioctl_cardInfo_s::CARD_TYPE

//Channels...
	unsigned char		m_Input_Volume;			//every bit is a channel flag.... bit 0 = channel 0
	unsigned char		m_Output_Volume;
	unsigned char		m_Monitoring;
	unsigned char		m_Mute;

};
__declspec(align(1)) struct registry_data_s
{
//Cards data...
	unsigned short			m_BufferSize;
	unsigned short			m_Synced;

	registry_data_card_s	m_Cards[IOCTL_MAX_CARDS];

//Request...
	unsigned long			m_RequestDisplayed;		//display request the first time GUI window is opened by the user
};


class registry_c
{
	public: registry_c();

//Read/Write
	public: BOOL Read( registry_data_s* pData );
	public: BOOL Save( registry_data_s* pData );

//Delete
	public: BOOL DeleteKey( HKEY hKey, LPCTSTR lpSubKey );
};