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
#include "driver_ioctl_data.h"


#ifdef DBG_AND_LOGGER

void logger_c::Logger_IOCTL_Data_CardsInformation( IN char* pBuffer, IN void** ppVariables )
{
	ioctl_cardInfo_s*			pCardInfo = (ioctl_cardInfo_s*)ppVariables[0];

	for ( unsigned long i = 0; i < ELEMENTS_IN_ARRAY( pDriver->m_Cards ); i++ )
	{
		pBuffer += sprintf( pBuffer, "  ioctl_cardInfo_s ( ID = %d ):\n",  i );

		switch ( pCardInfo->m_Type )
		{
		case ioctl_cardInfo_s::CARD_TYPE_NOT_SET:
			pBuffer += sprintf( pBuffer, "    type: not set\n" );
			break;
		case ioctl_cardInfo_s::CARD_TYPE_424:
			pBuffer += sprintf( pBuffer, "    type: 424\n" );
			break;
		case ioctl_cardInfo_s::CARD_TYPE_496:
			pBuffer += sprintf( pBuffer, "    type: 496\n" );
			break;
		case ioctl_cardInfo_s::CARD_TYPE_824:
			pBuffer += sprintf( pBuffer, "    type: 824\n" );
			break;
		}

		pBuffer += sprintf( pBuffer, "    sampleRate: %d\n", pCardInfo->m_SampleRate );

		switch ( pCardInfo->m_ClockSource )
		{
		case ioctl_cardInfo_s::CLOCK_SOURCE_NOT_SET:
			pBuffer += sprintf( pBuffer, "    clockSource: not set\n" );
			break;
		case ioctl_cardInfo_s::CLOCK_SOURCE_CARD:
			pBuffer += sprintf( pBuffer, "    clockSource: local card clock.\n" );
			break;
		case ioctl_cardInfo_s::CLOCK_SOURCE_EXTERN:
			pBuffer += sprintf( pBuffer, "    clockSource: synced with the syncCable\n" );
			break;
		};

		pBuffer += sprintf( pBuffer, "    serialNumber: %d\n", pCardInfo->m_SerialNumber );
		pBuffer += sprintf( pBuffer, "    serialNumber_masterCard: %d\n", pCardInfo->m_SerialNumber_masterCard );
		pBuffer += sprintf( pBuffer, "    serialNumber_slaveCard: %d\n", pCardInfo->m_SerialNumber_slaveCard );
	}

	return;
}

#endif //DBG_AND_LOGGER
