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

#ifdef DBG_AND_LOGGER

#pragma CODE_PAGED
void logger_c::Logger_Driver_AVStream_Start
(
	char*		pBuffer,
	void**		ppVariables
)
{
//Log card that was just added.
	card_c*		pCard = (card_c*)ppVariables[0];
	if ( pCard != NULL )
	{
		pBuffer = Logger_Card_Class( pBuffer, pCard );
	}

	return;
}


#endif //DBG_AND_LOGGER