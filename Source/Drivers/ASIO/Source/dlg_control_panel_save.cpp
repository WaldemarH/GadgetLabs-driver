#include "main.h"
#include "dlg_control_panel.h"
#include "registry.h"

BOOL dlg_controlPanel_c::Save_Channels( HWND hwndDlg, ioctl_cardInfo_s cards[IOCTL_MAX_CARDS] )
{
//Fill data.
	registry_data_s		data;
	ZeroMemory( &data, sizeof( data ) );

	//Sample type.
	data.m_SampleType = (unsigned long) SendMessage( GetDlgItem( hwndDlg, IDC_SAMPLE_TYPE ), CB_GETCURSEL, 0, 0 );

	//Cards and channels...
	unsigned char		channelsInUse = 0;

	for ( int i = 0; i < IOCTL_MAX_CARDS; i++ )
	{
	//Does card exist?
		if ( cards[i].m_Type == ioctl_cardInfo_s::CARD_TYPE_NOT_SET )
		{
		//Ignore any data.
			continue;
		}

	//Set card data.
		data.m_Channels_Opened[i].m_Type = (unsigned char)cards[i].m_Type;
		data.m_Channels_Opened[i].m_SerialNumber = cards[i].m_SerialNumber;

	//Set channels.
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

		//Get check states.
		unsigned char		bit = 0x01;
		for ( int j = 0; j < 4; j++ )
		{
			if ( BST_CHECKED == IsDlgButtonChecked( hwndDlg, checkBox_IDs[j] ) )
			{
				data.m_Channels_Opened[i].m_Channels_IN |= bit;
			}

		//Next bit...
			bit <<= 1;
		}
		channelsInUse |= data.m_Channels_Opened[i].m_Channels_IN;

		bit = 0x01;
		for ( int j = 4; j < 8; j++ )
		{
			if ( BST_CHECKED == IsDlgButtonChecked( hwndDlg, checkBox_IDs[j] ) )
			{
				data.m_Channels_Opened[i].m_Channels_OUT |= bit;
			}

		//Next bit...
			bit <<= 1;
		}
		channelsInUse |= data.m_Channels_Opened[i].m_Channels_OUT;
	}

//Are any channels in use?
	if ( channelsInUse == 0 )
	{
		MessageBox(
		  hwndDlg,
		  "You need to use at least 1 channel.",
		  "Error",
		  MB_OK | MB_ICONWARNING | MB_TOPMOST
		);

		return FALSE;
	}

//Save data.
	registry_c		registry;

	return registry.Save( &data );
}
