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
#include "channel_stereo.h"


//***************************************************************************
// ProcedureName:
//		Constructor / Destructor
//
// Explanation:
//		Procedure will initialize/free stereo channel class.
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
channel_stereo_c::channel_stereo_c
(
	IN card_c*					pCard,
	IN bool						isInput,
	IN unsigned long			channelSize_inSamples,
	IN unsigned long			channelIndex,
	IN PULONG					pAddressOnCard_left,
	IN PULONG					pAddressOnCard_right
) : channel_base_c( pCard, isInput, true, channelSize_inSamples )
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

//Initialize variables.
	//Index.
	m_Index = channelIndex;

	//Addresses.
	m_pAddressOnCard_root_left = pAddressOnCard_left;
	m_pAddressOnCard_root_right = pAddressOnCard_right;

	m_pAddressOnCard_left = NULL;
	m_pAddressOnCard_right = NULL;

	m_pBuffer_Left_A = NULL;
	m_pBuffer_Left_B = NULL;

	m_pBuffer_Right_A = NULL;
	m_pBuffer_Right_B = NULL;

	m_BufferSelect = CHANNEL_STEREO_USE_BUFFER_A;

	//ASIO variables
	m_hAsio = 0;


#ifdef DBG_AND_LOGGER
	Procedure_Exit( STATUS_SUCCESS );
#endif
}

//void channel_stereo_c::~channel_stereo_c()
//{
//}
