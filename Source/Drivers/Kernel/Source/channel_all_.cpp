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
//      card.cpp
//
// PURPOSE:
//      File contains the channel class functions.
//
// STATUS:
//      Unfinished/Untested.
//
//***************************************************************************
//***************************************************************************
#include "main.h"
#include "channel_all.h"


//***************************************************************************
// ProcedureName:
//		Constructor / Destructor
//
// Explanation:
//		Procedure will initialize/free class.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// Synhronization:
//		None
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return Codes:
//		Non.
//
#pragma CODE_PAGED
channel_all_c::channel_all_c
(
	IN card_c*					pCard,
	IN bool						isInput,
	IN unsigned long			channelSize_inSamples,
	IN PULONG					pAddressOnCard[8]
) : channel_base_c( pCard, isInput, false, channelSize_inSamples )
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

//Initialize variables.
	//Addresses.
	m_pAddressOnCard_root[0] = pAddressOnCard[0];
	m_pAddressOnCard_root[1] = pAddressOnCard[1];
	m_pAddressOnCard_root[2] = pAddressOnCard[2];
	m_pAddressOnCard_root[3] = pAddressOnCard[3];
	m_pAddressOnCard_root[4] = pAddressOnCard[4];
	m_pAddressOnCard_root[5] = pAddressOnCard[5];
	m_pAddressOnCard_root[6] = pAddressOnCard[6];
	m_pAddressOnCard_root[7] = pAddressOnCard[7];

	RtlZeroMemory( &m_pAddressOnCard, sizeof( m_pAddressOnCard ) );

	m_pBuffer_all = NULL;

#ifdef DBG_AND_LOGGER
	Procedure_Exit( STATUS_SUCCESS );
#endif
}

//channel_all_c::~channel_all_c()
//{
//}
