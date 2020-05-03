#include "main.h"
#include "dlg_control_panel.h"

BOOL dlg_controlPanel_c::GetData_Cards( HWND hwndDlg, ioctl_cardInfo_s cards[IOCTL_MAX_CARDS] )
{
	BOOL		Status = TRUE;

//Get handle to driver.
//
//Notice:
//Get all handles for uninstall protection.
	HANDLE		hDeviceIoControl_1 = GetDeviceIoControlHeader( &DEVICE_IO_GUID_1, FALSE );
	HANDLE		hDeviceIoControl_2 = GetDeviceIoControlHeader( &DEVICE_IO_GUID_2, FALSE );
	HANDLE		hDeviceIoControl_3 = GetDeviceIoControlHeader( &DEVICE_IO_GUID_3, FALSE );
	HANDLE		hDeviceIoControl_4 = GetDeviceIoControlHeader( &DEVICE_IO_GUID_4, FALSE );

	HANDLE		hDeviceIoControl;
	if ( hDeviceIoControl_1 != INVALID_HANDLE_VALUE )
	{
		hDeviceIoControl = hDeviceIoControl_1;
	}
	else if ( hDeviceIoControl_2 != INVALID_HANDLE_VALUE )
	{
		hDeviceIoControl = hDeviceIoControl_2;
	}
	else if ( hDeviceIoControl_3 != INVALID_HANDLE_VALUE )
	{
		hDeviceIoControl = hDeviceIoControl_3;
	}
	else if ( hDeviceIoControl_4 != INVALID_HANDLE_VALUE )
	{
		hDeviceIoControl = hDeviceIoControl_4;
	}
	else
	{
		hDeviceIoControl = INVALID_HANDLE_VALUE;
	}

	if ( hDeviceIoControl == INVALID_HANDLE_VALUE )
	{
		Status = FALSE;
	}

//Check io data version.
	unsigned long	bytesReturned;
	unsigned long	outputData;

	if ( Status == TRUE )
	{
		outputData = 0;

		Status = DeviceIoControl(
		  hDeviceIoControl,
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
				  hwndDlg,
				  "ControlPanel version is not the same as driver's one\n(communication with driver is not permited).\nYou need to get a newer version of the ASIO driver.",
				  "Error",
				  MB_OK | MB_ICONWARNING | MB_TOPMOST
				);
				Status = FALSE;
			}
		}
		else
		{
			MessageBox(
			  hwndDlg,
			  "Failed to get IO engine version.\n\nInternal error... please contact author about this.",
			  "Error",
			  MB_OK | MB_ICONWARNING | MB_TOPMOST
			);
		}
	}

//Get cards data.
	if ( Status == TRUE )
	{
		outputData = 0;

		Status = DeviceIoControl(
		  hDeviceIoControl,
		  IOCTL_DATA_GET_CARDS_INFORMATION,
		  NULL,
		  0,
		  &cards[0],
		  sizeof( ioctl_cardInfo_s ) * IOCTL_MAX_CARDS,
		  &bytesReturned,
		  NULL
		);
		if ( Status == FALSE )
		{
			MessageBox(
			  hwndDlg,
			  "Failed to get cards data.",
			  "Error",
			  MB_OK | MB_ICONWARNING | MB_TOPMOST
			);
		}
	}

//Free resources...
	hDeviceIoControl = INVALID_HANDLE_VALUE;
	if ( hDeviceIoControl_4 != NULL )
	{
		CloseHandle( hDeviceIoControl_4 );
		hDeviceIoControl_4 = INVALID_HANDLE_VALUE;
	}
	if ( hDeviceIoControl_3 != NULL )
	{
		CloseHandle( hDeviceIoControl_3 );
		hDeviceIoControl_3 = INVALID_HANDLE_VALUE;
	}
	if ( hDeviceIoControl_2 != NULL )
	{
		CloseHandle( hDeviceIoControl_2 );
		hDeviceIoControl_2 = INVALID_HANDLE_VALUE;
	}
	if ( hDeviceIoControl_1 != NULL )
	{
		CloseHandle( hDeviceIoControl_1 );
		hDeviceIoControl_1 = INVALID_HANDLE_VALUE;
	}

	return Status;
}
