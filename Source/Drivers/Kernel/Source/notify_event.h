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
//		File contains definitions for main class.
//
//***************************************************************************
//***************************************************************************
#pragma once

//Includes
#include "linked_array.h"
#include "driver_ioctl_asio.h"
#include "driver_ioctl_gui.h"


class notifyEvent_c : public linkedArray_c
{
	public: notifyEvent_c( notifyEvent_c* pPrevious );
	public: ~notifyEvent_c();

//Events...
	public: NTSTATUS GetEvent( ioctl_notify_event_s* pEvent );
	public: NTSTATUS SetEvent_ChannelProperty_ControlEnabled( unsigned long serialNumber_card, unsigned long serialNumber_channel, unsigned long controlEnabled );
	public: NTSTATUS SetEvent_ChannelProperty_Monitor( unsigned long serialNumber_card, unsigned long serialNumber_channel, unsigned long monitor );
	public: NTSTATUS SetEvent_ChannelsOpened( unsigned long serialNumber_card, unsigned char ASIO_in, unsigned char ASIO_out, unsigned char AVSTREAM_in, unsigned char AVSTREAM_out );
	public: NTSTATUS SetEvent_SampleRate( unsigned long serialNumber_card, unsigned long sampleRate );

	private: ioctl_notify_event_s	m_Event;
};