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
char* logger_c::Logger_Card_Class
(
	char*		pBuffer,
	card_c*		pCard
)
{
	pBuffer += sprintf( pBuffer, "card_c : 0x%p\n", pCard );

	pBuffer += sprintf( pBuffer, "  DeviceObject: 0x%p\n", pCard->m_DeviceObject );

	switch ( pCard->m_Type )
	{
	case card_c::CARD_TYPE_424 :
		pBuffer += sprintf( pBuffer, "  type: 424\n" );
		break;
	case card_c::CARD_TYPE_496 :
		pBuffer += sprintf( pBuffer, "  type: 496\n" );
		break;
	case card_c::CARD_TYPE_824 :
		pBuffer += sprintf( pBuffer, "  type: 824\n" );
		break;
	default :
		pBuffer += sprintf( pBuffer, "  type: ERROR (%d)\n", pCard->m_Type );
		break;
	}

	pBuffer += sprintf( pBuffer, "  IoPlx: 0x%p\n", pCard->m_IoPlx );
	pBuffer += sprintf( pBuffer, "  IoWave: 0x%p\n", pCard->m_IoWave );
	pBuffer += sprintf( pBuffer, "  IoUart: 0x%p\n", pCard->m_IoUart );
	pBuffer += sprintf( pBuffer, "  Mem_Plx: 0x%p (length: 0x%X)\n", pCard->m_Mem_Plx, pCard->m_Mem_Plx_length );
	pBuffer += sprintf( pBuffer, "  Mem_Analog: 0x%p (length: 0x%X)\n", pCard->m_Mem_Analog, pCard->m_Mem_Analog_length );
	pBuffer += sprintf( pBuffer, "  IRQ (interrupt_s):\n" );
	pBuffer += sprintf( pBuffer, "    Level: 0x%X\n", pCard->m_IRQ.Level );
	pBuffer += sprintf( pBuffer, "    Vector: 0x%X\n", pCard->m_IRQ.Vector );
	pBuffer += sprintf( pBuffer, "    Affinity: 0x%X\n", pCard->m_IRQ.Affinity );
	pBuffer += sprintf( pBuffer, "    flags: 0x%X\n", pCard->m_IRQ.flags );

	pBuffer += sprintf( pBuffer, "  sample rate: %d\n", (int)pCard->m_SampleRate );
	pBuffer += sprintf( pBuffer, "  sample rate owned: %d\n", pCard->m_SampleRate_owned );
	switch ( pCard->m_ClockSource )
	{
	case card_c::CLOCK_SOURCE_NOT_SET:
		pBuffer += sprintf( pBuffer, "  clock source: not set\n" );
		break;
	case card_c::CLOCK_SOURCE_CARD:
		pBuffer += sprintf( pBuffer, "  clock source: local card clock\n" );
		break;
	case card_c::CLOCK_SOURCE_EXTERN:
		pBuffer += sprintf( pBuffer, "  clock source: card is synced with the syncCable\n" );
		break;
	}

	pBuffer += sprintf( pBuffer, " pCard_master: 0x%p\n", pCard->m_pCard_master );
	pBuffer += sprintf( pBuffer, " pCard_slave: 0x%p\n", pCard->m_pCard_slave );
	pBuffer += sprintf( pBuffer, " stop wave stream: %s\n", ( pCard->m_StopStream_wave == 0 ) ? "no" : "yes" );
	pBuffer += sprintf( pBuffer, " serialNumber masterCard: %d\n", pCard->m_SerialNumber_masterCard );
	pBuffer += sprintf( pBuffer, " serialNumber slaveCard: %d\n", pCard->m_SerialNumber_slaveCard );

	pBuffer = Logger_Channel_Base_Class( pBuffer, pCard->m_pChannels_All_INs );
	pBuffer = Logger_Channel_Base_Class( pBuffer, pCard->m_pChannels_All_OUTs );

	channel_stereo_c*		pChannel;
	channel_stereo_c**		ppChannelInfo = (channel_stereo_c**)&pCard->m_pChannels_Stereo_INs;
	while ( ( pChannel = *ppChannelInfo ) != NULL )
	{
		pBuffer = Logger_Channel_Stereo_Class( pBuffer, pChannel );

	//Next...
		++ppChannelInfo;
	}
	ppChannelInfo = (channel_stereo_c**)&pCard->m_pChannels_Stereo_OUTs;
	while ( ( pChannel = *ppChannelInfo ) != NULL )
	{
		pBuffer = Logger_Channel_Stereo_Class( pBuffer, pChannel );

	//Next...
		++ppChannelInfo;
	}

	return pBuffer;
}

#endif //DBG_AND_LOGGER