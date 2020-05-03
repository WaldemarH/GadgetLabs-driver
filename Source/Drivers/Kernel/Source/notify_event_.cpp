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


#pragma CODE_LOCKED
notifyEvent_c::notifyEvent_c( notifyEvent_c* pPrevious ) : linkedArray_c( (linkedArray_c*)pPrevious )
{
//Initialize variables...
	//Event...
	m_Event.m_Type = ioctl_notify_event_s::EVENT_TYPE_NO_EVENT;		//master class doesn't have have an evetn... it's a manager
}

#pragma CODE_LOCKED
notifyEvent_c::~notifyEvent_c()
{
}
