#include "main.h"
#include "gl_asio.h"
#include "registry.h"
#include "..\..\Kernel\Source\driver_ioctl_asio.h"


ASIOError glAsio_c::getChannelInfo( ASIOChannelInfo* pInfo )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Global();
	pLogger->Log( "  Before:\n" );
	pLogger->Log( logger_c::LS_GetChannelInfo, pInfo );
#endif

	ASIOError		AStatus = ASE_OK;

//Validate...
	if (
	  ( pInfo->channel < 0 )
	  ||
	  (
	    ( pInfo->isInput == ASIOTrue ) ? ( (unsigned long)( pInfo->channel/2 ) >= m_nChannels_INs ) : ( (unsigned long)( pInfo->channel/2 ) >= m_nChannels_OUTs )
	  )
	)
	{
		strcpy_s( m_ErrorMessage, sizeof( m_ErrorMessage ), "GetChannelInfo: Requested channel index out of limits." );
		AStatus = ASE_InvalidParameter;
	}

//Return requested channel data.
	if ( AStatus == ASE_OK )
	{
	//Get stero channel.
		channelInfo_s*		pChannel;

		if ( pInfo->isInput == ASIOTrue )
		{
		//Input.
			pChannel = m_pChannels_INs + pInfo->channel/2;
		}
		else
		{
		//Output.
			pChannel = m_pChannels_OUTs + pInfo->channel/2;
		}

	//Get left/right channel info...
		ASIOChannelInfo*		pInfo_temp;

		if ( ( pInfo->channel & 0x00000001 ) == 0 )
		{
		//Left channel...
			pInfo_temp = &pChannel->m_ChannelInfo_left;
		}
		else
		{
		//Right channel...
			pInfo_temp = &pChannel->m_ChannelInfo_right;
		}

	//Copy channel data.
		CopyMemory( pInfo, pInfo_temp, sizeof( ASIOChannelInfo ) );
	}

#ifdef ASIO_LOGGER
	pLogger->Log( "  After:\n" );
	pLogger->Log( logger_c::LS_GetChannelInfo, pInfo );
	Procedure_Exit( AStatus, -1 );
#endif
	return AStatus;
}

ASIOError glAsio_c::getChannels( long* pNumInputChannels, long* pNumOutputChannels )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Global();
#endif

//Number of mono channels.
	*pNumInputChannels = 2*m_nChannels_INs;
	*pNumOutputChannels = 2*m_nChannels_OUTs;

#ifdef ASIO_LOGGER
	pLogger->Log( "  numInputChannels: %d\n  numOutputChannels: %d\n", *pNumInputChannels, *pNumOutputChannels );
	Procedure_Exit( ASE_OK, -1 );
#endif
	return ASE_OK;
}

BOOL glAsio_c::SetChannels( registry_data_s* pData )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Local();
#endif

	BOOL		Status = TRUE;

//Validate...
	if ( m_pChannels_INs != NULL )
	{
		Status = FALSE;
	}
	if ( SUCCESS( Status ) )
	{
		if ( m_pChannels_OUTs != NULL )
		{
			Status = FALSE;
		}
	}

//Allocate memory...
	if ( SUCCESS( Status ) )
	{
		if ( m_nChannels_INs > 0 )
		{
			m_pChannels_INs = new channelInfo_s[m_nChannels_INs];
			if ( m_pChannels_INs == NULL )
			{
				Status = FALSE;
			}
		}
	}
	if ( SUCCESS( Status ) )
	{
		if ( m_nChannels_OUTs > 0 )
		{
			m_pChannels_OUTs = new channelInfo_s[m_nChannels_OUTs];
			if ( m_pChannels_OUTs == NULL )
			{
				Status = FALSE;
			}
		}
	}

//Get sample type.
	ASIOSampleType		sampleType;

	if ( SUCCESS( Status ) )
	{
		switch ( pData->m_SampleType )
		{
		default:
		case SAMPLETYPE_32BIT_LSB24:
			sampleType = ASIOSTInt32LSB24;
			break;
		case SAMPLETYPE_32BIT_LSB:
			sampleType = ASIOSTInt32LSB;
			break;
		case SAMPLETYPE_32BIT_MSB24:
			sampleType = ASIOSTInt32MSB24;
			break;
		}
	}

