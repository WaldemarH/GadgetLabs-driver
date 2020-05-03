#include "main.h"
#include "gl_asio.h"
#include "gl_com_object.h"
#include "..\..\Kernel\Source\driver_ioctl_asio.h"
#include "dlg_control_panel.h"

ASIOBool glAsio_c::init( void* pSysHandle )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Global();
#endif

	ASIOBool	Status;

//Was initialization succesful?
	if ( m_Status_initialization == TRUE )
	{
	//All ok.
		Status = ASIOTrue;
	}
	else
	{
	//Initialization failed...
	//
	//Notice:
	//Don't copy anything to error message... it's already set.
		Status = ASIOFalse;
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, Status );
#endif
	return Status;
}

void glAsio_c::getDriverName( char* pName )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Global();
#endif

	strcpy_s( pName, 32, NAME_GL_ASIO_DRIVER );

#ifdef ASIO_LOGGER
	pLogger->Log( "  name: %s\n", pName );
	Procedure_Exit( -1, TRUE );
#endif
}

long glAsio_c::getDriverVersion( void )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Global();
#endif

	long	version = IOCTL_ASIO_ENGINE_VERSION;

#ifdef ASIO_LOGGER
	pLogger->Log( "  version: %d\n", version );
	Procedure_Exit( -1, TRUE );
#endif

	return version;
}

void glAsio_c::getErrorMessage( char* pString )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Global();
#endif

	strcpy_s( pString, 128, m_ErrorMessage );

#ifdef ASIO_LOGGER
	pLogger->Log( "ErrorMessage: %s\n", m_ErrorMessage );
	Procedure_Exit( -1, TRUE );
#endif
}

ASIOError glAsio_c::controlPanel( void )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Global();
#endif

//Display ASIO control panel.
	BOOL	Status = (BOOL)DialogBox( GetModuleHandle( FILE_NAME_ASIO_DRIVER ), MAKEINTRESOURCE( IDD_DIALOGBAR ), GetActiveWindow(), &dlg_controlPanel_c::DialogProc );

//Did user save new data?
	if ( Status == TRUE )
	{
		if ( m_pHostCallbacks != NULL )
		{
		//Send ASIO host that ASIO_Exit -> ASIO_Init must be called.
			m_pHostCallbacks->asioMessage( kAsioResetRequest, 0, NULL, 0 );

		#ifdef ASIO_LOGGER
			pLogger->Log( "  kAsioResetRequest\n" );
		#endif
		}
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, TRUE );
#endif
	return ASE_OK;
}

ASIOError glAsio_c::future( long selector, void* pOpt )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Global();
	pLogger->Log( logger_c::LS_Future, &selector, pOpt );
#endif

	ASIOError		Status = ASE_SUCCESS;

	switch ( selector )
	{
//	case kAsioEnableTimeCodeRead:
//		timeCodeMode = TRUE;
//		break;
//	case kAsioDisableTimeCodeRead:
//		timeCodeMode = FALSE;
//		break;
	case kAsioSetInputMonitor:
	{
		if ( FALSE == Kernel_DATA_InputMonitoring( (ASIOInputMonitor*)pOpt ) )
		{
			Status = ASE_HWMalfunction;
		}
		break;
	}
	case kAsioCanInputMonitor:
	case kAsioCanTimeInfo:
//	case kAsioCanTimeCode:
		break;
	default:
		Status = ASE_NotPresent;
		break;
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( Status, -1 );
#endif
	return Status;
}

