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
void logger_c::Logger_Card_AcquireResources
(
	char*		pBuffer,
	void**		ppVariables
)
{
	card_c*		pCard = (card_c*)ppVariables[0];

	pBuffer += sprintf( pBuffer, "  IoPlx: 0x%p\n", pCard->m_IoPlx );
	pBuffer += sprintf( pBuffer, "  IoWave: 0x%p\n", pCard->m_IoWave );
	pBuffer += sprintf( pBuffer, "  IoUart: 0x%p\n", pCard->m_IoUart );
	pBuffer += sprintf( pBuffer, "  IRQ.Level: 0x%X\n", pCard->m_IRQ.Level );
	pBuffer += sprintf( pBuffer, "  IRQ.Vector: 0x%X\n", pCard->m_IRQ.Vector );
	pBuffer += sprintf( pBuffer, "  IRQ.Affinity: 0x%X\n", pCard->m_IRQ.Affinity );
	pBuffer += sprintf( pBuffer, "  IRQ.flags: 0x%X\n", pCard->m_IRQ.flags );
	pBuffer += sprintf( pBuffer, "  Mem_Plx: 0x%p (length: 0x%X)\n", pCard->m_Mem_Plx, pCard->m_Mem_Plx_length );
	pBuffer += sprintf( pBuffer, "  Mem_Analog: 0x%p (length: 0x%X)\n", pCard->m_Mem_Analog, pCard->m_Mem_Analog_length );
	return;
}

#pragma CODE_PAGED
void logger_c::Logger_Card_SetCardType
(
	char*		pBuffer,
	void**		ppVariables
)
{
	unsigned long	cardType;
	unsigned long	cardType_sub;

	switch ( *( (card_c::CARD_TYPE*)ppVariables[0] ) )
	{
	case card_c::CARD_TYPE_424 :
		cardType = 424;
		break;
	case card_c::CARD_TYPE_496 :
		cardType = 496;
		break;
	case card_c::CARD_TYPE_824 :
		cardType = 824;
		break;
	default:
		cardType = 0;
		break;
	}
	cardType_sub = *( (unsigned long*)ppVariables[1] );

	sprintf( pBuffer, "  Card Type: %d (sub: %d)\n", cardType, cardType_sub );
	return;
}

#endif //DBG_AND_LOGGER