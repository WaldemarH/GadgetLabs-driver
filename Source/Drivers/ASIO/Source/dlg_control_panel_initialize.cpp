#include "main.h"
#include "dlg_control_panel.h"
#include "registry.h"

BOOL dlg_controlPanel_c::Initialize_Components( HWND hwndDlg )
{
	BOOL		Status = TRUE;

//Show host path.
	char		hostPath[1024];

	DWORD		pathLength = GetModuleFileName( GetModuleHandle( NULL ), hostPath, sizeof( hostPath ) );
	if ( pathLength != 0 )
	{
		if ( pathLength > 63 )
		{
		//Shorten the path.
			char		visualPath[64];

			//Copy first 12 chars.
			strncpy_s( visualPath, sizeof(visualPath), hostPath, 12 );

			//Add dots...
			visualPath[12] = '.';
			visualPath[13] = '.';
			visualPath[14] = '.';

			//Copy last n chars.
			strncpy_s( &visualPath[15], sizeof(visualPath)-15, &hostPath[pathLength - 63 + 15], sizeof(visualPath)-15 );

		//Send to the dialogBox.
			SetDlgItemText( hwndDlg, ID_HOST_PATH, (char*)&visualPath );
		}
		else
		{
		//Send to the dialogBox.
			SetDlgItemText( hwndDlg, ID_HOST_PATH, (char*)&hostPath );
		}
	}
	else
	{
		Status = FALSE;
	}

//Set defines.
	if ( Status == TRUE )
	{
	//Set combo box -> sample type.
		HWND	hComboBox_sampleType = GetDlgItem( hwndDlg, IDC_SAMPLE_TYPE );
		if ( hComboBox_sampleType != NULL )
		{
		//ASIOSTInt32LSB24 <- default (SAMPLETYPE_32BIT_LSB24)
			SendMessage( hComboBox_sampleType, CB_ADDSTRING, 0, (LPARAM)"Type 1: 32bit -> 24LSB (default)" );
		//ASIOSTInt32LSB (SAMPLETYPE_32BIT_LSB)
			SendMessage( hComboBox_sampleType, CB_ADDSTRING, 0, (LPARAM)"Type 2: 32bit -> LSB" );
		//ASIOSTInt32MSB24 (SAMPLETYPE_32BIT_MSB24)
			SendMessage( hComboBox_sampleType, CB_ADDSTRING, 0, (LPARAM)"Type 3: 32bit -> 24MSB" );

		//Set default.
			SendMessage( hComboBox_sampleType, CB_SETCURSEL, 0, 0 );
		}
	}

	return Status;
}

