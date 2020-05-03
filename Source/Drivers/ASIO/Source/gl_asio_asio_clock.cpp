#include "main.h"
#include "gl_asio.h"
#include "registry.h"
#include "..\..\Kernel\Source\driver_ioctl_asio.h"


ASIOError glAsio_c::getClockSources( ASIOClockSource* pClocks, long* pNumSources )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Global();
#endif

//Driver supports only internal clock source.
	pClocks->index = 0;
	pClocks->associatedChannel = -1;
	pClocks->associatedGroup = -1;
	pClocks->isCurrentSource = ASIOTrue;
	strcpy_s( pClocks->name, sizeof( pClocks->name ), "Internal" );

	*pNumSources = 1;

#ifdef ASIO_LOGGER
	Procedure_Exit( ASE_OK, -1 );
#endif
	return ASE_OK;
}

ASIOError glAsio_c::setClockSource( long index )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Global();
#endif

	ASIOError	AStatus;

	if ( index == 0 )
	{
		//WALDEMAR
		//asioTime.timeInfo.flags |= kClockSourceChanged;
		AStatus = ASE_OK;
	}
	else
	{
		AStatus = ASE_NotPresent;
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( AStatus, -1 );
#endif
	return AStatus;
}

ASIOError glAsio_c::getLatencies( long* pInputLatency, long* pOutputLatency )
{
//Input latency: always 1 buffer size
//Output latency:
//It will usually be either one block, if the host writes directly to a DMA buffer
//or two or more blocks if the buffer is ’latched’ by the driver.
//As an example, on ASIOStart(),
//the host will have filled the play buffer at index 1 already;
//when it gets the callback (with the parameter index == 0), this tells it to read
//from the input buffer 0, and start to fill the play buffer 0 (assuming that now
//play buffer 1 is already sounding).
//In this case, the output latency is one block.
//If the driver decides to copy buffer 1 at that time, and pass it to the
//hardware at the next slot (which is most commonly done, but should be
//avoided), the output latency becomes two blocks instead, resulting in a total
//i/o latency of at least 3 blocks. As memory access is the main bottleneck in
//native DSP processing, and to achieve lower latency, it is highly
//recommended to try to avoid copying (this is also why the driver is the owner
//of the buffers).
//To summarize, the minimum i/o latency can be achieved if the
//input buffer is processed by the host into the output buffer which will
//physically start to sound on the next time slice.
//Also note that the host expects the bufferSwitch() callback to be accessed for
//each time slice in order to retain sync, possibly recursively;
//if it fails to process a block in time, it will suspend its operation for
//some time in order to recover.
//
//On the card(normal):
//
// ---------------------------------
// |   |IN |   |OUT|   |   |   |   |
// ---------------------------------
//         |
//         |
//    current offset
//
//Now as seen -> when offset comes to the start of the middle buffer
//card sends interrupt which signals that the previous buffer
//was filled and that we should fill the next(not current) buffer with
//output samples.
//
//This would give us the output latency of 2 blocks.
//
//Is there a way to lower output latency to 1 block?
//Yes there is on expense of some CPU cycles.
//
//When ASIO application finishes it's processing the GLAsio::outputReady
//is called. Now at this time we can signal the lower driver that it
//should copy it's output buffers to the card.
//
//Now let us check how the buffers offset look:
// ---------------------------------
// |   |IN |OUT|   |   |   |   |   |
// ---------------------------------
//        |
//        |
//    current offset
//
//As seen just before the card would send the interrupt we would fill the buffers
//with the output buffers. And so drop the output latency to 1 buffer.
//
//Latencies:
//496: 31/fs = ADC and DAC
//824: 32/fs = ADC
//824: 25/fs = DAC
//
#ifdef ASIO_LOGGER
	Procedure_Start_Global();
#endif

	*pInputLatency = m_BufferSize + m_Latency_IN;
	*pOutputLatency = m_BufferSize + m_Latency_OUT;

#ifdef ASIO_LOGGER
	pLogger->Log( "  inputLatency: 0x%X, outputlatency: 0x%X\n", *pInputLatency, *pOutputLatency );
	Procedure_Exit( ASE_OK, -1 );
#endif
	return ASE_OK;
}

