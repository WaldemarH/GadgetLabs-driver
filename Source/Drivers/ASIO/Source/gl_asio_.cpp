#include "main.h"
#include "gl_asio.h"
#include "gl_com_object.h"
#include "registry.h"
#include "..\..\Kernel\Source\driver_ioctl.h"
#include "..\..\Kernel\Source\driver_ioctl_asio.h"
#include "dlg_control_panel.h"


glAsio_c::glAsio_c( LPUNKNOWN pUnk, HRESULT* phr ) : IASIO(), CUnknown( TEXT( NAME_GL_ASIO_DRIVER ), pUnk, phr)
{
#ifdef ASIO_LOGGER
//Create logger.
	if ( pLogger == NULL )
	{
		pLogger = new logger_c();
	}

//Function start.
	Procedure_Start_Global();
#endif

//-------------------------------------------------------------------------------------------------
//Initialize variables...
	//ASIO buffers...
	m_pHostCallbacks = NULL;
	m_pStart = NULL;

	//ASIO channels...
	m_nChannels_INs = 0;
	m_pChannels_INs = NULL;

	m_nChannels_OUTs = 0;
	m_pChannels_OUTs = NULL;

	//ASIO clock...
	m_Latency_IN = 0;
	m_Latency_OUT = 0;

	//ASIO common stuff...
	m_ErrorMessage[0] = 0;
	m_ErrorMessage_showControlPanel = FALSE;

	//ASIO switch buffers...
	m_Switch_type = SWITCH_TYPE_ASIO_1;

	m_Switch_hThread = NULL;
	m_Switch_stopThread = FALSE;

	ZeroMemory( &m_AsioTime, sizeof( m_AsioTime ) );

	//Kernel ASIO...
	m_hAsio = 0;

	m_ProtectingIRP_hDeviceIoControl = NULL;
	ZeroMemory( &m_ProtectingIRP_overllaped, sizeof( m_ProtectingIRP_overllaped ) );

	m_pMemory_global = NULL;

	m_Switch_hEvent = NULL;

	//Kernel common...
	m_hDeviceIoControl = INVALID_HANDLE_VALUE;
	m_hDeviceIoControl_1 = INVALID_HANDLE_VALUE;
	m_hDeviceIoControl_2 = INVALID_HANDLE_VALUE;
	m_hDeviceIoControl_3 = INVALID_HANDLE_VALUE;
	m_hDeviceIoControl_4 = INVALID_HANDLE_VALUE;

	//Kernel DATA...
	m_BufferSize = 0;
	m_SampleRate = 0;

	ZeroMemory( &m_Cards, sizeof( m_Cards ) );

//-------------------------------------------------------------------------------------------------
//Do the honky tonk...
	BOOL	Status = TRUE;

//Get deviceIO handle to driver.
	m_hDeviceIoControl_1 = GetDeviceIoControlHeader( &DEVICE_IO_GUID_1, FALSE );
	m_hDeviceIoControl_2 = GetDeviceIoControlHeader( &DEVICE_IO_GUID_2, FALSE );
	m_hDeviceIoControl_3 = GetDeviceIoControlHeader( &DEVICE_IO_GUID_3, FALSE );
	m_hDeviceIoControl_4 = GetDeviceIoControlHeader( &DEVICE_IO_GUID_4, FALSE );

	m_hDeviceIoControl = GetDeviceIoControl();
	if ( m_hDeviceIoControl == INVALID_HANDLE_VALUE )
	{
		Status = FALSE;
	}

//Test driver engine versions.
	//ASIO
	if ( SUCCESS( Status ) )
	{
		Status = Kernel_ASIO_EngineVersion();
	}
	//DATA
	if ( SUCCESS( Status ) )
	{
		Status = Kernel_DATA_EngineVersion();
	}

//Get ASIO driver handle.
	if ( SUCCESS( Status ) )
	{
	//Notice:
	//If the host crashes right after we attain ASIO handle, we will loose a few bytes of memory in kernel driver.
	//Unfortunately there is nothing we can do about it... not until protecting IRP is set.
		Status = Kernel_ASIO_DriverHandle( DRIVER_HANDLE_TYPE_GET );
	}

//Set protecting IRP.
	if ( SUCCESS( Status ) )
	{
		Status = Kernel_ASIO_ProtectingIRP_Set();
	}

//Get cards data.
	if ( SUCCESS( Status ) )
	{
		Status = Kernel_DATA_CardsInfo();
	}

//Get data from registry.
	registry_data_s		data;
	registry_c			registry;

	if ( SUCCESS( Status ) )
	{
		BOOL	Status = registry.Read( &data );
		if ( Status == FALSE )
		{
		//Failed to read data from registry... probably a new ASIO host was started... registry value doesn't exist yet.
		//
		//Set default data values.
			ZeroMemory( &data, sizeof( data ) );

			//ASIOSTInt32LSB24 <- default
			data.m_SampleType = SAMPLETYPE_32BIT_LSB24;

			//First stereo IN and OUT channel on master card.
			Status = FALSE;
			for ( int i = 0; i < ELEMENTS_IN_ARRAY( m_Cards ); i++ )
			{
				if ( m_Cards[i].m_Type != ioctl_cardInfo_s::CARD_TYPE_NOT_SET )
				{
				//Cards data...
					data.m_Channels_Opened[i].m_Type = (unsigned char)m_Cards[i].m_Type;
					data.m_Channels_Opened[i].m_SerialNumber = m_Cards[i].m_SerialNumber;

				//Open 1st stereo IN & OUT channel...
					data.m_Channels_Opened[i].m_Channels_IN = 0x01;
					data.m_Channels_Opened[i].m_Channels_OUT = 0x01;

					Status = TRUE;
					break;
				}
			}

		//Save default value to registry.
		//
		//Notice:
		//This is intended mainly for ASIO ControlPanel, so that channels checkboxes will be marked.
			if ( SUCCESS( Status ) )
			{
				registry.Save( &data );
			}
		}

	#ifdef ASIO_LOGGER
		pLogger->Log( logger_c::LS_Channels_Open2, &data );
	#endif
	}

//Open selected channels.
//
//Notice:
//As soon as an ASIO channel will be opened, GUI will get the disable monitor checkBox event.
//At that time it will save old state and will disable monitoring on that channel.
//When the channel will be closed that monitoring state will be set to it's original form.
	if ( SUCCESS( Status ) )
	{
		Status = Kernel_ASIO_Channels_Open( &data );
	}

//Get buffer size.
//
//As channels are opened the bufferSize can't change anymore.
	if ( SUCCESS( Status ) )
	{
		Status = Kernel_DATA_BufferSize();
	}

//Lock buffer memory.
	if ( SUCCESS( Status ) )
	{
		Status = Kernel_ASIO_LockMemory();
	}

//Set switch event.
	if ( SUCCESS( Status ) )
	{
		Status = Kernel_ASIO_SwitchEvent_Set();
	}

//Set channels.
	if ( SUCCESS( Status ) )
	{
		Status = SetChannels( &data );
	}

//Set latencies.
	if ( SUCCESS( Status ) )
	{
		SetLatencies( &data );
	}

//Reset monitoring states.
//
//Notice:
//If user would want to monitor a channel it should do it through direct monitoring.
	if ( SUCCESS( Status ) )
	{
	//Reset monitoring on all channels.
		ASIOInputMonitor	monitoring;
		monitoring.input = -1;
		monitoring.state = ASIOFalse;

		monitoring.gain = 0;	//ignored
		monitoring.output = 0;	//ignored
		monitoring.pan = 0;		//ignored

		future( kAsioSetInputMonitor, &monitoring );
	}

//How did it went?
	m_Status_initialization = Status;

	if ( Status == FALSE )
	{
	//Show MessageBox as some ASIO hosts don't show error message if ASIO fails at instance creation.
		if ( m_ErrorMessage_showControlPanel == FALSE )
		{
			MessageBox( GetActiveWindow(), m_ErrorMessage, "Error", MB_OK | MB_ICONERROR | MB_TOPMOST );
		}
		else
		{
		//Show error message and control panel if selected...
			char	message[254];
			sprintf_s( message, sizeof( message ), "%s\n\nOpen ASIO Control Panel?", m_ErrorMessage );

			if ( IDYES == MessageBox( GetActiveWindow(), message, "Error", MB_YESNO | MB_ICONERROR | MB_TOPMOST ) )
			{
			//User requested that ControlPanel should be opened...
				DialogBox( GetModuleHandle( FILE_NAME_ASIO_DRIVER ), MAKEINTRESOURCE( IDD_DIALOGBAR ), GetActiveWindow(), &dlg_controlPanel_c::DialogProc );
			}
		}
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, Status );
#endif
}

