#include "main.h"
#include "gl_asio.h"
#include "..\..\Kernel\Source\driver_ioctl_data.h"


BOOL glAsio_c::Kernel_DATA_EngineVersion()
{
#ifdef ASIO_LOGGER
	Procedure_Start_Local();
#endif

//Check DATA engine version.
	unsigned long	bytesReturned;
	unsigned long	outputData = 0;

	BOOL	Status = DeviceIoControl(
	  m_hDeviceIoControl,
	  IOCTL_DATA_GET_ENGINE_VERSION,
	  NULL,
	  0,
	  &outputData,
	  sizeof( outputData ),
	  &bytesReturned,
	  NULL
	);
	if ( Status == TRUE )
	{
	#ifdef ASIO_LOGGER
		pLogger->Log( "  IOCTL_DATA_ENGINE_VERSION: %d\n", outputData );
	#endif

	//Test deviceIoControl version.
		if ( outputData != IOCTL_DATA_ENGINE_VERSION )
		{
			strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "ASIO(DATA) driver's version is not the same as driver's one. Get a newer version of the driver." );
			Status = FALSE;
		}
	}
	else
	{
		strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Failed to get IO engine version. Internal error... please contact author about this." );
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, Status );
#endif
	return Status;
}

BOOL glAsio_c::Kernel_DATA_CardsInfo()
{
#ifdef ASIO_LOGGER
	Procedure_Start_Local();
#endif

//Read data from driver.
	unsigned long	bytesReturned;

	BOOL	Status = DeviceIoControl(
	  m_hDeviceIoControl,
	  IOCTL_DATA_GET_CARDS_INFORMATION,
	  NULL,
	  0,
	  &m_Cards,
	  sizeof( m_Cards ),
	  &bytesReturned,
	  NULL
	);
	if ( Status == FALSE )
	{
		strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Failed to get cards information from driver." );
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, Status );
#endif
	return Status;
}

BOOL glAsio_c::Kernel_DATA_BufferSize()
{
#ifdef ASIO_LOGGER
	Procedure_Start_Local();
#endif

//Read data from driver.
	unsigned long	bytesReturned;

	m_BufferSize = 0;	//get buffer size

	BOOL	Status = DeviceIoControl(
	  m_hDeviceIoControl,
	  IOCTL_DATA_GET_SET_BUFFER_SIZE,
	  &m_BufferSize,
	  sizeof( m_BufferSize ),
	  &m_BufferSize,
	  sizeof( m_BufferSize ),
	  &bytesReturned,
	  NULL
	);
	if ( Status == FALSE )
	{
		strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Failed to get buffer size from driver." );
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, Status );
#endif
	return Status;
}

BOOL glAsio_c::Kernel_DATA_InputMonitoring( ASIOInputMonitor* pMonitoring )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Local();
#endif

	BOOL	Status = TRUE;

//Validate.
	if ( m_pChannels_INs == NULL )
	{
		strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Only input channels can be monitored. This is ASIO application's bug." );
		Status = FALSE;
	}
	if ( SUCCESS( Status ) )
	{
		if ( pMonitoring->input >= (long)( 2*m_nChannels_INs ) )		//mono... stereo
		{
			strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Channel index out of limits." );
			Status = FALSE;
		}
	}

//Get memory for ioctl buffer.
	ioctl_channelProperty_header_s*		pHeader = NULL;
	unsigned long						size;

	if ( SUCCESS( Status ) )
	{
		size = sizeof(ioctl_channelProperty_header_s) + 2*sizeof(ioctl_channelProperty_s);

	//Allocate memory.
		pHeader = (ioctl_channelProperty_header_s*)new char[size];
		if ( pHeader == NULL )
		{
			strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Out of memory." );
			Status = FALSE;
		}
	}