void dlg_controlPanel_c::Initialize_Cards( HWND hwndDlg, ioctl_cardInfo_s cards[IOCTL_MAX_CARDS] )
{
//Set cards name and enable check boxes.
	for ( int i = 0; i < IOCTL_MAX_CARDS; i++ )
	{
	//Does card exist?
		if ( cards[i].m_Type == ioctl_cardInfo_s::CARD_TYPE_NOT_SET )
		{
		//No...
			int		ID = -1;

			switch ( i )
			{
			case 0: ID = ID_TYPE_CARD1; break;
			case 1: ID = ID_TYPE_CARD2; break;
			case 2: ID = ID_TYPE_CARD3; break;
			case 3: ID = ID_TYPE_CARD4; break;
			default:
				break;
			}
			if ( ID != -1 )
			{
				SetDlgItemText( hwndDlg, ID, "Card Not Installed" );
			}

		//Next...
			continue;
		}

	//Set card's name.
		unsigned long		card = 0;

		switch ( cards[i].m_Type )
		{
		case ioctl_cardInfo_s::CARD_TYPE_424: card = 424; break;
		case ioctl_cardInfo_s::CARD_TYPE_496: card = 496; break;
		case ioctl_cardInfo_s::CARD_TYPE_824: card = 824; break;
		default :
			break;
		}
		char		txt[64];
		sprintf_s( txt, sizeof( txt ), "%d (%s card)", card, ( cards[i].m_SerialNumber_masterCard == 0 ) ? "master" : "slave" );

		//Set name to dialog.
		int		ID = -1;
		switch ( i )
		{
		case 0: ID = ID_TYPE_CARD1; break;
		case 1: ID = ID_TYPE_CARD2; break;
		case 2: ID = ID_TYPE_CARD3; break;
		case 3: ID = ID_TYPE_CARD4; break;
		default:
			break;
		}
		if ( ID != -1 )
		{
			SetDlgItemText( hwndDlg, ID, txt );
		}

	//Enable channel.
		//Get IDs...
		unsigned long		checkBox_IDs[8];
		switch ( i )
		{
		case 0:
		{
			checkBox_IDs[0] = ID_CARD1_IN_1_2;
			checkBox_IDs[1] = ID_CARD1_IN_3_4;
			checkBox_IDs[2] = ID_CARD1_IN_5_6;
			checkBox_IDs[3] = ID_CARD1_IN_7_8;
			checkBox_IDs[4] = ID_CARD1_OUT_1_2;
			checkBox_IDs[5] = ID_CARD1_OUT_3_4;
			checkBox_IDs[6] = ID_CARD1_OUT_5_6;
			checkBox_IDs[7] = ID_CARD1_OUT_7_8;
			break;
		}
		case 1:
		{
			checkBox_IDs[0] = ID_CARD2_IN_1_2;
			checkBox_IDs[1] = ID_CARD2_IN_3_4;
			checkBox_IDs[2] = ID_CARD2_IN_5_6;
			checkBox_IDs[3] = ID_CARD2_IN_7_8;
			checkBox_IDs[4] = ID_CARD2_OUT_1_2;
			checkBox_IDs[5] = ID_CARD2_OUT_3_4;
			checkBox_IDs[6] = ID_CARD2_OUT_5_6;
			checkBox_IDs[7] = ID_CARD2_OUT_7_8;
			break;
		}
		case 2:
		{
			checkBox_IDs[0] = ID_CARD3_IN_1_2;
			checkBox_IDs[1] = ID_CARD3_IN_3_4;
			checkBox_IDs[2] = ID_CARD3_IN_5_6;
			checkBox_IDs[3] = ID_CARD3_IN_7_8;
			checkBox_IDs[4] = ID_CARD3_OUT_1_2;
			checkBox_IDs[5] = ID_CARD3_OUT_3_4;
			checkBox_IDs[6] = ID_CARD3_OUT_5_6;
			checkBox_IDs[7] = ID_CARD3_OUT_7_8;
			break;
		}
		case 3:
		{
			checkBox_IDs[0] = ID_CARD4_IN_1_2;
			checkBox_IDs[1] = ID_CARD4_IN_3_4;
			checkBox_IDs[2] = ID_CARD4_IN_5_6;
			checkBox_IDs[3] = ID_CARD4_IN_7_8;
			checkBox_IDs[4] = ID_CARD4_OUT_1_2;
			checkBox_IDs[5] = ID_CARD4_OUT_3_4;
			checkBox_IDs[6] = ID_CARD4_OUT_5_6;
			checkBox_IDs[7] = ID_CARD4_OUT_7_8;
			break;
		}
		}

		//Enable checkboxes...
		switch ( cards[i].m_Type )
		{
		case ioctl_cardInfo_s::CARD_TYPE_424:
		case ioctl_cardInfo_s::CARD_TYPE_496:
		{
		//Enable INs
			EnableWindow( GetDlgItem( hwndDlg, checkBox_IDs[0] ), TRUE );
			EnableWindow( GetDlgItem( hwndDlg, checkBox_IDs[1] ), TRUE );

		//Enable OUTs
			EnableWindow( GetDlgItem( hwndDlg, checkBox_IDs[4] ), TRUE );
			EnableWindow( GetDlgItem( hwndDlg, checkBox_IDs[5] ), TRUE );
			break;
		}
		case ioctl_cardInfo_s::CARD_TYPE_824:
		{
		//Enable INs
			EnableWindow( GetDlgItem( hwndDlg, checkBox_IDs[0] ), TRUE );
			EnableWindow( GetDlgItem( hwndDlg, checkBox_IDs[1] ), TRUE );
			EnableWindow( GetDlgItem( hwndDlg, checkBox_IDs[2] ), TRUE );
			EnableWindow( GetDlgItem( hwndDlg, checkBox_IDs[3] ), TRUE );

		//Enable OUTs
			EnableWindow( GetDlgItem( hwndDlg, checkBox_IDs[4] ), TRUE );
			EnableWindow( GetDlgItem( hwndDlg, checkBox_IDs[5] ), TRUE );
			EnableWindow( GetDlgItem( hwndDlg, checkBox_IDs[6] ), TRUE );
			EnableWindow( GetDlgItem( hwndDlg, checkBox_IDs[7] ), TRUE );
			break;
		}
		default:
			break;
		}
	}
}

