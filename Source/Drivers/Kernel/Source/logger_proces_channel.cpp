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
#include "channel_all.h"
#include "channel_stereo.h"

#ifdef DBG_AND_LOGGER

#pragma DATA_PAGED

#pragma CODE_PAGED
char* logger_c::Logger_Channel_All_Class
(
	char*				pBuffer,
	channel_all_c*		pChannel
)
{
	pBuffer += sprintf( pBuffer, "  channel_all_c : 0x%p\n", pChannel );

	//pBuffer += sprintf( pBuffer, "    AddressOnCard ch.1: 0x%p\n", pChannel->m_pAddressOnCard[0] );
	//pBuffer += sprintf( pBuffer, "    AddressOnCard ch.2: 0x%p\n", pChannel->m_pAddressOnCard[1] );
	//pBuffer += sprintf( pBuffer, "    AddressOnCard ch.3: 0x%p\n", pChannel->m_pAddressOnCard[2] );
	//pBuffer += sprintf( pBuffer, "    AddressOnCard ch.4: 0x%p\n", pChannel->m_pAddressOnCard[3] );
	//pBuffer += sprintf( pBuffer, "    AddressOnCard ch.5: 0x%p\n", pChannel->m_pAddressOnCard[4] );
	//pBuffer += sprintf( pBuffer, "    AddressOnCard ch.6: 0x%p\n", pChannel->m_pAddressOnCard[5] );
	//pBuffer += sprintf( pBuffer, "    AddressOnCard ch.7: 0x%p\n", pChannel->m_pAddressOnCard[6] );
	//pBuffer += sprintf( pBuffer, "    AddressOnCard ch.8: 0x%p\n", pChannel->m_pAddressOnCard[7] );

	pBuffer = Logger_Channel_Base_Class( pBuffer, pChannel );

	return pBuffer;
}

#pragma CODE_PAGED
char* logger_c::Logger_Channel_Stereo_Class
(
	char*				pBuffer,
	channel_stereo_c*	pChannel
)
{
	pBuffer += sprintf( pBuffer, "  channel_stereo_c : 0x%p\n", pChannel );

	if ( pChannel != NULL )
	{
		pBuffer += sprintf( pBuffer, "    index: %d\n", pChannel->m_Index );

		pBuffer += sprintf( pBuffer, "    AddressOnCard root left: 0x%p\n", pChannel->m_pAddressOnCard_root_left );
		pBuffer += sprintf( pBuffer, "    AddressOnCard root right: 0x%p\n", pChannel->m_pAddressOnCard_root_right );
		pBuffer += sprintf( pBuffer, "    AddressOnCard left: 0x%p\n", pChannel->m_pAddressOnCard_left );
		pBuffer += sprintf( pBuffer, "    AddressOnCard right: 0x%p\n", pChannel->m_pAddressOnCard_right );
		pBuffer += sprintf( pBuffer, "    Buffer A left: 0x%p\n", pChannel->m_pBuffer_Left_A );
		pBuffer += sprintf( pBuffer, "    Buffer B left: 0x%p\n", pChannel->m_pBuffer_Left_B );
		pBuffer += sprintf( pBuffer, "    Buffer A right: 0x%p\n", pChannel->m_pBuffer_Right_A );
		pBuffer += sprintf( pBuffer, "    Buffer B right: 0x%p\n", pChannel->m_pBuffer_Right_B );
		pBuffer += sprintf( pBuffer, "    BufferSelect: %d\n", pChannel->m_BufferSelect );
		pBuffer += sprintf( pBuffer, "    hAsio: %d\n", pChannel->m_hAsio );

		pBuffer = Logger_Channel_Base_Class( pBuffer, pChannel );
	}

	return pBuffer;
}

#pragma CODE_PAGED
char* logger_c::Logger_Channel_Base_Class
(
	char*				pBuffer,
	channel_base_c*		pChannel
)
{
	pBuffer += sprintf( pBuffer, "    channel_base_c\n" );
	pBuffer += sprintf( pBuffer, "      m_pCard(%d): 0x%p\n", pChannel->m_pCard->m_SerialNumber, pChannel->m_pCard );

	switch ( pChannel->m_Type )
	{
	case channel_base_c::CHANNEL_TYPE_IN:
		pBuffer += sprintf( pBuffer, "      type: input\n" );
		break;
	case channel_base_c::CHANNEL_TYPE_OUT:
		pBuffer += sprintf( pBuffer, "      type: output\n" );
		break;
	}
	switch ( pChannel->m_Format )
	{
	case channel_base_c::CHANNEL_FORMAT_STEREO:
		pBuffer += sprintf( pBuffer, "      format: stereo\n" );
		break;
	case channel_base_c::CHANNEL_FORMAT_SURROUND:
		pBuffer += sprintf( pBuffer, "      format: surround\n" );
		break;
	}
	switch ( pChannel->m_Type_client )
	{
	case channel_base_c::CHANNEL_TYPE_CLIENT_NON:
		pBuffer += sprintf( pBuffer, "      client: non\n" );
		break;
	case channel_base_c::CHANNEL_TYPE_CLIENT_AVSTREAM:
		pBuffer += sprintf( pBuffer, "      client: AVStream\n" );
		break;
	case channel_base_c::CHANNEL_TYPE_CLIENT_ASIO:
		pBuffer += sprintf( pBuffer, "      client: ASIO\n" );
		break;
	}
	switch ( pChannel->m_BitSize )
	{
	case channel_base_c::CHANNEL_BIT_SIZE_16:
		pBuffer += sprintf( pBuffer, "      bit size: 16\n" );
		break;
	case channel_base_c::CHANNEL_BIT_SIZE_24:
		pBuffer += sprintf( pBuffer, "      bit size: 24\n" );
		break;
	case channel_base_c::CHANNEL_BIT_SIZE_32:
		pBuffer += sprintf( pBuffer, "      bit size: 32\n" );
		break;
	}
	
	switch ( pChannel->m_State )
	{
	case KSSTATE_STOP:
		pBuffer += sprintf( pBuffer, "      state: stop\n" );
		break;
	case KSSTATE_ACQUIRE:
		pBuffer += sprintf( pBuffer, "      state: acquire\n" );
		break;
	case KSSTATE_PAUSE:
		pBuffer += sprintf( pBuffer, "      state: pause\n" );
		break;
	case KSSTATE_RUN:
		pBuffer += sprintf( pBuffer, "      state: run\n" );
		break;
	}

	pBuffer += sprintf( pBuffer, "    channel size: %d\n", pChannel->m_Channel_size );

	pBuffer += sprintf( pBuffer, "      m_pCopyBuffersProcedure: 0x%p\n", pChannel->m_pCopyBuffersProcedure );

	return pBuffer;
}

#endif //DBG_AND_LOGGER
