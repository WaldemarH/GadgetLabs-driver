#include "main.h"
#include "logger.h"


#ifdef ASIO_LOGGER
#pragma warning( disable : 4996 ) 

void logger_c::Logger_GetChannelInfo( IN char* pBuffer, IN void** ppVariables )
{
//Get channel.
	ASIOChannelInfo*	pChannel = (ASIOChannelInfo*)ppVariables[0];

//Log...
	pBuffer += sprintf( pBuffer, "  ChannelInfo: 0x%p\n", pChannel );
	pBuffer += sprintf( pBuffer, "    channel      = %d\n", pChannel->channel );
	pBuffer += sprintf( pBuffer, "    isInput      = %s\n", ( pChannel->isInput == ASIOFalse )?"No":"Yes" );
	pBuffer += sprintf( pBuffer, "    isActive     = %s\n", ( pChannel->isActive == ASIOFalse )?"No":"Yes" );
	pBuffer += sprintf( pBuffer, "    channelGroup = %d\n", pChannel->channelGroup );
	switch ( pChannel->type )
	{
	case ASIOSTInt32LSB24 :
		pBuffer += sprintf( pBuffer, "    type         = ASIOSTInt32LSB24\n" );
		break;
	case ASIOSTInt32LSB :
		pBuffer += sprintf( pBuffer, "    type         = ASIOSTInt32LSB\n" );
		break;
	case ASIOSTInt32MSB24 :
		pBuffer += sprintf( pBuffer, "    type         = ASIOSTInt32MSB24\n" );
		break;
	default:
		pBuffer += sprintf( pBuffer, "    type         = Unknown (%d)\n", pChannel->type );
		break;
	}
	pBuffer += sprintf( pBuffer, "    name         = %s\n", pChannel->name );

	return;
}

#endif //ASIO_LOGGER