//Get buffers pointer.
	unsigned long*		pBuffer;

	if ( SUCCESS( Status ) )
	{
		pBuffer = (unsigned long*)( (ioctl_asio_switch*)m_pMemory_global + 1 );
	}

//Set input channels.
	if ( SUCCESS( Status ) )
	{
		unsigned long		nStereoChannel_IN_current = 0;

		for ( int i = 0; i < ELEMENTS_IN_ARRAY( m_Cards ); i++ )
		{
		//Is card defined?
			unsigned long		nStereoChannels;
			unsigned long		cardType;

			switch ( ioctl_cardInfo_s::CARD_TYPE( pData->m_Channels_Opened[i].m_Type ) )
			{
			case ioctl_cardInfo_s::CARD_TYPE_424:
			{
				cardType = 424;
				nStereoChannels = 2;
				break;
			}
			case ioctl_cardInfo_s::CARD_TYPE_496:
			{
				cardType = 496;
				nStereoChannels = 2;
				break;
			}
			case ioctl_cardInfo_s::CARD_TYPE_824:
			{
				cardType = 824;
				nStereoChannels = 4;
				break;
			}
			case ioctl_cardInfo_s::CARD_TYPE_NOT_SET:
			default:
				nStereoChannels = 0;
				break;
			}
			if ( nStereoChannels == 0 )
			{
				continue;
			}

		//Check which channels are opened.
			unsigned char		bit = 0x01;

			//Inputs...
			for ( unsigned long j = 0; j < nStereoChannels; j++ )
			{
				if ( ( pData->m_Channels_Opened[i].m_Channels_IN & bit ) != 0 )
				{
				//Got an open input channel...
					channelInfo_s*		pChannel = &m_pChannels_INs[nStereoChannel_IN_current];

				//Left channel.
					pChannel->m_SerialNumber_card = (unsigned char)i;
					pChannel->m_SerialNumber_channel = (unsigned short)j;

					pChannel->m_ChannelInfo_left.channel = (long)( 2*nStereoChannel_IN_current );		//from stereo to mono
					pChannel->m_ChannelInfo_left.isInput = ASIOTrue;
					pChannel->m_ChannelInfo_left.isActive = ASIOFalse;
					pChannel->m_ChannelInfo_left.channelGroup = 0;
					pChannel->m_ChannelInfo_left.type = sampleType;

					//name ex.: "WavePro496(1) ch. 1"
					sprintf_s(
					  pChannel->m_ChannelInfo_left.name,
					  sizeof( pChannel->m_ChannelInfo_left.name ),
					  "WavePro%d(%d) ch. %d",
					  cardType,
					  m_Cards[i].m_SerialNumber + 1,	//same ID as AVStream
					  2*j + 1							//same ID as AVStream (from stereo to mono)
					);

					pChannel->m_pBuffer_A_left = pBuffer; pBuffer += m_BufferSize;
					pChannel->m_pBuffer_B_left = pBuffer; pBuffer += m_BufferSize;

				//Right channel.
					pChannel->m_ChannelInfo_right.channel = (long)( 2*nStereoChannel_IN_current + 1 );		//from stereo to mono
					pChannel->m_ChannelInfo_right.isInput = ASIOTrue;
					pChannel->m_ChannelInfo_right.isActive = ASIOFalse;
					pChannel->m_ChannelInfo_right.channelGroup = 0;
					pChannel->m_ChannelInfo_right.type = sampleType;

					//name ex.: "WavePro496(1) ch. 2"
					sprintf_s(
					  pChannel->m_ChannelInfo_right.name,
					  sizeof( pChannel->m_ChannelInfo_left.name ),
					  "WavePro%d(%d) ch. %d",
					  cardType,
					  m_Cards[i].m_SerialNumber + 1,	//same ID as AVStream
					  2*j + 2							//same ID as AVStream (from stereo to mono)
					);

					pChannel->m_pBuffer_A_right = pBuffer; pBuffer += m_BufferSize;
					pChannel->m_pBuffer_B_right = pBuffer; pBuffer += m_BufferSize;

				//Next...
					++nStereoChannel_IN_current;
				}

			//Next...
				bit <<= 1;
			}
		}
	}

