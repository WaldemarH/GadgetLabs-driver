#include "main.h"
#include "dlg_main.h"
#include "..\..\Kernel\Source\driver_ioctl.h"
#include "..\..\Kernel\Source\driver_ioctl_gui.h"

void CControlPanelDlg::ProtectingIRP_Remove()
{
	unsigned long	bytesReturned;

	BOOL Status = DeviceIoControl(
	  m_hDeviceIoControl,
	  IOCTL_GUI_REMOVE_PROTECTION_IRP,
	  NULL,
	  0,
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
	//else
	//{
	////Failed to release protecting IRP -> as the application will close the Protecting IRP will fire and do what needs to be done.
	////And the system will close all remaining opened handles.
	//}
}

BOOL CControlPanelDlg::ProtectingIRP_Set()
{
	BOOL			Status = TRUE;

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
		  IOCTL_GUI_SET_PROTECTION_IRP,
		  NULL,
		  0,
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

	//Display error message.
		MessageBox(
		  "ControlPanel failed to establish protection IRP with the driver.\n\nPlease report this to the author.",
		  "Error",
		  MB_OK | MB_ICONWARNING | MB_TOPMOST
		);
	}

	return Status;
}
