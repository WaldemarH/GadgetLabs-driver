#include "main.h"
#include "gl_asio.h"
#include "..\..\Kernel\Source\driver_ioctl_asio.h"
#include "..\..\Kernel\Source\driver_ioctl_data.h"
#include "registry.h"


BOOL glAsio_c::Kernel_ASIO_EngineVersion()
{
#ifdef ASIO_LOGGER
	Procedure_Start_Local();
#endif

//Check ASIO engine version.
	unsigned long	bytesReturned;
	unsigned long	outputData = 0;

	BOOL	Status = DeviceIoControl(
	  m_hDeviceIoControl,
	  IOCTL_ASIO_GET_ENGINE_VERSION,
	  NULL,
	  0,
	  &outputData,
	  sizeof( outputData ),
	  &bytesReturned,
	  NULL
	);
	if ( Status == TRUE )
	{
	//Test ASIO engine version.
	#ifdef ASIO_LOGGER
		pLogger->Log( "  IOCTL_ASIO_ENGINE_VERSION: %d\n", outputData );
	#endif

		if ( outputData != IOCTL_ASIO_ENGINE_VERSION )
		{
			strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "ASIO(ASIO) driver's version is not the same as driver's one. Get a newer version of the driver." );
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

BOOL glAsio_c::Kernel_ASIO_DriverHandle( DRIVER_HANDLE_TYPE type )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Local();
	pLogger->Log( "  Type: %s\n", ( type == DRIVER_HANDLE_TYPE_GET ) ? "DRIVER_HANDLE_TYPE_GET" : "DRIVER_HANDLE_TYPE_CLOSE" );
#endif

	BOOL	Status = TRUE;

//Get/Close ASIO handle?
	if ( type == DRIVER_HANDLE_TYPE_GET )
	{
	//Get...
		if ( m_hAsio != 0 )
		{
		//As it looks we already have ASIO handle.
			Status = FALSE;

		#ifdef ASIO_LOGGER
			pLogger->Log( "ASIO already opened.\n" );
		#endif
		}
	}
	else
	{
	//Close...
		if ( m_hAsio == 0 )
		{
		//We don't have anything to close.
			Status = FALSE;

		#ifdef ASIO_LOGGER
			pLogger->Log( "ASIO already closed.\n" );
		#endif
		}
	}

//Send command...
	if ( SUCCESS( Status ) )
	{
		unsigned long	bytesReturned;

		Status = DeviceIoControl(
		  m_hDeviceIoControl,
		  IOCTL_ASIO_GET_CLOSE_HANDLE,
		  &m_hAsio,
		  sizeof( m_hAsio ),
		  &m_hAsio,
		  sizeof( m_hAsio ),
		  &bytesReturned,
		  NULL
		);
		if ( Status == TRUE )
		{
		//Test for proper return.
			if ( type == DRIVER_HANDLE_TYPE_GET )
			{
			//Get...
				if ( m_hAsio == 0 )
				{
				//Something went wrong...
					strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Failed to get ASIO handle." );
					Status = FALSE;

				#ifdef ASIO_LOGGER
					pLogger->Log( "Failed to get ASIO handle.\n" );
				#endif
				}
			}
			else
			{
			//Close...
				if ( m_hAsio != 0 )
				{
				//Failed to close ASIO handle.
					strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Failed to close ASIO handle." );
					Status = FALSE;

				#ifdef ASIO_LOGGER
					pLogger->Log( "Failed to close ASIO handle.\n" );
				#endif
				}
			}
		}
		else
		{
			strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Failed to get ASIO handle." );

		#ifdef ASIO_LOGGER
			pLogger->Log( "Failed to get ASIO handle.\n" );
		#endif
		}
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, Status );
#endif
	return Status;
}

BOOL glAsio_c::Kernel_ASIO_Channels_Open( registry_data_s* pData )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Local();
#endif

//Set kernel struct data.
	ioctl_asio_channels_header_s	data_kernel;

	data_kernel.m_hAsio = m_hAsio;
	data_kernel.m_Request = ASIO_CHANNELS_OPEN;
	data_kernel.m_SampleType = (unsigned short)pData->m_SampleType;

	unsigned long					nCardsUsed = 0;
	for ( int i = 0; i < ELEMENTS_IN_ARRAY( data_kernel.m_Channels_Opened ); i++ )
	{
	//Only use those cards that have at least 1 channel opened.
		if (
		  ( pData->m_Channels_Opened[i].m_Channels_IN != 0 )
		  ||
		  ( pData->m_Channels_Opened[i].m_Channels_OUT != 0 )
		)
		{
		//Card available and channels opened.
			data_kernel.m_Channels_Opened[i].m_Type = pData->m_Channels_Opened[i].m_Type;
			data_kernel.m_Channels_Opened[i].m_SerialNumber = pData->m_Channels_Opened[i].m_SerialNumber;
			data_kernel.m_Channels_Opened[i].m_Channels_IN = pData->m_Channels_Opened[i].m_Channels_IN;
			data_kernel.m_Channels_Opened[i].m_Channels_OUT = pData->m_Channels_Opened[i].m_Channels_OUT;

			++nCardsUsed;
		}
		else
		{
		//Card not available or no channels opened.
			data_kernel.m_Channels_Opened[i].m_Type = (unsigned char)ioctl_cardInfo_s::CARD_TYPE_NOT_SET;
			data_kernel.m_Channels_Opened[i].m_SerialNumber = 0;
			data_kernel.m_Channels_Opened[i].m_Channels_IN = 0;
			data_kernel.m_Channels_Opened[i].m_Channels_OUT = 0;
		}
	}

#ifdef ASIO_LOGGER
	pLogger->Log( logger_c::LS_Channels_Open, &data_kernel );
#endif

//Send command...
	unsigned long	bytesReturned;

	BOOL	Status = DeviceIoControl(
	  m_hDeviceIoControl,
	  IOCTL_ASIO_OPEN_CLOSE_CHANNELS,
	  &data_kernel,
	  sizeof( data_kernel ),
	  NULL,
	  0,
	  &bytesReturned,
	  NULL
	);
	if ( Status == TRUE )
	{
	//Get number of opened channels.
		m_nChannels_INs = 0;
		m_nChannels_OUTs = 0;

		for ( int i = 0; i < ELEMENTS_IN_ARRAY( pData->m_Channels_Opened ); i++ )
		{
		//Is card defined?
			unsigned long		nStereoChannels;

			switch ( ioctl_cardInfo_s::CARD_TYPE( pData->m_Channels_Opened[i].m_Type ) )
			{
			case ioctl_cardInfo_s::CARD_TYPE_424:
			case ioctl_cardInfo_s::CARD_TYPE_496:
				nStereoChannels = 2;
				break;
			case ioctl_cardInfo_s::CARD_TYPE_824:
				nStereoChannels = 4;
				break;
			case ioctl_cardInfo_s::CARD_TYPE_NOT_SET:
			default:
				nStereoChannels = 0;
				break;
			}
			if ( nStereoChannels == 0 )
			{
				continue;
			}

		//Get number of channels opened.
			//Inputs...
			unsigned char		bit = 0x01;

			for ( unsigned long j = 0; j < nStereoChannels; j++ )
			{
				if ( ( pData->m_Channels_Opened[i].m_Channels_IN & bit ) != 0 )
				{
					++m_nChannels_INs;
				}

			//Next...
				bit <<= 1;
			}

			//Outputs...
			bit = 0x01;
			for ( unsigned long j = 0; j < nStereoChannels; j++ )
			{
				if ( ( pData->m_Channels_Opened[i].m_Channels_OUT & bit ) != 0 )
				{
					++m_nChannels_OUTs;
				}

			//Next...
				bit <<= 1;
			}
		}
	}
	else
	{
	//Something went wrong... are some channels already opened or are cards unsynced?
		if ( nCardsUsed > 0 )
		{
		//Are cards synced?
			unsigned long		nMasterCards = 0;

			for ( int i = 0; i < ELEMENTS_IN_ARRAY( m_Cards ); i++ )
			{
				if ( m_Cards[i].m_Type != ioctl_cardInfo_s::CARD_TYPE_NOT_SET )
				{
					if ( m_Cards[i].m_ClockSource == ioctl_cardInfo_s::CLOCK_SOURCE_CARD )
					{
						++nMasterCards;
					}
				}
			}

			if ( nMasterCards > 1 )
			{
				strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Channels from multiple cards were selected... please sync cards and try again." );
				m_ErrorMessage_showControlPanel = TRUE;

			#ifdef ASIO_LOGGER
				pLogger->Log( "Channels from multiple cards were selected... please sync cards and try again.\n" );
			#endif
			}
			else
			{
				strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Some channels could already be opened... close them or select other channels and try again." );

			#ifdef ASIO_LOGGER
				pLogger->Log( "Some channels could already be opened... close them or select other channels and try again.\n" );
			#endif
			}
		}
		else
		{
			strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Some channels could already be opened... close them or select other channels and try again." );

		#ifdef ASIO_LOGGER
			pLogger->Log( "Some channels could already be opened... close them or select other channels and try again.\n" );
		#endif
		}
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, Status );
#endif
	return Status;
}

