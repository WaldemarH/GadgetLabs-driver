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

void logger_c::Logger_IOCTL_ASIO_Start( IN char* pBuffer, IN void** ppVariables )
{
	unsigned long				handleID = *(unsigned long*)ppVariables[0];
	channel_stereo_c**			ppChannels_Stereo_INs = (channel_stereo_c**)ppVariables[1];
	channel_stereo_c**			ppChannels_Stereo_OUTs = (channel_stereo_c**)ppVariables[2];

	pBuffer += sprintf( pBuffer, "\n  asioHandle_c ( ID = %d ):\n",  handleID );

	for ( int i = 0; i < (MAX_CARDS*MAX_STEREO_PAIRS_PER_CARD+1); i++ )
	{
		pBuffer = Logger_Channel_Stereo_Class( pBuffer, ppChannels_Stereo_INs[i] );
	}

	for ( int i = 0; i < (MAX_CARDS*MAX_STEREO_PAIRS_PER_CARD+1); i++ )
	{
		pBuffer = Logger_Channel_Stereo_Class( pBuffer, ppChannels_Stereo_OUTs[i] );
	}

	return;
}

#endif //DBG_AND_LOGGER