//Send monitoring requests.
//
//Notice:
//Even if we will have to send request for all channels we will send it one-by-one... it will be a little slower, but it will save us some problems.
	if ( SUCCESS( Status ) )
	{
	//Set header.
		pHeader->m_Request = PROPERTY_SET_PROPERTY;
		pHeader->m_RequestBy = PROPERTY_REQUEST_BY_ASIO;
		//pHeader->m_SerialNumber = pCard->m_SerialNumber;
		pHeader->m_nPropertyDefinitions = 2;

	//Send request.
		ioctl_channelProperty_s*	pProperty = (ioctl_channelProperty_s*)( pHeader + 1 );

		if ( pMonitoring->input == -1 )
		{
		//Send all.
			for ( unsigned long i = 0; i < m_nChannels_INs; i++ )
			{
			//Get channel.
				channelInfo_s*		pChannel = m_pChannels_INs + i;

			//Set header.
				pHeader->m_SerialNumber = (unsigned char)m_Cards[pChannel->m_SerialNumber_card].m_SerialNumber;

			//Set property first...
				( pProperty + 0 )->m_SerialNumber = (unsigned char)( 2*pChannel->m_SerialNumber_channel );
				( pProperty + 0 )->m_Property = PROPERTY_TYPE_MONITORING;

				if ( pMonitoring->state == ASIOTrue )
				{
				//Enable monitoring.
					( pProperty + 0 )->m_Position = PROPERTY_MONITORING_ENABLE;
				}
				else
				{
				//Disable monitoring.
					( pProperty + 0 )->m_Position = PROPERTY_MONITORING_DISABLE;
				}

			//Set property second...
				( pProperty + 1 )->m_SerialNumber = (unsigned char)( 2*pChannel->m_SerialNumber_channel + 1 );
				( pProperty + 1 )->m_Property = PROPERTY_TYPE_MONITORING;

				if ( pMonitoring->state == ASIOTrue )
				{
				//Enable monitoring.
					( pProperty + 1 )->m_Position = PROPERTY_MONITORING_ENABLE;
				}
				else
				{
				//Disable monitoring.
					( pProperty + 1 )->m_Position = PROPERTY_MONITORING_DISABLE;
				}

			//Set channel properties.
				unsigned long		bytesReturned = 0;

				Status = DeviceIoControl(
				  m_hDeviceIoControl,
				  IOCTL_DATA_GET_SET_CHANNEL_PROPERTIES,
				  pHeader,
				  size,
				  pHeader,
				  size,
				  &bytesReturned,
				  NULL
				);
				if ( Status == FALSE )
				{
					strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Failed to send monitoring request to kernel driver." );
					break;
				}
			}
		}
		else
		{
		//Send request for one.
		//
		//Get channel.
			channelInfo_s*		pChannel = m_pChannels_INs + ( pMonitoring->input/2 );

		//Set header.
			pHeader->m_SerialNumber = (unsigned char)m_Cards[pChannel->m_SerialNumber_card].m_SerialNumber;

		//Set property first...
			( pProperty + 0 )->m_SerialNumber = (unsigned char) ( 2*pChannel->m_SerialNumber_channel + ( pMonitoring->input & 0x00000001 ) );
			( pProperty + 0 )->m_Property = PROPERTY_TYPE_MONITORING;

			if ( pMonitoring->state == ASIOTrue )
			{
			//Enable monitoring.
				( pProperty + 0 )->m_Position = PROPERTY_MONITORING_ENABLE;
			}
			else
			{
			//Disable monitoring.
				( pProperty + 0 )->m_Position = PROPERTY_MONITORING_DISABLE;
			}

		//Set property second...
			( pProperty + 1 )->m_Property = PROPERTY_TYPE_UNDEFINED;		//ignore second structure

		//Set channel properties.
			unsigned long		bytesReturned = 0;

			Status = DeviceIoControl(
			  m_hDeviceIoControl,
			  IOCTL_DATA_GET_SET_CHANNEL_PROPERTIES,
			  pHeader,
			  size,
			  pHeader,
			  size,
			  &bytesReturned,
			  NULL
			);
			if ( Status == FALSE )
			{
				strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Failed to send monitoring request to kernel driver." );
			}
		}
	}

//Free resources.
	if ( pHeader != NULL )
	{
		delete[] pHeader;
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, Status );
#endif
	return Status;
}

