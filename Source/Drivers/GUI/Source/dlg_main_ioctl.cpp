#include "main.h"
#include "dlg_main.h"
#include "..\..\Kernel\Source\driver_ioctl_gui.h"

HANDLE CControlPanelDlg::GetDeviceIoControl()
{
	if ( m_hDeviceIoControl_1 != INVALID_HANDLE_VALUE )
	{
		return m_hDeviceIoControl_1;
	}
	else if ( m_hDeviceIoControl_2 != INVALID_HANDLE_VALUE )
	{
		return m_hDeviceIoControl_2;
	}
	else if ( m_hDeviceIoControl_3 != INVALID_HANDLE_VALUE )
	{
		return m_hDeviceIoControl_3;
	}
	else if ( m_hDeviceIoControl_4 != INVALID_HANDLE_VALUE )
	{
		return m_hDeviceIoControl_4;
	}
	else
	{
		return INVALID_HANDLE_VALUE;
	}
}

//Procedure will test if the IO engines have the same version as the GUI's one.
//If not, GUT is not allowed to use the IO engines.
//
//Return: TRUE - the version is the same
//        FALSE - the version is not the same
BOOL CControlPanelDlg::TestIoEngineVersions( void )
{
	unsigned long	bytesReturned;
	unsigned long	outputData = 0;

	BOOL	Status = DeviceIoControl(
	  m_hDeviceIoControl,
	  IOCTL_GUI_GET_ENGINE_VERSION,
	  NULL,
	  0,
	  &outputData,
	  sizeof( outputData ),
	  &bytesReturned,
	  NULL
	);
	if ( Status == TRUE )
	{
	//Test deviceIoControl version.
		if ( outputData != IOCTL_GUI_ENGINE_VERSION )
		{
			MessageBox(
			  "ControlPanel version is not the same as driver's one\n(communication with driver is not permited).\nYou need to get a newer version of the ControlPanel.",
			  "Error",
			  MB_OK | MB_ICONWARNING | MB_TOPMOST
			);
			Status = FALSE;
		}
	}
	else
	{
		MessageBox(
		  "Failed to get IO engine version.\n\nInternal error... please contact author about this.",
		  "Error",
		  MB_OK | MB_ICONWARNING | MB_TOPMOST
		);
	}
	if ( Status == TRUE )
	{
		Status = DeviceIoControl(
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
		//Test deviceIoControl version.
			if ( outputData != IOCTL_DATA_ENGINE_VERSION )
			{
				MessageBox(
				  "ControlPanel version is not the same as driver's one\n(communication with driver is not permited).\nYou need to get a newer version of the ControlPanel.",
				  "Error",
				  MB_OK | MB_ICONWARNING | MB_TOPMOST
				);
				Status = FALSE;
			}
		}
		else
		{
			MessageBox(
			  "Failed to get IO engine version.\n\nInternal error... please contact author about this.",
			  "Error",
			  MB_OK | MB_ICONWARNING | MB_TOPMOST
			);
		}
	}

	return Status;
}