glAsio_c::~glAsio_c()
{
#ifdef ASIO_LOGGER
	Procedure_Start_Global();
#endif

//Free channels.
	if ( m_pChannels_INs != NULL )
	{
		delete[] m_pChannels_INs;
		m_pChannels_INs = NULL;
	}
	if ( m_pChannels_OUTs != NULL )
	{
		delete[] m_pChannels_OUTs;
		m_pChannels_OUTs = NULL;
	}

//Release switch event.
	Kernel_ASIO_SwitchEvent_Remove();

//Unlock memory.
	Kernel_ASIO_UnlockMemory();

//Close channels.
	Kernel_ASIO_Channels_Close();

//Close protecting IRP.
	Kernel_ASIO_ProtectingIRP_Remove();

//Close ASIO handle.
	Kernel_ASIO_DriverHandle( DRIVER_HANDLE_TYPE_CLOSE );

//Close deviceIoControl handle.
	m_hDeviceIoControl = INVALID_HANDLE_VALUE;

	if ( m_hDeviceIoControl_4 != NULL )
	{
		CloseHandle( m_hDeviceIoControl_4 );
		m_hDeviceIoControl_4 = INVALID_HANDLE_VALUE;
	}
	if ( m_hDeviceIoControl_3 != NULL )
	{
		CloseHandle( m_hDeviceIoControl_3 );
		m_hDeviceIoControl_3 = INVALID_HANDLE_VALUE;
	}
	if ( m_hDeviceIoControl_2 != NULL )
	{
		CloseHandle( m_hDeviceIoControl_2 );
		m_hDeviceIoControl_2 = INVALID_HANDLE_VALUE;
	}
	if ( m_hDeviceIoControl_1 != NULL )
	{
		CloseHandle( m_hDeviceIoControl_1 );
		m_hDeviceIoControl_1 = INVALID_HANDLE_VALUE;
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( ASE_OK, -1 );
#endif
}
