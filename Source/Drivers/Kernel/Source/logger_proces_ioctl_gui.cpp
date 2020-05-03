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
//		File contains definitions for logger and debugPrint functions.
//
//***************************************************************************
//***************************************************************************
#include "main.h"
#include "driver_ioctl_gui.h"


#ifdef DBG_AND_LOGGER

void logger_c::Logger_IOCTL_Gui_Notification_GetEvent( IN char* pBuffer, IN void** ppVariables )
{
//Which event was send?
	ioctl_notify_event_s*		pEvent = (ioctl_notify_event_s*)ppVariables[0];
	
	switch ( pEvent->m_Type )
	{
	case ioctl_notify_event_s::EVENT_TYPE_NO_EVENT:
	{
		pBuffer += sprintf( pBuffer, "GUI Notification Event Type: NO_EVENT\n" );
		break;
	}
	case ioctl_notify_event_s::EVENT_TYPE_SAMPLE_RATE_CHANGED:
	{
		pBuffer += sprintf( pBuffer, "GUI Notification Event Type: SAMPLE_RATE_CHANGED\n" );
		pBuffer += sprintf( pBuffer, "    Card_serialNumber: %d\n", pEvent->m_Data.m_SampleRate.m_Card_serialNumber );
		pBuffer += sprintf( pBuffer, "    SampleRate: %d\n", pEvent->m_Data.m_SampleRate.m_SampleRate );
		break;
	}
	case ioctl_notify_event_s::EVENT_TYPE_CHANNEL_PROPERTY_CONTROL_ENABLED:
	{
		pBuffer += sprintf( pBuffer, "GUI Notification Event Type: CHANNEL_PROPERTY_CONTROL_ENABLED\n" );
		pBuffer += sprintf( pBuffer, "    Card_serialNumber: %d\n", pEvent->m_Data.m_ChannelProperty.m_Card_serialNumber );
		pBuffer += sprintf( pBuffer, "    Channel_serialNumber: %d\n", pEvent->m_Data.m_ChannelProperty.m_Channel_serialNumber );
		pBuffer += sprintf( pBuffer, "    Property: %d\n", pEvent->m_Data.m_ChannelProperty.m_Property );
		break;
	}
	case ioctl_notify_event_s::EVENT_TYPE_CHANNEL_PROPERTY_MONITOR:
	{
		pBuffer += sprintf( pBuffer, "GUI Notification Event Type: CHANNEL_PROPERTY_MONITOR\n" );
		pBuffer += sprintf( pBuffer, "    Card_serialNumber: %d\n", pEvent->m_Data.m_ChannelProperty.m_Card_serialNumber );
		pBuffer += sprintf( pBuffer, "    Channel_serialNumber: %d\n", pEvent->m_Data.m_ChannelProperty.m_Channel_serialNumber );
		pBuffer += sprintf( pBuffer, "    Property: %d\n", pEvent->m_Data.m_ChannelProperty.m_Property );
		break;
	}
	default:
	{
		break;
	}
	}

	return;
}

#endif //DBG_AND_LOGGER