BOOL glAsio_c::Kernel_ASIO_Channels_Close()
{
#ifdef ASIO_LOGGER
	Procedure_Start_Local();
#endif

//Set kernel struct data.
	ioctl_asio_channels_header_s	data_kernel;

	ZeroMemory( &data_kernel, sizeof( data_kernel ) );

	data_kernel.m_hAsio = m_hAsio;
	data_kernel.m_Request = ASIO_CHANNELS_CLOSE;

	for ( int i = 0; i < ELEMENTS_IN_ARRAY( data_kernel.m_Channels_Opened ); i++ )
	{
		data_kernel.m_Channels_Opened[i].m_Type = m_Cards[i].m_Type;
		data_kernel.m_Channels_Opened[i].m_SerialNumber = m_Cards[i].m_SerialNumber;
	}

//Send command...
	unsigned long	bytesReturned;

	BOOL	Status = DeviceIoControl(
	  m_hDeviceIoControl,
	  IOCTL_ASIO_OPEN_CLOSE_CHANNELS,
	  &data_kernel,
	  sizeof( data_kernel ),
	  NULL,
	  0,
	  &bytesReturned,
	  NULL
	);
	if ( Status == FALSE )
	{
	//Something went wrong...
		strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Failed to close channels. Internal error... please contact author about this." );

	#ifdef ASIO_LOGGER
		pLogger->Log( "Failed to close channels.\n" );
	#endif
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, Status );
#endif
	return Status;
}

