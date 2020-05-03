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

void logger_c::Logger_Driver_AVStream_Pin_SetDeviceState( IN char* pBuffer, IN void** ppVariables )
{
//FromState
	switch ( *(KSSTATE*)ppVariables[1] )
	{
	case KSSTATE_STOP :
		pBuffer += sprintf( pBuffer, "  FromState: STOP\n" );
		break;
	case KSSTATE_ACQUIRE :
		pBuffer += sprintf( pBuffer, "  FromState: ACQUIRE\n" );
		break;
	case KSSTATE_PAUSE :
		pBuffer += sprintf( pBuffer, "  FromState: PAUSE\n" );
		break;
	case KSSTATE_RUN :
		pBuffer += sprintf( pBuffer, "  FromState: RUN\n" );
		break;
	default:
		pBuffer += sprintf( pBuffer, "  FromState: ERROR (%d)\n", *(KSSTATE*)ppVariables[1] );
		break;
	}
//ToState
	switch ( *(KSSTATE*)ppVariables[0] )
	{
	case KSSTATE_STOP :
		pBuffer += sprintf( pBuffer, "  ToState: STOP\n" );
		break;
	case KSSTATE_ACQUIRE :
		pBuffer += sprintf( pBuffer, "  ToState: ACQUIRE\n" );
		break;
	case KSSTATE_PAUSE :
		pBuffer += sprintf( pBuffer, "  ToState: PAUSE\n" );
		break;
	case KSSTATE_RUN :
		pBuffer += sprintf( pBuffer, "  ToState: RUN\n" );
		break;
	default:
		pBuffer += sprintf( pBuffer, "  ToState: ERROR (%d)\n", *(KSSTATE*)ppVariables[0] );
		break;
	}

//Parse.
	channel_base_c*		pChannel = (channel_base_c*)ppVariables[2];
	if ( pChannel != NULL )
	{
		pBuffer = Logger_Channel_Base_Class( pBuffer, pChannel );
	}
	return;
}

#endif //DBG_AND_LOGGER
