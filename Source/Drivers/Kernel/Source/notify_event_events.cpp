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
#include "notify_event.h"


#pragma DATA_LOCKED


//***************************************************************************
// ProcedureName:
//		GetEvent
//
// Explanation:
//		Procedure will return first event in queue and then it will remove it
//		from the queue.
//
// IRQ Level:
//		ANY
//
// Synhronization:
//		None
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return Value:
//		...
//
#pragma CODE_LOCKED
NTSTATUS notifyEvent_c::GetEvent( ioctl_notify_event_s* pEvent )
{
	NTSTATUS		Status = STATUS_SUCCESS;

//Validate...
	if (
	  ( m_pPrevious != NULL )
	  ||
	  ( pEvent == NULL )
	)
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//Get event if exist.
	if ( SUCCESS( Status ) )
	{
	//Is there an event in queue?
		if ( m_pNext != NULL )
		{
		//Yes... copy it's data and remove it from queue.
			//Copy...
			RtlCopyMemory( pEvent, &( (notifyEvent_c*)m_pNext )->m_Event, sizeof( ( (notifyEvent_c*)m_pNext )->m_Event ) );

			//Remove it...
			delete m_pNext;
		}
		else
		{
		//No... signal that there are no more events in queue.
			pEvent->m_Type = ioctl_notify_event_s::EVENT_TYPE_NO_EVENT;
		}
	}

	return Status;
}

#pragma CODE_LOCKED
NTSTATUS notifyEvent_c::SetEvent_ChannelProperty_ControlEnabled
(
	unsigned long		serialNumber_card,
	unsigned long		serialNumber_channel,
	unsigned long		controlEnabled
)
{
	NTSTATUS		Status = STATUS_SUCCESS;

//Validate...
	if ( m_pPrevious != NULL )
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//Create new event.
	notifyEvent_c*		pEvent;

	if ( SUCCESS( Status ) )
	{
		pEvent = new notifyEvent_c( (notifyEvent_c*)GetLast() );
		if ( pEvent == NULL )
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
		}
	}

//Set event.
	if ( SUCCESS( Status ) )
	{
		pEvent->m_Event.m_Type = ioctl_notify_event_s::EVENT_TYPE_CHANNEL_PROPERTY_CONTROL_ENABLED;

		pEvent->m_Event.m_Data.m_ChannelProperty.m_Card_serialNumber = (unsigned char)serialNumber_card;
		pEvent->m_Event.m_Data.m_ChannelProperty.m_Channel_serialNumber = (unsigned char)serialNumber_channel;
		pEvent->m_Event.m_Data.m_ChannelProperty.m_Property = (unsigned char)controlEnabled;
	}

	return Status;
}

#pragma CODE_LOCKED
NTSTATUS notifyEvent_c::SetEvent_ChannelProperty_Monitor
(
	unsigned long		serialNumber_card,
	unsigned long		serialNumber_channel,
	unsigned long		monitor
)
{
	NTSTATUS		Status = STATUS_SUCCESS;

//Validate...
	if ( m_pPrevious != NULL )
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//Create new event.
	notifyEvent_c*		pEvent;

	if ( SUCCESS( Status ) )
	{
		pEvent = new notifyEvent_c( (notifyEvent_c*)GetLast() );
		if ( pEvent == NULL )
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
		}
	}

//Set event.
	if ( SUCCESS( Status ) )
	{
		pEvent->m_Event.m_Type = ioctl_notify_event_s::EVENT_TYPE_CHANNEL_PROPERTY_MONITOR;

		pEvent->m_Event.m_Data.m_ChannelProperty.m_Card_serialNumber = (unsigned char)serialNumber_card;
		pEvent->m_Event.m_Data.m_ChannelProperty.m_Channel_serialNumber = (unsigned char)serialNumber_channel;
		pEvent->m_Event.m_Data.m_ChannelProperty.m_Property = (unsigned char)monitor;
	}

	return Status;
}

#pragma CODE_LOCKED
NTSTATUS notifyEvent_c::SetEvent_ChannelsOpened
(
	unsigned long		serialNumber_card,
	unsigned char		ASIO_in,
	unsigned char		ASIO_out,
	unsigned char		AVSTREAM_in,
	unsigned char		AVSTREAM_out
)
{
	NTSTATUS		Status = STATUS_SUCCESS;

//Validate...
	if ( m_pPrevious != NULL )
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//Create new event.
	notifyEvent_c*		pEvent;

	if ( SUCCESS( Status ) )
	{
		pEvent = new notifyEvent_c( (notifyEvent_c*)GetLast() );
		if ( pEvent == NULL )
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
		}
	}

//Set event.
	if ( SUCCESS( Status ) )
	{
		pEvent->m_Event.m_Type = ioctl_notify_event_s::EVENT_TYPE_CHANNEL_NUMBER_CHANGED;

		pEvent->m_Event.m_Data.m_ChannelsOpened.m_Card_serialNumber = serialNumber_card;
		pEvent->m_Event.m_Data.m_ChannelsOpened.m_Channels_opened_INs_ASIO = ASIO_in;
		pEvent->m_Event.m_Data.m_ChannelsOpened.m_Channels_opened_OUTs_ASIO = ASIO_out;
		pEvent->m_Event.m_Data.m_ChannelsOpened.m_Channels_opened_INs_AVStream = AVSTREAM_in;
		pEvent->m_Event.m_Data.m_ChannelsOpened.m_Channels_opened_OUTs_AVStream = AVSTREAM_out;
	}

	return Status;
}

#pragma CODE_LOCKED
NTSTATUS notifyEvent_c::SetEvent_SampleRate
(
	unsigned long		serialNumber_card,
	unsigned long		sampleRate
)
{
	NTSTATUS		Status = STATUS_SUCCESS;

//Validate...
	if ( m_pPrevious != NULL )
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//Create new event.
	notifyEvent_c*		pEvent;

	if ( SUCCESS( Status ) )
	{
		pEvent = new notifyEvent_c( (notifyEvent_c*)GetLast() );
		if ( pEvent == NULL )
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
		}
	}

//Set event.
	if ( SUCCESS( Status ) )
	{
		pEvent->m_Event.m_Type = ioctl_notify_event_s::EVENT_TYPE_SAMPLE_RATE_CHANGED;

		pEvent->m_Event.m_Data.m_SampleRate.m_Card_serialNumber = serialNumber_card;
		pEvent->m_Event.m_Data.m_SampleRate.m_SampleRate = sampleRate;
	}

	return Status;
}