BOOL glAsio_c::Kernel_ASIO_LockMemory()
{
#ifdef ASIO_LOGGER
	Procedure_Start_Local();
#endif

	BOOL	Status = TRUE;

//Validate.
	if ( m_pMemory_global != NULL )
	{
		Status = FALSE;
	}

//Allocate global memory.
//
//Size of buffers: ( (2*A + 2*B buffers) * nChannels ) * ( bufferSize * sizeof( SAMPLE ) )
	unsigned long		memorySize;

	if ( SUCCESS( Status ) )
	{
		memorySize = sizeof( ioctl_asio_switch ) + ( 4 * ( m_nChannels_INs + m_nChannels_OUTs ) * m_BufferSize * sizeof( unsigned long ) );

		m_pMemory_global = (unsigned long*)GlobalAlloc( GPTR, memorySize );
		if ( m_pMemory_global == NULL )
		{
			Status = FALSE;
		}
	}

//Lock memory...
	if ( SUCCESS( Status ) )
	{
	//Set data...
		ioctl_asio_memory_s		data;

		data.m_hAsio = m_hAsio;
		data.m_Memory_Size = memorySize;
		data.m_Memory_Address = (__int64)m_pMemory_global;

	//Send command...
		unsigned long			bytesReturned;

		Status = DeviceIoControl(
		  m_hDeviceIoControl,
		  IOCTL_ASIO_LOCK_MEMORY,
		  &data,
		  sizeof( data ),
		  NULL,
		  0,
		  &bytesReturned,
		  NULL
		);
		if ( Status == FALSE )
		{
		//Something went wrong...
			strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Failed to lock memory. Internal error... please contact author about this." );
		}
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, Status );
#endif
	return Status;
}