//Set output channels.
	if ( SUCCESS( Status ) )
	{
		unsigned long		nStereoChannel_OUT_current = 0;

		for ( int i = 0; i < ELEMENTS_IN_ARRAY( m_Cards ); i++ )
		{
		//Is card defined?
			unsigned long		nStereoChannels;
			unsigned long		cardType;

			switch ( ioctl_cardInfo_s::CARD_TYPE( pData->m_Channels_Opened[i].m_Type ) )
			{
			case ioctl_cardInfo_s::CARD_TYPE_424:
			{
				cardType = 424;
				nStereoChannels = 2;
				break;
			}
			case ioctl_cardInfo_s::CARD_TYPE_496:
			{
				cardType = 496;
				nStereoChannels = 2;
				break;
			}
			case ioctl_cardInfo_s::CARD_TYPE_824:
			{
				cardType = 824;
				nStereoChannels = 4;
				break;
			}
			case ioctl_cardInfo_s::CARD_TYPE_NOT_SET:
			default:
				nStereoChannels = 0;
				break;
			}
			if ( nStereoChannels == 0 )
			{
				continue;
			}

		//Check which channels are opened.
			unsigned char		bit = 0x01;

			//Outputs...
			for ( unsigned long j = 0; j < nStereoChannels; j++ )
			{
				if ( ( pData->m_Channels_Opened[i].m_Channels_OUT & bit ) != 0 )
				{
				//Got an open input channel...
					channelInfo_s*		pChannel = &m_pChannels_OUTs[nStereoChannel_OUT_current];

				//Left channel.
					pChannel->m_SerialNumber_card = (unsigned char)i;
					pChannel->m_SerialNumber_channel = (unsigned short)j;

					pChannel->m_ChannelInfo_left.channel = (long)( 2*nStereoChannel_OUT_current );		//from stereo to mono
					pChannel->m_ChannelInfo_left.isInput = ASIOFalse;
					pChannel->m_ChannelInfo_left.isActive = ASIOFalse;
					pChannel->m_ChannelInfo_left.channelGroup = 0;
					pChannel->m_ChannelInfo_left.type = sampleType;

					//name ex.: "WavePro496(1) ch. 1"
					sprintf_s(
					  pChannel->m_ChannelInfo_left.name,
					  sizeof( pChannel->m_ChannelInfo_left.name ),
					  "WavePro%d(%d) ch. %d",
					  cardType,
					  m_Cards[i].m_SerialNumber + 1,	//same ID as AVStream
					  2*j + 1							//same ID as AVStream (from stereo to mono)
					);

					pChannel->m_pBuffer_A_left = pBuffer; pBuffer += m_BufferSize;
					pChannel->m_pBuffer_B_left = pBuffer; pBuffer += m_BufferSize;

				//Right channel.
					pChannel->m_ChannelInfo_right.channel = (long)( 2*nStereoChannel_OUT_current + 1 );		//from stereo to mono
					pChannel->m_ChannelInfo_right.isInput = ASIOFalse;
					pChannel->m_ChannelInfo_right.isActive = ASIOFalse;
					pChannel->m_ChannelInfo_right.channelGroup = 0;
					pChannel->m_ChannelInfo_right.type = sampleType;

					//name ex.: "WavePro496(1) ch. 2"
					sprintf_s(
					  pChannel->m_ChannelInfo_right.name,
					  sizeof( pChannel->m_ChannelInfo_left.name ),
					  "WavePro%d(%d) ch. %d",
					  cardType,
					  m_Cards[i].m_SerialNumber + 1,	//same ID as AVStream
					  2*j + 2							//same ID as AVStream (from stereo to mono)
					);

					pChannel->m_pBuffer_A_right = pBuffer; pBuffer += m_BufferSize;
					pChannel->m_pBuffer_B_right = pBuffer; pBuffer += m_BufferSize;

				//Next...
					++nStereoChannel_OUT_current;
				}

			//Next...
				bit <<= 1;
			}
		}
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, Status );
#endif
	return Status;
}