ASIOError glAsio_c::getSamplePosition( ASIOSamples* pSPosition, ASIOTimeStamp* pTStamp )
{
//#ifdef ASIO_PRINT
//	char		buffer[128];
//
//	sprintf( buffer, "glAsio_c::getSamplePosition\n" );
//	OutputDebugString( buffer );
//#endif

	ioctl_asio_switch*		pSwitch = (ioctl_asio_switch*)m_pMemory_global;
	if ( pSwitch != NULL )
	{
	//Get systemTime.
		sampleClock_s		timeStamp;
		timeStamp.clock = (ULONGLONG)timeGetTime() * (ULONGLONG)1000000;		//in 1ns

	//Set clock and timestamp.
		//Time stamp.
		pTStamp->lo = timeStamp.dwords.dword_lo;
		pTStamp->hi = timeStamp.dwords.dword_hi;

		//Sample position.
		pSPosition->lo = pSwitch->m_Clock_samples.dwords.dword_lo;
		pSPosition->hi = pSwitch->m_Clock_samples.dwords.dword_hi;

	//#ifdef ASIO_PRINT
	//	sprintf( buffer, "getSamplePosition\n" );
	//	OutputDebugString( buffer );
	//	sprintf( buffer, "  TimeStamp: 0x%I64X ( hi: 0x%X, lo: 0x%X )\n", timeStamp.clock, pTStamp->hi, pTStamp->lo );
	//	OutputDebugString( buffer );
	//	sprintf( buffer, "  timeInfo.samplePosition.lo: 0x%I64X\n", pSwitch->m_Clock_samples.clock );
	//	OutputDebugString( buffer );
	//#endif
		return ASE_OK;
	}
	else
	{
		return ASE_InvalidMode;
	}
}

void glAsio_c::SetLatencies( registry_data_s* pData )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Local();
#endif

//Get card & channel types.
	unsigned char		delayTypes_IN = 0;
	unsigned char		delayTypes_OUT = 0;

	for ( int i = 0; i < ELEMENTS_IN_ARRAY( pData->m_Channels_Opened ); i++ )
	{
	//Every card and channel type has it's latency.
		switch ( ioctl_cardInfo_s::CARD_TYPE( pData->m_Channels_Opened[i].m_Type ) )
		{
		case ioctl_cardInfo_s::CARD_TYPE_424:
		case ioctl_cardInfo_s::CARD_TYPE_496:
		{
			if ( ( pData->m_Channels_Opened[i].m_Channels_IN & 0x03 ) != 0 )
			{
				delayTypes_IN |= 0x01;
			}
			if ( ( pData->m_Channels_Opened[i].m_Channels_OUT & 0x03 ) != 0 )
			{
				delayTypes_OUT |= 0x01;
			}
			break;
		}
		case ioctl_cardInfo_s::CARD_TYPE_824:
		{
			if ( ( pData->m_Channels_Opened[i].m_Channels_IN & 0x07 ) != 0 )
			{
				delayTypes_IN |= 0x02;
			}
			if ( ( pData->m_Channels_Opened[i].m_Channels_OUT & 0x07 ) != 0 )
			{
				delayTypes_OUT |= 0x02;
			}
			break;
		}
		case ioctl_cardInfo_s::CARD_TYPE_NOT_SET:
		default:
			break;
		}
	}

//Set cards latencies.
	switch ( delayTypes_IN )
	{
	case 1 :
	//Just 4xx cards -> ADC delay = 31 samples(filter delay).
		m_Latency_IN = 31;
		break;
	case 2 :
	//Just 824 cards -> ADC delay = 32 samples(filter delay).
		m_Latency_IN = 32;
		break;
	case 3 :
	//4xx and 824 cards -> ADC delay = 31 samples.
		m_Latency_IN = 31;
		break;
	default:
		break;
	}
	switch ( delayTypes_OUT )
	{
	case 1 :
	//Just 4xx cards -> DAC delay = 30 samples(filter delay).
		m_Latency_OUT = 32;
		break;
	case 2 :
	//Just 824 cards -> DAC delay = 25 samples(filter delay) +?? (it looks the data sheets aren't correct).
		m_Latency_OUT = 30;
		break;
	case 3 :
	//4xx and 824 cards -> DAC delay = ( 32 + 30 )/2 = 31 samples.
		m_Latency_OUT = 31;
		break;
	default:
		break;
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, TRUE );
#endif
}