BOOL glAsio_c::Kernel_ASIO_UnlockMemory()
{
#ifdef ASIO_LOGGER
	Procedure_Start_Local();
#endif

	BOOL	Status = TRUE;

//Do the honky tonk...
	if ( m_pMemory_global != NULL )
	{
	//Unlock memory...
		unsigned long			bytesReturned;

		Status = DeviceIoControl(
		  m_hDeviceIoControl,
		  IOCTL_ASIO_UNLOCK_MEMORY,
		  &m_hAsio,
		  sizeof( m_hAsio ),
		  NULL,
		  0,
		  &bytesReturned,
		  NULL
		);
		if ( Status == TRUE )
		{
		//Free memory...
			GlobalFree( m_pMemory_global );
			m_pMemory_global = NULL;
		}
		else
		{
		//Something went wrong...
		//
		//Notice:
		//We'll leave system to do the cleanup.
			strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Failed to unlock memory. Internal error... please contact author about this." );
		}
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, Status );
#endif
	return Status;
}

BOOL glAsio_c::Kernel_ASIO_Start()
{
#ifdef ASIO_LOGGER
	Procedure_Start_Local();
#endif

	BOOL	Status = TRUE;

//Do the honky tonk...
	if ( m_pStart != NULL )
	{
	//Start channels...
		unsigned long			bytesReturned;

		Status = DeviceIoControl(
		  m_hDeviceIoControl,
		  IOCTL_ASIO_START,
		  m_pStart,
		  sizeof( ioctl_asio_start_header_s ),
		  NULL,
		  0,
		  &bytesReturned,
		  NULL
		);
		if ( Status == FALSE )
		{
			strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Kernel_ASIO_Start: failed to start channels." );
		}
	}
	else
	{
		strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Kernel_ASIO_Start: no start structure was set." );
		Status = FALSE;
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, Status );
#endif
	return Status;
}

BOOL glAsio_c::Kernel_ASIO_Stop()
{
#ifdef ASIO_LOGGER
	Procedure_Start_Local();
#endif

//Stop channels.
	unsigned long			bytesReturned;

	BOOL	Status = DeviceIoControl(
	  m_hDeviceIoControl,
	  IOCTL_ASIO_STOP,
	  &m_hAsio,
	  sizeof( m_hAsio ),
	  NULL,
	  0,
	  &bytesReturned,
	  NULL
	);
	if ( Status == FALSE )
	{
		strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Kernel_ASIO_Stop: failed to stop channels." );
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, Status );
#endif
	return Status;
}


BOOL glAsio_c::Kernel_ASIO_SwitchEvent_Remove()
{
#ifdef ASIO_LOGGER
	Procedure_Start_Local();
#endif

	BOOL	Status = TRUE;

	if ( m_Switch_hEvent != NULL )
	{
	//Release notification event.
		unsigned long		bytesReturned = 0;

		Status = DeviceIoControl(
		  m_hDeviceIoControl,
		  IOCTL_ASIO_RELEASE_SWITCH_EVENT,
		  &m_hAsio,
		  sizeof( m_hAsio ),
		  NULL,
		  0,
		  &bytesReturned,
		  NULL
		);

	//Close notification event.
	//
	//Notice:
	//We will force this one and let the driver be the one to release this handle.
		CloseHandle( m_Switch_hEvent );
		m_Switch_hEvent = NULL;
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, Status );
#endif
	return Status;
}

BOOL glAsio_c::Kernel_ASIO_SwitchEvent_Set()
{
#ifdef ASIO_LOGGER
	Procedure_Start_Local();
#endif

	BOOL	Status = TRUE;

//Create event.
	m_Switch_hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	if ( m_Switch_hEvent == NULL )
	{
		strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Failed to create switch event." );
		Status = FALSE;
	}

//Set notification.
	if ( Status == TRUE )
	{
	//Set structure.
		ioctl_asio_switch_event		switchEvent;

		switchEvent.m_hAsio = m_hAsio;
		switchEvent.m_SwitchEvent = (__int64)m_Switch_hEvent;

	//Send command.
		unsigned long		bytesReturned = 0;

		Status = DeviceIoControl(
		  m_hDeviceIoControl,
		  IOCTL_ASIO_SET_SWITCH_EVENT,
		  &switchEvent,
		  sizeof( switchEvent ),
		  NULL,
		  0,
		  &bytesReturned,
		  NULL
		);
		if ( Status == FALSE )
		{
			strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Driver failed to lock switch event." );
		}
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, Status );
#endif
	return Status;
}
