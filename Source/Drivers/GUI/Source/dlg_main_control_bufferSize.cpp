#include "main.h"
#include "dlg_main.h"

//Procedure will get current driver bufferSize.
//
//Return: 0 (request failed) or 32, 64, 128,256, 512 or 1024
BOOL CControlPanelDlg::BufferSize_Get( void )
{
	unsigned long	bytesReturned;
	unsigned long	bufferSize = 0;		//get bufferSize...

	BOOL Status = DeviceIoControl(
	  m_hDeviceIoControl,
	  IOCTL_DATA_GET_SET_BUFFER_SIZE,
	  &bufferSize,
	  sizeof( bufferSize ),
	  &bufferSize,
	  sizeof( bufferSize ),
	  &bytesReturned,
	  NULL
	);
	if ( Status == TRUE )
	{
		CMenu*		pMenu = GetMenu();
		if ( pMenu != NULL )
		{
			switch ( bufferSize )
			{
			case 32:
				pMenu->CheckMenuItem( ID_BUFFERSIZE_32SAMPLES, MF_CHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_64SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_128SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_256SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_512SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_1024SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				break;
			case 64:
				pMenu->CheckMenuItem( ID_BUFFERSIZE_32SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_64SAMPLES, MF_CHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_128SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_256SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_512SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_1024SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				break;
			case 128:
				pMenu->CheckMenuItem( ID_BUFFERSIZE_32SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_64SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_128SAMPLES, MF_CHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_256SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_512SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_1024SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				break;
			case 256:
				pMenu->CheckMenuItem( ID_BUFFERSIZE_32SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_64SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_128SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_256SAMPLES, MF_CHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_512SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_1024SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				break;
			case 512:
				pMenu->CheckMenuItem( ID_BUFFERSIZE_32SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_64SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_128SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_256SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_512SAMPLES, MF_CHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_1024SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				break;
			case 1024:
				pMenu->CheckMenuItem( ID_BUFFERSIZE_32SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_64SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_128SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_256SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_512SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
				pMenu->CheckMenuItem( ID_BUFFERSIZE_1024SAMPLES, MF_CHECKED | MF_BYCOMMAND );
				break;
			}
		}
		else
		{
			Status = FALSE;
		}
	}

	if ( Status == FALSE )
	{
		MessageBox(
		  "Failed to get bufferSize.\n\nInternal error... please contact author about this.",
		  "Error",
		  MB_OK | MB_ICONWARNING | MB_TOPMOST
		);
	}

	return Status;
}

unsigned long CControlPanelDlg::BufferSize_GetValue()
{
//Get current bufferSize setting.
	unsigned long	bufferSize_current = 0;

	CMenu*			pMenu = GetMenu();
	if ( pMenu != NULL )
	{
		if ( 0 != ( MF_CHECKED & pMenu->GetMenuState( ID_BUFFERSIZE_32SAMPLES, MF_BYCOMMAND ) ) )
		{
			bufferSize_current = 32;
		}
		else if ( 0 != ( MF_CHECKED & pMenu->GetMenuState( ID_BUFFERSIZE_64SAMPLES, MF_BYCOMMAND ) ) )
		{
			bufferSize_current = 64;
		}
		else if ( 0 != ( MF_CHECKED & pMenu->GetMenuState( ID_BUFFERSIZE_128SAMPLES, MF_BYCOMMAND ) ) )
		{
			bufferSize_current = 128;
		}
		else if ( 0 != ( MF_CHECKED & pMenu->GetMenuState( ID_BUFFERSIZE_256SAMPLES, MF_BYCOMMAND ) ) )
		{
			bufferSize_current = 256;
		}
		else if ( 0 != ( MF_CHECKED & pMenu->GetMenuState( ID_BUFFERSIZE_512SAMPLES, MF_BYCOMMAND ) ) )
		{
			bufferSize_current = 512;
		}
		else
		{
			bufferSize_current = 1024;
		}
	}

	return bufferSize_current;
}

//Procedure will set new driver bufferSize.
//
//bufferSize: 32, 64, 128,256,512,1024
//
//Return: TRUE/FALSE
BOOL CControlPanelDlg::BufferSize_Set( unsigned long bufferSize )
{
	BOOL	Status = TRUE;

//Validate...
	switch ( bufferSize )
	{
	case 32:
	case 64:
	case 128:
	case 256:
	case 512:
	case 1024:
		break;
	default:
		Status = FALSE;
		break;
	}

//Get current bufferSize setting.
	unsigned long	bufferSize_current = 0;

	if ( Status == TRUE )
	{
		CMenu*			pMenu = GetMenu();
		if ( 0 != ( MF_CHECKED & pMenu->GetMenuState( ID_BUFFERSIZE_32SAMPLES, MF_BYCOMMAND ) ) )
		{
			bufferSize_current = 32;
		}
		else if ( 0 != ( MF_CHECKED & pMenu->GetMenuState( ID_BUFFERSIZE_64SAMPLES, MF_BYCOMMAND ) ) )
		{
			bufferSize_current = 64;
		}
		else if ( 0 != ( MF_CHECKED & pMenu->GetMenuState( ID_BUFFERSIZE_128SAMPLES, MF_BYCOMMAND ) ) )
		{
			bufferSize_current = 128;
		}
		else if ( 0 != ( MF_CHECKED & pMenu->GetMenuState( ID_BUFFERSIZE_256SAMPLES, MF_BYCOMMAND ) ) )
		{
			bufferSize_current = 256;
		}
		else if ( 0 != ( MF_CHECKED & pMenu->GetMenuState( ID_BUFFERSIZE_512SAMPLES, MF_BYCOMMAND ) ) )
		{
			bufferSize_current = 512;
		}
		else
		{
			bufferSize_current = 1024;
		}
	}

//Set new bufferSize.
	if ( Status == TRUE )
	{
		if ( bufferSize_current != bufferSize )
		{
			unsigned long	bytesReturned;

			Status = DeviceIoControl(
			  m_hDeviceIoControl,
			  IOCTL_DATA_GET_SET_BUFFER_SIZE,
			  &bufferSize,
			  sizeof( bufferSize ),
			  &bufferSize,
			  sizeof( bufferSize ),
			  &bytesReturned,
			  NULL
			);
			if ( Status == TRUE )
			{
				if ( bufferSize == 0 )
				{
					Status = FALSE;
				}
			}
		}
	}

//Update GUI.
	if ( Status == TRUE )
	{
		CMenu*			pMenu = GetMenu();

		switch ( bufferSize )
		{
		case 32:
			pMenu->CheckMenuItem( ID_BUFFERSIZE_32SAMPLES, MF_CHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_64SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_128SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_256SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_512SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_1024SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			break;
		case 64:
			pMenu->CheckMenuItem( ID_BUFFERSIZE_32SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_64SAMPLES, MF_CHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_128SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_256SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_512SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_1024SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			break;
		case 128:
			pMenu->CheckMenuItem( ID_BUFFERSIZE_32SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_64SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_128SAMPLES, MF_CHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_256SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_512SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_1024SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			break;
		case 256:
			pMenu->CheckMenuItem( ID_BUFFERSIZE_32SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_64SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_128SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_256SAMPLES, MF_CHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_512SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_1024SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			break;
		case 512:
			pMenu->CheckMenuItem( ID_BUFFERSIZE_32SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_64SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_128SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_256SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_512SAMPLES, MF_CHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_1024SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			break;
		case 1024:
			pMenu->CheckMenuItem( ID_BUFFERSIZE_32SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_64SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_128SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_256SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_512SAMPLES, MF_UNCHECKED | MF_BYCOMMAND );
			pMenu->CheckMenuItem( ID_BUFFERSIZE_1024SAMPLES, MF_CHECKED | MF_BYCOMMAND );
			break;
		}
	}

//Was it set?
	if ( Status == FALSE )
	{
		MessageBox(
		  "Failed to set bufferSize.\nAre there any channels opened... close them and try again.",
		  "Error",
		  MB_OK | MB_ICONWARNING | MB_TOPMOST
		);
	}

	return Status;
}

