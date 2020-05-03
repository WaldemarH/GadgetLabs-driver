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

//-----------------------------------------------------------------------------
//Engine version
#define IOCTL_GUI_GET_ENGINE_VERSION				CTL_CODE( FILE_DEVICE_UNKNOWN, 0x810, METHOD_BUFFERED, FILE_ANY_ACCESS )
// in: nothing
//out: unsigned long -> version number
#define IOCTL_GUI_ENGINE_VERSION					12

//-----------------------------------------------------------------------------
//Protection agains GUI crash.
#define IOCTL_GUI_SET_PROTECTION_IRP				CTL_CODE( FILE_DEVICE_UNKNOWN, 0x811, METHOD_BUFFERED, FILE_ANY_ACCESS )
// in: nothing
//out: nothing
//
//This IRP will be set to protect driver agains ControlPanel crash ->
//if the ControlPanel will crash the IRP will signal that, so the driver
//can clean the already set variables.
//
//This IOCTL must be called with OVERLAPPED flag set, because the IRP will be blocked (STATUS_PENDING).
//
//This must be called before any other ControlPanel controlCode.
//
//Notice:
//The driver supports only 1 pending IRP for ControlPanel part, so if
//user will want to open 2 ControlPanels the driver will fail this call.

#define IOCTL_GUI_REMOVE_PROTECTION_IRP				CTL_CODE( FILE_DEVICE_UNKNOWN, 0x812, METHOD_BUFFERED, FILE_ANY_ACCESS )
// in: nothing
//out: nothing
//
//This IOCTL should be the last command that is called from GUI when closing.
//
//Notice:
//If GUI wasn't able to de-sync the cards this call will force the de-sync.

//-----------------------------------------------------------------------------
//Notifications to GUI from driver.
#define IOCTL_GUI_SET_EVENT_NOTIFICATION			CTL_CODE( FILE_DEVICE_UNKNOWN, 0x813, METHOD_BUFFERED, FILE_ANY_ACCESS )
// in: __int64 -> hEvent_common (this event is used for all notification from driver)
//out: nothing

#define IOCTL_GUI_RELEASE_EVENT_NOTIFICATION		CTL_CODE( FILE_DEVICE_UNKNOWN, 0x814, METHOD_BUFFERED, FILE_ANY_ACCESS )
// in: nothing
//out: nothing
//
//GUI will send this IOCTL when GUI is closing.

#define IOCTL_GUI_GET_NOTIFICATION_EVENT			CTL_CODE( FILE_DEVICE_UNKNOWN, 0x815, METHOD_BUFFERED, FILE_ANY_ACCESS )
// in: nothing
//out: ioctl_notify_event_s
//
//Notice:
//GUI can call this command even if there is no event present.
//
//Procedure:
//1. GUI will receive notification from the driver.
//2. GUI will call this command until it will receive EVENT_TYPE_NO_EVENT event.
//3. GUI notification thread can go into sleep
//
__declspec(align(1)) struct ioctl_notify_event_s
{
//Event type
	enum EVENT_TYPE
	{
		EVENT_TYPE_NO_EVENT = 0,
		EVENT_TYPE_SAMPLE_RATE_CHANGED,
		EVENT_TYPE_CHANNEL_PROPERTY_CONTROL_ENABLED,		//intended to notify GUI to enable/disable monitoring checkbox (if ASIO opens a channel)
		EVENT_TYPE_CHANNEL_PROPERTY_MONITOR,				//intended to notify GUI about current input monitoring status
		EVENT_TYPE_CHANNEL_NUMBER_CHANGED					//number of opened channels
	};
	EVENT_TYPE			m_Type;

//Event data.
	union
	{
		__declspec(align(1)) struct ioctl_notify_event_sampleRate_s
		{
		//Card info...
			unsigned long		m_Card_serialNumber;		//card zero based serial number

		//Event info...
			unsigned long		m_SampleRate;
		} m_SampleRate;
		__declspec(align(1)) struct ioctl_notify_event_channelProperty_s	//notification of ASIO driver to GUI
		{
		//Card info...
			unsigned char		m_Card_serialNumber;		//card zero based serial number

		//Channel info...
			unsigned char		m_Channel_serialNumber;		//channel zero base serial number

			unsigned char		m_Property;
		#define PROPERTY_CONTROL_DISABLED		0		//don't change the value it's set the same way as EnableWindow()
		#define PROPERTY_CONTROL_ENABLED		1

		#define PROPERTY_MONITORING_DISABLE		0
		#define PROPERTY_MONITORING_ENABLE		1
		} m_ChannelProperty;
		__declspec(align(1)) struct ioctl_notify_event_channelNumberChanged_s
		{
		//Card info...
			unsigned long		m_Card_serialNumber;		//card zero based serial number

		//Event info.
			unsigned char		m_Channels_opened_INs_ASIO;
			unsigned char		m_Channels_opened_OUTs_ASIO;
			unsigned char		m_Channels_opened_INs_AVStream;
			unsigned char		m_Channels_opened_OUTs_AVStream;
		} m_ChannelsOpened;
	} m_Data;
};
