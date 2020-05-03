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
#include "sync_start.h"


#pragma DATA_LOCKED


#pragma CODE_LOCKED
syncStart_c::syncStart_c( syncStart_c* pPrevious ) : linkedArray_c( (linkedArray_c*)pPrevious )
{
//Initialize variables...
	//Channels...
	m_Channels_nReferences = 0;
	m_Channels_nInRunningState = 0;

	RtlZeroMemory( &m_ppChannels, sizeof( m_ppChannels ) );
}

#pragma CODE_LOCKED
syncStart_c::~syncStart_c()
{
}