BOOL glAsio_c::Kernel_DATA_SampleRate_Get( unsigned long* pSampleRate )
{
	BOOL	Status = TRUE;

//Do we already have sampleRate?
	if ( m_SampleRate > 0 )
	{
	//Yes... just return it's value.
		*pSampleRate = m_SampleRate;
	}
	else
	{
	#ifdef ASIO_LOGGER
		Procedure_Start_Local();
	#endif

	//Not yet... get it from driver.
	//
	//Get master card.
		ioctl_cardInfo_s*	pCard_master = Kernel_GetMasterCard();
		if ( pCard_master == NULL )
		{
			Status = FALSE;
		}

	//Get master card sample rate.
		if ( SUCCESS( Status ) )
		{
		//Set data.
			ioctl_setSampleRate_s		data;

			data.m_SerialNumber = pCard_master->m_SerialNumber;
			data.m_SampleRate = 0;		//get request

		//Send command.
			unsigned long	bytesReturned;

			Status = DeviceIoControl(
			  m_hDeviceIoControl,
			  IOCTL_DATA_GET_SET_SAMPLE_RATE,
			  &data,
			  sizeof( data ),
			  &data,
			  sizeof( data ),
			  &bytesReturned,
			  NULL
			);
			if ( Status == TRUE )
			{
				m_SampleRate = data.m_SampleRate;
				*pSampleRate = data.m_SampleRate;
			}
		}

	//Return.
		if ( Status == FALSE )
		{
			strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Failed to get sample rate from driver." );
		}

	#ifdef ASIO_LOGGER
		Procedure_Exit( -1, Status );
	#endif
	}

	return Status;
}

BOOL glAsio_c::Kernel_DATA_SampleRate_Set( unsigned long sampleRate )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Local();
#endif

	BOOL	Status = TRUE;

//Is new sample rate different then current one?
	if ( m_SampleRate != sampleRate )
	{
	//Yes it's different... send new sample rate to driver.
	//
	//Get master card.
		ioctl_cardInfo_s*	pCard_master = Kernel_GetMasterCard();
		if ( pCard_master == NULL )
		{
			Status = FALSE;
		}

	//Get master card sample rate.
		if ( SUCCESS( Status ) )
		{
		//Set data.
			ioctl_setSampleRate_s		data;

			data.m_SerialNumber = pCard_master->m_SerialNumber;
			data.m_SampleRate = sampleRate;			//set request

		//Send command.
			unsigned long	bytesReturned;

			Status = DeviceIoControl(
			  m_hDeviceIoControl,
			  IOCTL_DATA_GET_SET_SAMPLE_RATE,
			  &data,
			  sizeof( data ),
			  &data,
			  sizeof( data ),
			  &bytesReturned,
			  NULL
			);
			if ( Status == TRUE )
			{
				if ( data.m_SampleRate == sampleRate )
				{
				//New sample rate was succesfully set.
					m_SampleRate = sampleRate;
				}
				else
				{
				//Requested sample rate wasn't set.
					Status = FALSE;
				}
			}
		}

	//Return.
		if ( Status == FALSE )
		{
			strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Failed to set requested sample rate... is channel opened by another application?" );
		}
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, Status );
#endif
	return Status;
}

ioctl_cardInfo_s* glAsio_c::Kernel_GetMasterCard()
{
#ifdef ASIO_LOGGER
	Procedure_Start_Local();
#endif

//Get master card.
	ioctl_cardInfo_s*		pCard_master = NULL;

	for ( int i = 0; i < ELEMENTS_IN_ARRAY( m_Cards ); i++ )
	{
		if ( m_Cards[i].m_Type != ioctl_cardInfo_s::CARD_TYPE_NOT_SET )
		{
		//Is current card a master card?
			if (
			  ( m_Cards[i].m_ClockSource == ioctl_cardInfo_s::CLOCK_SOURCE_CARD )
			  &&
			  ( m_Cards[i].m_SerialNumber_masterCard == 0 )
			)
			{
			//Yes it's a master card.
				pCard_master = &m_Cards[i];
				break;
			}
		}
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, ( pCard_master != NULL ) ? TRUE : FALSE );
#endif
	return pCard_master;
}



