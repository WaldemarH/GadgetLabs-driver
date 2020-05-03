#include "main.h"
#include "dlg_control_panel.h"


INT_PTR CALLBACK dlg_controlPanel_c::DialogProc( __in HWND hwndDlg, __in UINT uMsg, __in WPARAM wParam, __in LPARAM lParam )
{
//Process command...
	switch ( uMsg )
	{
	case WM_INITDIALOG:
	{
	//Initialize dialogBox.
		BOOL	Status = Initialize_Components( hwndDlg );

		//Get cards information...
		ioctl_cardInfo_s		cards[IOCTL_MAX_CARDS];
		ZeroMemory( &cards, sizeof( cards ) );

		if ( Status == TRUE )
		{
			Status = GetData_Cards( hwndDlg, cards );
		}

		//Set cards information...
		if ( Status == TRUE )
		{
			Initialize_Cards( hwndDlg, cards );
		}

		//Set channels...
		if ( Status == TRUE )
		{
			Initialize_Channels( hwndDlg, cards );
		}
		break;
	}
	case WM_COMMAND:
	{
	//Parse the menu selections:
		switch ( LOWORD( wParam ) )
		{
		case ID_FILE_SAVE_AND_EXIT:
		{
		//Save current settings to registry.
			//Get cards information...
			ioctl_cardInfo_s		cards[IOCTL_MAX_CARDS];

			ZeroMemory( cards, sizeof( cards ) );

			if ( FALSE == GetData_Cards( hwndDlg, cards ) )
			{
			//Something failed... let the user press the exit button...
				break;
			}

			//Save settings...
			if ( FALSE == Save_Channels( hwndDlg, cards ) )
			{
			//Something failed... let the user press the exit button...
				break;
			}

		//Exit -> signal that settings have changed.
			EndDialog( hwndDlg, TRUE );
			break;
		}
		case ID_FILE_EXIT:
		{
		//Exit without saving the settings.
			EndDialog( hwndDlg, FALSE );
			break;
		}
		default:
			break;
		}
		break;
	}
	case WM_CLOSE:
	{
	//Exit dialogBox.
		EndDialog( hwndDlg, FALSE );
		break;
	}
	default :
		return FALSE;
	}

	return TRUE;
}
