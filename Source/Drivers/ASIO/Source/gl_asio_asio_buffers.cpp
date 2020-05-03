#include "main.h"
#include "gl_asio.h"

ASIOError glAsio_c::getBufferSize( long* pMinSize, long* pMaxSize, long* pPreferredSize, long* pGranularity )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Global();
#endif

	*pMinSize = m_BufferSize;
	*pMaxSize = m_BufferSize;
	*pPreferredSize = m_BufferSize;
	*pGranularity = 0;

#ifdef ASIO_LOGGER
	pLogger->Log( "  minSize: 0x%X\n  maxSize: 0x%X\n  preferredSize: 0x%X\n  granularity: 0x%X\n", *pMinSize, *pMaxSize, *pPreferredSize, *pGranularity );
	Procedure_Exit( ASE_OK, -1 );
#endif
	return ASE_OK;
}

ASIOError glAsio_c::createBuffers( ASIOBufferInfo* pBufferInfos, long numChannels, long bufferSize, ASIOCallbacks* pCallbacks )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Global();
	pLogger->Log( "  numChannels: %d, bufferSize: 0x%X\n", numChannels, bufferSize );
#endif

	ASIOError	Status = ASE_OK;
	
//Validate...
	if ( (unsigned long)numChannels > ( 2*( m_nChannels_INs + m_nChannels_OUTs ) ) )
	{
		strcpy_s( m_ErrorMessage, sizeof( m_ErrorMessage ), "CreateBuffers number of channels out of range." );
		Status = ASE_InvalidParameter;
	}
	if ( SUCCESS_ASIO( Status ) )
	{
		if ( bufferSize != m_BufferSize )
		{
			strcpy_s( m_ErrorMessage, sizeof( m_ErrorMessage ), "CreateBuffers invalid bufferSize." );
			Status = ASE_InvalidParameter;
		}
	}

//Set start structure.
	if ( SUCCESS_ASIO( Status ) )
	{
		m_pStart = (ioctl_asio_start_header_s*)new char[ sizeof(ioctl_asio_start_header_s) ];
		if ( m_pStart == NULL )
		{
			strcpy_s( m_ErrorMessage, sizeof( m_ErrorMessage ), "Out of memory error." );
			Status = ASE_InvalidMode;
		}
		ZeroMemory( m_pStart, sizeof ( ioctl_asio_start_header_s ) );
	}
	if ( SUCCESS_ASIO( Status ) )
	{
	//Set header.
		m_pStart->m_hAsio = m_hAsio;

	//Set channels.
		for ( long i = 0; i < numChannels; i++ )
		{
		//Get buffer info.
			ASIOBufferInfo*		pBufferInfo = &pBufferInfos[i];
		#ifdef ASIO_LOGGER
			pLogger->Log( "channel: %d, isInput = %d, channelNum = %d\n", i, pBufferInfo->isInput, pBufferInfo->channelNum );
		#endif

		//Get channel.
			channelInfo_s*		pChannel;

			if ( pBufferInfo->isInput == ASIOTrue )
			{
			//Input.
				//Validate...
				if ( ( (unsigned long)pBufferInfo->channelNum/2 ) >= m_nChannels_INs  )
				{
					Status = ASE_InvalidParameter;
					break;
				}

				//Set...
				pChannel = &m_pChannels_INs[pBufferInfo->channelNum/2];		//mono to stereo
			}
			else
			{
			//Output.
				//Validate...
				if ( ( (unsigned long)pBufferInfo->channelNum/2 ) >= m_nChannels_OUTs  )
				{
					Status = ASE_InvalidParameter;
					break;
				}

				//Set...
				pChannel = &m_pChannels_OUTs[pBufferInfo->channelNum/2];		//mono to stereo
			}

		//Set bufferInfo structure.
			//Is it left or right channel?
			if ( ( pBufferInfo->channelNum & 0x00000001 ) == 0 )
			{
			//Left.
				pBufferInfo->buffers[0] = pChannel->m_pBuffer_A_left;
				pBufferInfo->buffers[1] = pChannel->m_pBuffer_B_left;

			//Update channel.
				pChannel->m_ChannelInfo_left.isActive = ASIOTrue;

			#ifdef ASIO_LOGGER
				unsigned long	leftRight = 0;
				pLogger->Log( logger_c::LS_CreateBuffers, pChannel, &leftRight );
			#endif
			}
			else
			{
			//Right.
				pBufferInfo->buffers[0] = pChannel->m_pBuffer_A_right;
				pBufferInfo->buffers[1] = pChannel->m_pBuffer_B_right;

			//Update channel.
				pChannel->m_ChannelInfo_right.isActive = ASIOTrue;

			#ifdef ASIO_LOGGER
				unsigned long	leftRight = 1;
				pLogger->Log( logger_c::LS_CreateBuffers, pChannel, &leftRight );
			#endif
			}
		
		//Set start structure.
			unsigned char		bit = 0x01 << ( 2*pChannel->m_SerialNumber_channel + ( pBufferInfo->channelNum & 0x00000001 ) );

			if ( pBufferInfo->isInput == ASIOTrue )
			{
			//Input.
				m_pStart->m_Channels_Started[pChannel->m_SerialNumber_card].m_Channels_IN |= bit;
			}
			else
			{
			//Output.
				m_pStart->m_Channels_Started[pChannel->m_SerialNumber_card].m_Channels_OUT |= bit;
			}
		}

		//Set cards parameters.
		for ( int i = 0; i < ELEMENTS_IN_ARRAY( m_pStart->m_Channels_Started ); i++ )
		{
			if (
			  ( m_pStart->m_Channels_Started[i].m_Channels_IN != 0 )
			  ||
			  ( m_pStart->m_Channels_Started[i].m_Channels_OUT != 0 )
			)
			{
				m_pStart->m_Channels_Started[i].m_Type = (unsigned char)m_Cards[i].m_Type;
				m_pStart->m_Channels_Started[i].m_SerialNumber = (unsigned char)m_Cards[i].m_SerialNumber;
			}
		}
	}

