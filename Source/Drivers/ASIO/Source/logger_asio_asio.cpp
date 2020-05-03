#include "main.h"
#include "logger.h"
#include "gl_asio.h"

#ifdef ASIO_LOGGER
#pragma warning( disable : 4996 ) 

void logger_c::Logger_Future( IN char* pBuffer, IN void** ppVariables )
{
//Get selector.
	long		selector = *(long*)ppVariables[0];

	switch ( selector )
	{
	case kAsioEnableTimeCodeRead:
		pBuffer += sprintf( pBuffer, "  kAsioEnableTimeCodeRead:\n" );
		break;
	case kAsioDisableTimeCodeRead:
		pBuffer += sprintf( pBuffer, "  kAsioDisableTimeCodeRead:\n" );
		break;
	case kAsioSetInputMonitor:
	{
		ASIOInputMonitor*		pOpt = (ASIOInputMonitor*)ppVariables[1];

		pBuffer += sprintf( pBuffer, "  kAsioSetInputMonitor:\n" );
		pBuffer += sprintf( pBuffer, "    input:   %d\n", pOpt->input );
		pBuffer += sprintf( pBuffer, "    output:  %d\n", pOpt->output );
		pBuffer += sprintf( pBuffer, "    gain:    %d\n", pOpt->gain );
		pBuffer += sprintf( pBuffer, "    state:   %s\n", ( pOpt->state == ASIOFalse ) ? "ASIOFalse" : "ASIOTrue" );
		pBuffer += sprintf( pBuffer, "    pan:     %d\n", pOpt->pan );
		break;
	}
	case kAsioTransport:
		pBuffer += sprintf( pBuffer, "  kAsioTransport:\n" );
		break;
	case kAsioSetInputGain:
		pBuffer += sprintf( pBuffer, "  kAsioSetInputGain:\n" );
		break;
	case kAsioGetInputMeter:
		pBuffer += sprintf( pBuffer, "  kAsioGetInputMeter:\n" );
		break;
	case kAsioSetOutputGain:
		pBuffer += sprintf( pBuffer, "  kAsioSetOutputGain:\n" );
		break;
	case kAsioGetOutputMeter:
		pBuffer += sprintf( pBuffer, "  kAsioGetOutputMeter:\n" );
		break;
	case kAsioCanInputMonitor:
		pBuffer += sprintf( pBuffer, "  kAsioCanInputMonitor:\n" );
		break;
	case kAsioCanTimeInfo:
		pBuffer += sprintf( pBuffer, "  kAsioCanTimeInfo:\n" );
		break;
	case kAsioCanTimeCode:
		pBuffer += sprintf( pBuffer, "  kAsioCanTimeCode:\n" );
		break;
	case kAsioCanTransport:
		pBuffer += sprintf( pBuffer, "  kAsioCanTransport:\n" );
		break;
	case kAsioCanInputGain:
		pBuffer += sprintf( pBuffer, "  kAsioCanInputGain:\n" );
		break;
	case kAsioCanInputMeter:
		pBuffer += sprintf( pBuffer, "  kAsioCanInputMeter:\n" );
		break;
	case kAsioCanOutputGain:
		pBuffer += sprintf( pBuffer, "  kAsioCanOutputGain:\n" );
		break;
	case kAsioCanOutputMeter:
		pBuffer += sprintf( pBuffer, "  kAsioCanOutputMeter:\n" );
		break;
	default:
		pBuffer += sprintf( pBuffer, "  unknown: %d\n", selector );
		break;
	}
}

#endif //ASIO_LOGGER
