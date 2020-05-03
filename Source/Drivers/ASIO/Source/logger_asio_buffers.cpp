#include "main.h"
#include "logger.h"
#include "gl_asio.h"

#ifdef ASIO_LOGGER
#pragma warning( disable : 4996 ) 

void logger_c::Logger_CreateBuffers( IN char* pBuffer, IN void** ppVariables )
{
//Get channel.
	channelInfo_s*		pChannel = (channelInfo_s*)ppVariables[0];
	unsigned long		leftRight = *(unsigned long*)ppVariables[1];

//Log...
	pBuffer += sprintf( pBuffer, "  ChannelInfo: 0x%p\n", pChannel );
	pBuffer += sprintf( pBuffer, "    m_SerialNumber_card     = %d\n", pChannel->m_SerialNumber_card );
	pBuffer += sprintf( pBuffer, "    m_SerialNumber_channel  = %d\n", pChannel->m_SerialNumber_channel );

	//ASIO Channel Info...
	ASIOChannelInfo*		pChannel_ASIO;
	if ( leftRight == 0 )
	{
	//Left...
		pChannel_ASIO = &pChannel->m_ChannelInfo_left;

		pBuffer += sprintf( pBuffer, "    ASIO Channel: left\n" );
	}
	else
	{
	//Right...
		pChannel_ASIO = &pChannel->m_ChannelInfo_right;

		pBuffer += sprintf( pBuffer, "    ASIO Channel: right\n" );
	}

	pBuffer += sprintf( pBuffer, "      channel      = %d\n", pChannel_ASIO->channel );
	pBuffer += sprintf( pBuffer, "      isInput      = %s\n", ( pChannel_ASIO->isInput == ASIOFalse )?"No":"Yes" );
	pBuffer += sprintf( pBuffer, "      isActive     = %s\n", ( pChannel_ASIO->isActive == ASIOFalse )?"No":"Yes" );
	pBuffer += sprintf( pBuffer, "      channelGroup = %d\n", pChannel_ASIO->channelGroup );
	switch ( pChannel_ASIO->type )
	{
	case ASIOSTInt32LSB24 :
		pBuffer += sprintf( pBuffer, "      type         = ASIOSTInt32LSB24\n" );
		break;
	case ASIOSTInt32LSB :
		pBuffer += sprintf( pBuffer, "      type         = ASIOSTInt32LSB\n" );
		break;
	case ASIOSTInt32MSB24 :
		pBuffer += sprintf( pBuffer, "      type         = ASIOSTInt32MSB24\n" );
		break;
	default:
		pBuffer += sprintf( pBuffer, "      type         = Unknown (%d)\n", pChannel_ASIO->type );
		break;
	}
	pBuffer += sprintf( pBuffer, "      name         = %s\n", pChannel_ASIO->name );

	//Buffers...
	if ( leftRight == 0 )
	{
	//Left...
		pBuffer += sprintf( pBuffer, "      m_pBuffer_A_left = 0x%p\n", pChannel->m_pBuffer_A_left );
		pBuffer += sprintf( pBuffer, "      m_pBuffer_B_left = 0x%p\n", pChannel->m_pBuffer_B_left );
	}
	else
	{
	//Right...
		pBuffer += sprintf( pBuffer, "      m_pBuffer_A_right = 0x%p\n", pChannel->m_pBuffer_A_right );
		pBuffer += sprintf( pBuffer, "      m_pBuffer_B_right = 0x%p\n", pChannel->m_pBuffer_B_right );
	}

	return;
}

#endif //ASIO_LOGGER