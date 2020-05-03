#include "main.h"
#include "gl_asio.h"
#include "..\..\Kernel\Source\driver_ioctl.h"
#include "..\..\Kernel\Source\driver_ioctl_asio.h"

BOOL glAsio_c::Kernel_ASIO_ProtectingIRP_Remove()
{
#ifdef ASIO_LOGGER
	Procedure_Start_Local();
#endif

//Validate.
	if ( m_hAsio == 0 )
	{
	//Failed to release protecting IRP -> as the application will close the Protecting IRP will fire and do what needs to be done.
	//And the system will close all remaining opened handles.
		strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Failed to remove protecting IRP." );
		return FALSE;
	}

//Send command.
	unsigned long	bytesReturned;

	BOOL Status = DeviceIoControl(
	  m_hDeviceIoControl,
	  IOCTL_ASIO_REMOVE_PROTECTION_IRP,
	  &m_hAsio,
	  sizeof( m_hAsio ),
	  NULL,
	  0,
	  &bytesReturned,
	  NULL
	);
	if ( Status == TRUE )
	{
	//Close event.
		if ( m_ProtectingIRP_overllaped.hEvent != NULL )
		{
			CloseHandle( m_ProtectingIRP_overllaped.hEvent );
			m_ProtectingIRP_overllaped.hEvent = NULL;
		}

	//Close handle to driver.
		if ( m_ProtectingIRP_hDeviceIoControl != NULL )
		{
			CloseHandle( m_ProtectingIRP_hDeviceIoControl );
			m_ProtectingIRP_hDeviceIoControl = NULL;
		}
	}
	else
	{
	//Failed to release protecting IRP -> as the application will close the Protecting IRP will fire and do what needs to be done.
	//And the system will close all remaining opened handles.
		strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Failed to remove protecting IRP." );
		return FALSE;
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, Status );
#endif
	return TRUE;
}

BOOL glAsio_c::Kernel_ASIO_ProtectingIRP_Set()
{
#ifdef ASIO_LOGGER
	Procedure_Start_Local();
#endif

	BOOL			Status = TRUE;

//Validate.
	if ( m_hAsio == 0 )
	{
		strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Failed to set protecting IRP." );
		return FALSE;
	}

//Get overllaped handle to driver.
	m_ProtectingIRP_hDeviceIoControl = GetDeviceIoControlHeader( &DEVICE_IO_GUID_1, TRUE );
	if ( m_ProtectingIRP_hDeviceIoControl == NULL )
	{
		Status = FALSE;
	}

//Create event for overllaped structure.
	if ( Status == TRUE )
	{
		m_ProtectingIRP_overllaped.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
		if ( m_ProtectingIRP_overllaped.hEvent == NULL )
		{
			Status = FALSE;
		}
	}

//Send protecting ioctl command to driver.
	if ( Status == TRUE )
	{
		Status = DeviceIoControl(
		  m_ProtectingIRP_hDeviceIoControl,
		  IOCTL_ASIO_SET_PROTECTION_IRP,
		  &m_hAsio,
		  sizeof( m_hAsio ),
		  NULL,
		  0,
		  NULL,
		  &m_ProtectingIRP_overllaped
		);

	//DeviceIoControl should return FALSE with ERROR_IO_PENDING error.
	//
	//For us this means STATUS_SUCCESS.
		if (
		  ( Status == FALSE )
		  &&
		  ( ERROR_IO_PENDING == GetLastError() )
		)
		{
		//All is ok.
			Status = TRUE;
		}
		else
		{
			Status = FALSE;
		}
	}

//Free resources?
	if ( Status == FALSE )
	{
	//Close event.
		if ( m_ProtectingIRP_overllaped.hEvent != NULL )
		{
			CloseHandle( m_ProtectingIRP_overllaped.hEvent );
			m_ProtectingIRP_overllaped.hEvent = NULL;
		}

	//Close handle to driver.
		if ( m_ProtectingIRP_hDeviceIoControl != NULL )
		{
			CloseHandle( m_ProtectingIRP_hDeviceIoControl );
			m_ProtectingIRP_hDeviceIoControl = NULL;
		}

		strcpy_s( m_ErrorMessage, sizeof(m_ErrorMessage), "Failed to set protecting IRP." );
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, Status );
#endif
	return Status;
}