//Set callback.
	m_pHostCallbacks = pCallbacks;

//Does application(driver caller) supports timeInfo and timeCode?
	if ( SUCCESS_ASIO( Status ) )
	{
		if ( TRUE == pCallbacks->asioMessage( kAsioSupportsTimeInfo, 0, 0, 0 ) )
		{
		//Time info.
			m_Switch_type = SWITCH_TYPE_ASIO_2;

			m_AsioTime.timeInfo.speed = 1.0;

			m_AsioTime.timeInfo.systemTime.lo = 0;
			m_AsioTime.timeInfo.systemTime.hi = 0;

			m_AsioTime.timeInfo.samplePosition.lo = 0;
			m_AsioTime.timeInfo.samplePosition.hi = 0;

			Status = getSampleRate( &m_AsioTime.timeInfo.sampleRate );

			m_AsioTime.timeInfo.flags = kSpeedValid | kSystemTimeValid | kSamplePositionValid | kSampleRateValid;

			//if ( TRUE == pCallbacks->asioMessage( kAsioSupportsTimeCode, 0, 0, 0 ) )
			//{
			////Time code.
			//	asioTime.timeCode.speed = 1.0;
			//	asioTime.timeCode.timeCodeSamples.lo = 0;
			//	asioTime.timeCode.timeCodeSamples.hi = 0;
			//	asioTime.timeCode.flags = kTcValid | kTcRunning;
			//}

		#ifdef ASIO_LOGGER
			pLogger->Log( "  ASIO version 2.0\n" );
		#endif
		}
		else
		{
			m_Switch_type = SWITCH_TYPE_ASIO_1;

		#ifdef ASIO_LOGGER
			pLogger->Log( "  ASIO version 1.0\n" );
		#endif
		}
	}

#ifdef ASIO_LOGGER
	pLogger->Log( "\n  Return values:\n" );
	for ( long i =0; i < numChannels; i++ )
	{
		ASIOBufferInfo*		pChannel = &pBufferInfos[i];

		pLogger->Log(
		  "  channelNum: %d, isInput: %s, buffers: %p, %p\n",
		  pChannel->channelNum,
		  ( pChannel->isInput == ASIOFalse ) ? "NO" : "YES",
		  pChannel->buffers[0],
		  pChannel->buffers[1]
		);
	}
	Procedure_Exit( Status, -1 );
#endif
	return Status;
}

ASIOError glAsio_c::disposeBuffers( void )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Global();
#endif

	ASIOError	AStatus;

//Deactivate channels.
	for ( unsigned long i = 0; i < m_nChannels_INs; i ++)
	{
		m_pChannels_INs[i].m_ChannelInfo_left.isActive = ASIOFalse;
		m_pChannels_INs[i].m_ChannelInfo_right.isActive = ASIOFalse;
	}
	for ( unsigned long i = 0; i < m_nChannels_OUTs; i ++)
	{
		m_pChannels_OUTs[i].m_ChannelInfo_left.isActive = ASIOFalse;
		m_pChannels_OUTs[i].m_ChannelInfo_right.isActive = ASIOFalse;
	}

//Free resources.
	if ( m_pStart != NULL )
	{
		delete[] m_pStart;
		m_pStart = NULL;
		AStatus = ASE_OK;
	}
	else
	{
		AStatus = ASE_InvalidMode;
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( AStatus, -1 );
#endif
	return AStatus;
}