BOOL dlg_controlPanel_c::Initialize_Channels( HWND hwndDlg, ioctl_cardInfo_s cards[IOCTL_MAX_CARDS] )
{
//Get data from registry.
	registry_data_s		data;
	registry_c			registry;

	BOOL	Status = registry.Read( &data );

//Channel type.
	if ( Status == TRUE )
	{
		SendMessage( GetDlgItem( hwndDlg, IDC_SAMPLE_TYPE ), CB_SETCURSEL, data.m_SampleType, 0 );
	}

//Check channels.
	if ( Status == TRUE )
	{
		for ( int i = 0; i < IOCTL_MAX_CARDS; i++ )
		{
		//Does card exist?
			if ( cards[i].m_Type == ioctl_cardInfo_s::CARD_TYPE_NOT_SET )
			{
			//Ignore any data.
				continue;
			}

		//Is the card the same?
			if (
			  ( data.m_Channels_Opened[i].m_Type != (unsigned char)cards[i].m_Type )
			  ||
			  ( data.m_Channels_Opened[i].m_SerialNumber != cards[i].m_SerialNumber )
			)
			{
			//Data and driver's cards are not the same... user has changed cards...
				continue;
			}

		//Get IDs...
			unsigned long		checkBox_IDs[8];
			switch ( i )
			{
			case 0:
			{
				checkBox_IDs[0] = ID_CARD1_IN_1_2;
				checkBox_IDs[1] = ID_CARD1_IN_3_4;
				checkBox_IDs[2] = ID_CARD1_IN_5_6;
				checkBox_IDs[3] = ID_CARD1_IN_7_8;
				checkBox_IDs[4] = ID_CARD1_OUT_1_2;
				checkBox_IDs[5] = ID_CARD1_OUT_3_4;
				checkBox_IDs[6] = ID_CARD1_OUT_5_6;
				checkBox_IDs[7] = ID_CARD1_OUT_7_8;
				break;
			}
			case 1:
			{
				checkBox_IDs[0] = ID_CARD2_IN_1_2;
				checkBox_IDs[1] = ID_CARD2_IN_3_4;
				checkBox_IDs[2] = ID_CARD2_IN_5_6;
				checkBox_IDs[3] = ID_CARD2_IN_7_8;
				checkBox_IDs[4] = ID_CARD2_OUT_1_2;
				checkBox_IDs[5] = ID_CARD2_OUT_3_4;
				checkBox_IDs[6] = ID_CARD2_OUT_5_6;
				checkBox_IDs[7] = ID_CARD2_OUT_7_8;
				break;
			}
			case 2:
			{
				checkBox_IDs[0] = ID_CARD3_IN_1_2;
				checkBox_IDs[1] = ID_CARD3_IN_3_4;
				checkBox_IDs[2] = ID_CARD3_IN_5_6;
				checkBox_IDs[3] = ID_CARD3_IN_7_8;
				checkBox_IDs[4] = ID_CARD3_OUT_1_2;
				checkBox_IDs[5] = ID_CARD3_OUT_3_4;
				checkBox_IDs[6] = ID_CARD3_OUT_5_6;
				checkBox_IDs[7] = ID_CARD3_OUT_7_8;
				break;
			}
			case 3:
			{
				checkBox_IDs[0] = ID_CARD4_IN_1_2;
				checkBox_IDs[1] = ID_CARD4_IN_3_4;
				checkBox_IDs[2] = ID_CARD4_IN_5_6;
				checkBox_IDs[3] = ID_CARD4_IN_7_8;
				checkBox_IDs[4] = ID_CARD4_OUT_1_2;
				checkBox_IDs[5] = ID_CARD4_OUT_3_4;
				checkBox_IDs[6] = ID_CARD4_OUT_5_6;
				checkBox_IDs[7] = ID_CARD4_OUT_7_8;
				break;
			}
			}

		//Check channels.
			unsigned char		bit = 0x01;
			for ( int j = 0; j < 4; j++ )
			{
				if ( ( data.m_Channels_Opened[i].m_Channels_IN & bit ) != 0 )
				{
				//Bit is set -> check channel.
					CheckDlgButton( hwndDlg, checkBox_IDs[j], BST_CHECKED );
				}

			//Next bit...
				bit <<= 1;
			}

			bit = 0x01;
			for ( int j = 4; j < 8; j++ )
			{
				if ( ( data.m_Channels_Opened[i].m_Channels_OUT & bit ) != 0 )
				{
				//Bit is set -> check channel.
					CheckDlgButton( hwndDlg, checkBox_IDs[j], BST_CHECKED );
				}

			//Next bit...
				bit <<= 1;
			}
		}
	}

	return Status;
}
