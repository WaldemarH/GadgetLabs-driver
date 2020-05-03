#include "main.h"
#include "dlg_main.h"

BOOL CControlPanelDlg::ChannelStatus_GetAll( void )
{
//Get selected card.
	ioctl_cardInfo_s*	pCard = Cards_Tree_GetSelected();
	if ( pCard == NULL )
	{
	//Card doesn't exist -> disable all...
		ChannelStatus_DisableAll();
		return TRUE;
	}

//Get memory for ioctl buffer.
	unsigned long						size = sizeof(ioctl_channelProperty_header_s) + 32*sizeof(ioctl_channelProperty_s);

	ioctl_channelProperty_header_s*		pHeader = (ioctl_channelProperty_header_s*)new char[size];
	if ( pHeader == NULL )
		return FALSE;

//Set header.
	pHeader->m_Request = PROPERTY_GET_PROPERTY;
	pHeader->m_RequestBy = PROPERTY_REQUEST_BY_GUI;
	pHeader->m_SerialNumber = pCard->m_SerialNumber;
	pHeader->m_nPropertyDefinitions = 32;

//Set property structures.
	ioctl_channelProperty_s*			pProperty = (ioctl_channelProperty_s*)( pHeader + 1 );

	ChannelStatus_PrepareForAll( pProperty, pCard->m_Type );

//Get channel properties.
	unsigned long		bytesReturned;

	BOOL Status = DeviceIoControl(
	  m_hDeviceIoControl,
	  IOCTL_DATA_GET_SET_CHANNEL_PROPERTIES,
	  pHeader,
	  size,
	  pHeader,
	  size,
	  &bytesReturned,
	  NULL
	);

	//All ok?
	if ( Status == FALSE )
	{
	//Free resources...
		delete[] pHeader;

		MessageBox( "Failed to get channels data.", "Error", MB_OK | MB_ICONWARNING | MB_TOPMOST );

		return FALSE;
	}

//Set GUI.
	CButton*			hwndCheck;

	//Input gain...
	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_1 ); hwndCheck->SetCheck( pProperty[0].m_Position ); hwndCheck->EnableWindow( pProperty[0].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_2 ); hwndCheck->SetCheck( pProperty[1].m_Position ); hwndCheck->EnableWindow( pProperty[1].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_3 ); hwndCheck->SetCheck( pProperty[2].m_Position ); hwndCheck->EnableWindow( pProperty[2].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_4 ); hwndCheck->SetCheck( pProperty[3].m_Position ); hwndCheck->EnableWindow( pProperty[3].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_5 ); hwndCheck->SetCheck( pProperty[4].m_Position ); hwndCheck->EnableWindow( pProperty[4].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_6 ); hwndCheck->SetCheck( pProperty[5].m_Position ); hwndCheck->EnableWindow( pProperty[5].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_7 ); hwndCheck->SetCheck( pProperty[6].m_Position ); hwndCheck->EnableWindow( pProperty[6].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_8 ); hwndCheck->SetCheck( pProperty[7].m_Position ); hwndCheck->EnableWindow( pProperty[7].m_ControlEnabled );

	//Output gain...
	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_1 ); hwndCheck->SetCheck( pProperty[8].m_Position ); hwndCheck->EnableWindow( pProperty[8].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_2 ); hwndCheck->SetCheck( pProperty[9].m_Position ); hwndCheck->EnableWindow( pProperty[9].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_3 ); hwndCheck->SetCheck( pProperty[10].m_Position ); hwndCheck->EnableWindow( pProperty[10].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_4 ); hwndCheck->SetCheck( pProperty[11].m_Position ); hwndCheck->EnableWindow( pProperty[11].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_5 ); hwndCheck->SetCheck( pProperty[12].m_Position ); hwndCheck->EnableWindow( pProperty[12].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_6 ); hwndCheck->SetCheck( pProperty[13].m_Position ); hwndCheck->EnableWindow( pProperty[13].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_7 ); hwndCheck->SetCheck( pProperty[14].m_Position ); hwndCheck->EnableWindow( pProperty[14].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_8 ); hwndCheck->SetCheck( pProperty[15].m_Position ); hwndCheck->EnableWindow( pProperty[15].m_ControlEnabled );

	//Monitoring...
	hwndCheck = (CButton*)GetDlgItem( IDC_MON_1 ); hwndCheck->SetCheck( pProperty[16].m_Position ); hwndCheck->EnableWindow( pProperty[16].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_MON_2 ); hwndCheck->SetCheck( pProperty[17].m_Position ); hwndCheck->EnableWindow( pProperty[17].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_MON_3 ); hwndCheck->SetCheck( pProperty[18].m_Position ); hwndCheck->EnableWindow( pProperty[18].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_MON_4 ); hwndCheck->SetCheck( pProperty[19].m_Position ); hwndCheck->EnableWindow( pProperty[19].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_MON_5 ); hwndCheck->SetCheck( pProperty[20].m_Position ); hwndCheck->EnableWindow( pProperty[20].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_MON_6 ); hwndCheck->SetCheck( pProperty[21].m_Position ); hwndCheck->EnableWindow( pProperty[21].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_MON_7 ); hwndCheck->SetCheck( pProperty[22].m_Position ); hwndCheck->EnableWindow( pProperty[22].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_MON_8 ); hwndCheck->SetCheck( pProperty[23].m_Position ); hwndCheck->EnableWindow( pProperty[23].m_ControlEnabled );

	//Mute...
	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_1 ); hwndCheck->SetCheck( pProperty[24].m_Position ); hwndCheck->EnableWindow( pProperty[24].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_2 ); hwndCheck->SetCheck( pProperty[25].m_Position ); hwndCheck->EnableWindow( pProperty[25].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_3 ); hwndCheck->SetCheck( pProperty[26].m_Position ); hwndCheck->EnableWindow( pProperty[26].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_4 ); hwndCheck->SetCheck( pProperty[27].m_Position ); hwndCheck->EnableWindow( pProperty[27].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_5 ); hwndCheck->SetCheck( pProperty[28].m_Position ); hwndCheck->EnableWindow( pProperty[28].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_6 ); hwndCheck->SetCheck( pProperty[29].m_Position ); hwndCheck->EnableWindow( pProperty[29].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_7 ); hwndCheck->SetCheck( pProperty[30].m_Position ); hwndCheck->EnableWindow( pProperty[30].m_ControlEnabled );
	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_8 ); hwndCheck->SetCheck( pProperty[31].m_Position ); hwndCheck->EnableWindow( pProperty[31].m_ControlEnabled );

//Free resources...
	delete[] pHeader;

//Expand Opened channels.
	if ( m_Cards_Tree_openedChannels[pCard->m_SerialNumber] != NULL )
	{
	//Get tree.
		CTreeCtrl*			hwndTree = (CTreeCtrl*)GetDlgItem( IDC_TREE );
		if ( hwndTree != NULL )
		{
			hwndTree->Expand( m_Cards_Tree_openedChannels[pCard->m_SerialNumber], TVE_EXPAND );
		}
	}

	return TRUE;
}

BOOL CControlPanelDlg::ChannelStatus_GetAll_GetValue( registry_data_s* pSettings )
{
//Get memory for ioctl buffer.
	unsigned long						size = sizeof(ioctl_channelProperty_header_s) + 32*sizeof(ioctl_channelProperty_s);

	ioctl_channelProperty_header_s*		pHeader = (ioctl_channelProperty_header_s*)new char[size];
	if ( pHeader == NULL )
		return FALSE;

//Set header.
	pHeader->m_Request = PROPERTY_GET_PROPERTY;
	pHeader->m_RequestBy = PROPERTY_REQUEST_BY_GUI;
	pHeader->m_nPropertyDefinitions = 32;

//Set property structures.
	ioctl_channelProperty_s*			pProperty = (ioctl_channelProperty_s*)( pHeader + 1 );

//Get channel peoperties from all the installed cards.
	for ( int i= 0; i < ELEMENTS_IN_ARRAY( m_Cards ); i++ )
	{
	//Reset flags...
		registry_data_card_s*		pCard_channels = &pSettings->m_Cards[i];
		
		pCard_channels->m_SerialNumber = 0;
		pCard_channels->m_Type = (unsigned short)ioctl_cardInfo_s::CARD_TYPE_NOT_SET;

		pCard_channels->m_Input_Volume = 0;
		pCard_channels->m_Output_Volume = 0;
		pCard_channels->m_Monitoring = 0;
		pCard_channels->m_Mute = 0;

	//Is card present?
		if ( m_Cards[i].m_Type == ioctl_cardInfo_s::CARD_TYPE_NOT_SET )
		{
			continue;
		}

	//Set header...
		pHeader->m_SerialNumber = m_Cards[i].m_SerialNumber;

	//Set channels...
		ChannelStatus_PrepareForAll( pProperty, m_Cards[i].m_Type );

	//Get channel properties.
		unsigned long		bytesReturned;

		BOOL Status = DeviceIoControl(
		  m_hDeviceIoControl,
		  IOCTL_DATA_GET_SET_CHANNEL_PROPERTIES,
		  pHeader,
		  size,
		  pHeader,
		  size,
		  &bytesReturned,
		  NULL
		);
		if ( Status == FALSE )
		{
		//Free resources...
			delete[] pHeader;

			return FALSE;
		}

	//Save channel properties.
		pCard_channels->m_SerialNumber = m_Cards[i].m_SerialNumber;
		pCard_channels->m_Type = (unsigned short)m_Cards[i].m_Type;

		//Input volume...
		unsigned char	bit = 0x01;
		for ( int j = 0; j < 8; j++ )
		{
			pCard_channels->m_Input_Volume |= ( pProperty[j].m_Position != 0 ) ? bit : 0;

		//Next bit...
			bit <<= 1;
		}

		//Output volume...
		bit = 0x01;
		for ( int j = 8; j < 16; j++ )
		{
			pCard_channels->m_Output_Volume |= ( pProperty[j].m_Position != 0 ) ? bit : 0;

		//Next bit...
			bit <<= 1;
		}

		//Monitoring...
		bit = 0x01;
		for ( int j = 16; j < 24; j++ )
		{
			if ( pProperty[j].m_ControlEnabled == PROPERTY_CONTROL_ENABLED )
			{
			//Channel is enabled -> save the value from the card.
				pCard_channels->m_Monitoring |= ( pProperty[j].m_Position != 0 ) ? bit : 0;
			}
			else
			{
			//Channel is disabled... it's an ASIO channel -> save the old state.
				pCard_channels->m_Monitoring |= ( m_Monitoring_oldState[i] & bit );
			}

		//Next bit...
			bit <<= 1;
		}

		//Mute...
		bit = 0x01;
		for ( int j = 24; j < 32; j++ )
		{
			pCard_channels->m_Mute |= ( pProperty[j].m_Position != 0 ) ? bit : 0;

		//Next bit...
			bit <<= 1;
		}
	}

//Free resources.
	delete[] pHeader;

	return TRUE;
}

void CControlPanelDlg::ChannelStatus_DisableAll()
{
	CButton*	hwndCheck;

	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_1 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_2 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_3 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_4 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_5 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_6 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_7 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_8 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );

	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_1 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_2 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_3 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_4 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_5 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_6 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_7 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_8 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );

	hwndCheck = (CButton*)GetDlgItem( IDC_MON_1 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_MON_2 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_MON_3 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_MON_4 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_MON_5 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_MON_6 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_MON_7 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_MON_8 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );

	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_1 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_2 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_3 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_4 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_5 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_6 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_7 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );
	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_8 ); hwndCheck->SetCheck( BST_UNCHECKED ); hwndCheck->EnableWindow( FALSE );

	return;
}

void CControlPanelDlg::ChannelStatus_PrepareForAll( ioctl_channelProperty_s pProperty[32], ioctl_cardInfo_s::CARD_TYPE type )
{
	switch ( type )
	{
	case ioctl_cardInfo_s::CARD_TYPE_424:
	{
	//Input gain...
		pProperty[0].m_Property = PROPERTY_TYPE_GAIN_ADC;
		pProperty[0].m_SerialNumber = 0;
		pProperty[0].m_Position = PROPERTY_GAIN_10dBv;
		pProperty[0].m_ControlEnabled = PROPERTY_CONTROL_ENABLED;

		//For nicer code below...
		for ( int i = 1; i < 8; i++ )
		{
			pProperty[i].m_Property = PROPERTY_TYPE_UNDEFINED;
			pProperty[i].m_SerialNumber = i;
			pProperty[i].m_Position = PROPERTY_GAIN_10dBv;
			pProperty[i].m_ControlEnabled = PROPERTY_CONTROL_DISABLED;
		}

	//Output gain...
		//For nicer code below...
		for ( int i = 8; i < 16; i++ )
		{
			pProperty[i].m_Property = PROPERTY_TYPE_UNDEFINED;
			pProperty[i].m_SerialNumber = i-8;
			pProperty[i].m_Position = PROPERTY_GAIN_10dBv;
			pProperty[i].m_ControlEnabled = PROPERTY_CONTROL_DISABLED;
		}

	//Monitoring...
		for ( int i = 16; i < 20; i++ )
		{
			pProperty[i].m_Property = PROPERTY_TYPE_MONITORING;
			pProperty[i].m_SerialNumber = i-16;
			pProperty[i].m_Position = PROPERTY_MONITORING_DISABLE;
			pProperty[i].m_ControlEnabled = PROPERTY_CONTROL_ENABLED;
		}

		//For nicer code below...
		for ( int i = 20; i < 24; i++ )
		{
			pProperty[i].m_Property = PROPERTY_TYPE_UNDEFINED;
			pProperty[i].m_SerialNumber = i-16;
			pProperty[i].m_Position = PROPERTY_MONITORING_DISABLE;
			pProperty[i].m_ControlEnabled = PROPERTY_CONTROL_DISABLED;
		}

	//Mute...
		for ( int i = 24; i < 28; i++ )
		{
			pProperty[i].m_Property = PROPERTY_TYPE_MUTE;
			pProperty[i].m_SerialNumber = i-24;
			pProperty[i].m_Position = PROPERTY_MUTE_OFF;
			pProperty[i].m_ControlEnabled = PROPERTY_CONTROL_ENABLED;
		}

		//For nicer code below...
		for ( int i = 28; i < 32; i++ )
		{
			pProperty[i].m_Property = PROPERTY_TYPE_UNDEFINED;
			pProperty[i].m_SerialNumber = i-24;
			pProperty[i].m_Position = PROPERTY_MUTE_OFF;
			pProperty[i].m_ControlEnabled = PROPERTY_CONTROL_DISABLED;
		}
		break;
	}
	case ioctl_cardInfo_s::CARD_TYPE_496:
	{
	//Input gain...
		pProperty[0].m_Property = PROPERTY_TYPE_GAIN_ADC;
		pProperty[0].m_SerialNumber = 0;
		pProperty[0].m_Position = PROPERTY_GAIN_10dBv;
		pProperty[0].m_ControlEnabled = PROPERTY_CONTROL_ENABLED;

		//For nicer code below...
		for ( int i = 1; i < 8; i++ )
		{
			pProperty[i].m_Property = PROPERTY_TYPE_UNDEFINED;
			pProperty[i].m_SerialNumber = i;
			pProperty[i].m_Position = PROPERTY_GAIN_10dBv;
			pProperty[i].m_ControlEnabled = PROPERTY_CONTROL_DISABLED;
		}

	//Output gain...
		pProperty[8].m_Property = PROPERTY_TYPE_GAIN_DAC;
		pProperty[8].m_SerialNumber = 0;
		pProperty[8].m_Position = PROPERTY_GAIN_10dBv;
		pProperty[8].m_ControlEnabled = PROPERTY_CONTROL_ENABLED;

		//For nicer code below...
		for ( int i = 9; i < 16; i++ )
		{
			pProperty[i].m_Property = PROPERTY_TYPE_UNDEFINED;
			pProperty[i].m_SerialNumber = i-8;
			pProperty[i].m_Position = PROPERTY_GAIN_10dBv;
			pProperty[i].m_ControlEnabled = PROPERTY_CONTROL_DISABLED;
		}

	//Monitoring...
		for ( int i = 16; i < 20; i++ )
		{
			pProperty[i].m_Property = PROPERTY_TYPE_MONITORING;
			pProperty[i].m_SerialNumber = i-16;
			pProperty[i].m_Position = PROPERTY_MONITORING_DISABLE;
			pProperty[i].m_ControlEnabled = PROPERTY_CONTROL_ENABLED;
		}

		//For nicer code below...
		for ( int i = 20; i < 24; i++ )
		{
			pProperty[i].m_Property = PROPERTY_TYPE_UNDEFINED;
			pProperty[i].m_SerialNumber = i-16;
			pProperty[i].m_Position = PROPERTY_MONITORING_DISABLE;
			pProperty[i].m_ControlEnabled = PROPERTY_CONTROL_DISABLED;
		}

	//Mute...
		for ( int i = 24; i < 28; i++ )
		{
			pProperty[i].m_Property = PROPERTY_TYPE_MUTE;
			pProperty[i].m_SerialNumber = i-24;
			pProperty[i].m_Position = PROPERTY_MUTE_OFF;
			pProperty[i].m_ControlEnabled = PROPERTY_CONTROL_ENABLED;
		}

		//For nicer code below...
		for ( int i = 28; i < 32; i++ )
		{
			pProperty[i].m_Property = PROPERTY_TYPE_UNDEFINED;
			pProperty[i].m_SerialNumber = i-24;
			pProperty[i].m_Position = PROPERTY_MUTE_OFF;
			pProperty[i].m_ControlEnabled = PROPERTY_CONTROL_DISABLED;
		}
		break;
	}
	case ioctl_cardInfo_s::CARD_TYPE_824:
	{
	//Input gain...
		for ( int i = 0; i < 8; i++ )
		{
			pProperty[i].m_Property = PROPERTY_TYPE_GAIN_ADC;
			pProperty[i].m_SerialNumber = i;
			pProperty[i].m_Position = PROPERTY_GAIN_10dBv;
			pProperty[i].m_ControlEnabled = PROPERTY_CONTROL_ENABLED;
		}

	//Output gain...
		for ( int i = 8; i < 16; i++ )
		{
			pProperty[i].m_Property = PROPERTY_TYPE_GAIN_DAC;
			pProperty[i].m_SerialNumber = i-8;
			pProperty[i].m_Position = PROPERTY_GAIN_10dBv;
			pProperty[i].m_ControlEnabled = PROPERTY_CONTROL_ENABLED;
		}

	//Monitoring...
		for ( int i = 16; i < 24; i++ )
		{
			pProperty[i].m_Property = PROPERTY_TYPE_MONITORING;
			pProperty[i].m_SerialNumber = i-16;
			pProperty[i].m_Position = PROPERTY_MONITORING_DISABLE;
			pProperty[i].m_ControlEnabled = PROPERTY_CONTROL_ENABLED;
		}

	//Mute...
		for ( int i = 24; i < 32; i++ )
		{
			pProperty[i].m_Property = PROPERTY_TYPE_MUTE;
			pProperty[i].m_SerialNumber = i-24;
			pProperty[i].m_Position = PROPERTY_MUTE_OFF;
			pProperty[i].m_ControlEnabled = PROPERTY_CONTROL_ENABLED;
		}
		break;
	}
	}
}

BOOL CControlPanelDlg::ChannelStatus_SetAll(  registry_data_s* pSettings )
{
	BOOL	Status = TRUE;

//Get memory for ioctl buffer.
	unsigned long						size = sizeof(ioctl_channelProperty_header_s) + 32*sizeof(ioctl_channelProperty_s);

	ioctl_channelProperty_header_s*		pHeader = (ioctl_channelProperty_header_s*)new char[size];
	if ( pHeader == NULL )
		return FALSE;

//Set header.
	pHeader->m_Request = PROPERTY_SET_PROPERTY;
	pHeader->m_RequestBy = PROPERTY_REQUEST_BY_GUI;
	//pHeader->m_SerialNumber = pCard->m_SerialNumber;
	pHeader->m_nPropertyDefinitions = 32;

//Set property structures.
	ioctl_channelProperty_s*			pProperty = (ioctl_channelProperty_s*)( pHeader + 1 );

	for ( int i = 0; i < ELEMENTS_IN_ARRAY( pSettings->m_Cards ); i++ )
	{
		registry_data_card_s*		pCard = &pSettings->m_Cards[i];

	//Does card exist?
		if ( m_Cards[i].m_Type == ioctl_cardInfo_s::CARD_TYPE_NOT_SET )
		{
		//Not present... -> ignore any data.
			continue;
		}

	//Is the card the same as in registry data?
		if (
		  ( m_Cards[i].m_Type != (ioctl_cardInfo_s::CARD_TYPE)pCard->m_Type )
		  ||
		  ( m_Cards[i].m_SerialNumber != pCard->m_SerialNumber )
		)
		{
		//Data and driver's cards are not the same... user has changed cards...
			continue;
		}

	//Same types and serial... same card...
		//Set header...
		pHeader->m_SerialNumber = (unsigned char)pCard->m_SerialNumber;

		//Prepare channels...
		ChannelStatus_PrepareForAll( pProperty, m_Cards[i].m_Type );

		//Input gain...
		unsigned char	bit = 0x01;
		for ( int j = 0; j < 8; j++ )
		{
			pProperty[j].m_Position = ( ( pCard->m_Input_Volume & bit ) != 0 ) ? PROPERTY_GAIN_4dBu : PROPERTY_GAIN_10dBv;

		//Shift bit...
			bit <<= 1;
		}

		//Output gain...
		bit = 0x01;
		for ( int j = 8; j < 16; j++ )
		{
			pProperty[j].m_Position = ( ( pCard->m_Output_Volume & bit ) != 0 ) ? PROPERTY_GAIN_4dBu : PROPERTY_GAIN_10dBv;

		//Shift bit...
			bit <<= 1;
		}

		//Monitoring...
		bit = 0x01;
		for ( int j = 16; j < 24; j++ )
		{
			pProperty[j].m_Position = ( ( pCard->m_Monitoring & bit ) != 0 ) ? PROPERTY_MONITORING_ENABLE : PROPERTY_MONITORING_DISABLE;

		//Shift bit...
			bit <<= 1;
		}
		//Also initialize old state.
		//
		//This is needed if ASIO host was opened before GUI. In that case GUI wouldn't know the proper monitoring states.
		//So we will set them to the last recorded values.
		//Now if GUI crashed it could happen that they aren't the last ones set.
		m_Monitoring_oldState[i] = pCard->m_Monitoring;

		//Mute...
		bit = 0x01;
		for ( int j = 24; j < 32; j++ )
		{
			pProperty[j].m_Position = ( ( pCard->m_Mute & bit ) != 0 ) ? PROPERTY_MUTE_ON : PROPERTY_MUTE_OFF;

		//Shift bit...
			bit <<= 1;
		}

	//Get channel properties.
		unsigned long		bytesReturned = 0;

		Status = DeviceIoControl(
		  m_hDeviceIoControl,
		  IOCTL_DATA_GET_SET_CHANNEL_PROPERTIES,
		  pHeader,
		  size,
		  pHeader,
		  size,
		  &bytesReturned,
		  NULL
		);
		if ( Status == FALSE )
		{
			break;
		}
	}

//Free resources...
	delete[] pHeader;

//All ok?
	if ( Status == FALSE )
	{
		MessageBox(
		  "Failed to set channel properties.\n\nInternal error... please contact author about this.",
		  "Error",
		  MB_OK | MB_ICONWARNING | MB_TOPMOST
		);
	}

	return Status;
}

void CControlPanelDlg::ChannelStatus_Set( unsigned char type, unsigned char channelNumber, unsigned char position )
{
//Get selected card.
	ioctl_cardInfo_s*	pCard = Cards_Tree_GetSelected();
	if ( pCard == NULL )
		return;

//Validate if current card supports requested change.
	BOOL	Status = TRUE;
	switch ( pCard->m_Type )
	{
	case ioctl_cardInfo_s::CARD_TYPE_424:
	{
		switch( type )
		{
		case PROPERTY_TYPE_GAIN_ADC:
			if ( channelNumber > 0 )
				Status = FALSE;
			break;
		case PROPERTY_TYPE_GAIN_DAC:
			Status = FALSE;
			break;
		case PROPERTY_TYPE_MONITORING:
		case PROPERTY_TYPE_MUTE:
			if ( channelNumber > 3 )
				Status = FALSE;
			break;
		default:
			Status = FALSE;
		}
		break;
	}
	case ioctl_cardInfo_s::CARD_TYPE_496:
	{
		switch( type )
		{
		case PROPERTY_TYPE_GAIN_ADC:
		case PROPERTY_TYPE_GAIN_DAC:
			if ( channelNumber > 0 )
				Status = FALSE;
			break;
		case PROPERTY_TYPE_MONITORING:
		case PROPERTY_TYPE_MUTE:
			if ( channelNumber > 3 )
				Status = FALSE;
			break;
		default:
			Status = FALSE;
		}
		break;
	}
	case ioctl_cardInfo_s::CARD_TYPE_824:
	{
		switch( type )
		{ 
		case PROPERTY_TYPE_GAIN_ADC:
		case PROPERTY_TYPE_GAIN_DAC:
		case PROPERTY_TYPE_MONITORING:
		case PROPERTY_TYPE_MUTE:
			if ( channelNumber > 7 )
				Status = FALSE;
			break;
		default:
			Status = FALSE;
		}
		break;
	}
	default:
	{
		Status = FALSE;
		break;
	}
	}
	if ( position > 1 )
		Status = FALSE;

	if ( Status == FALSE )
	{
		MessageBox(
		  "Current request couldn't be executed.\n\nUnsupported operation on current card or ControlPanel internal error.",
		  "Error",
		  MB_OK | MB_ICONWARNING | MB_TOPMOST
		);
		return;
	}

//Send request to driver.
	ChannelStatus_SendToDriver( pCard->m_SerialNumber, type, channelNumber, position );

	return;
}

BOOL CControlPanelDlg::ChannelStatus_SendToDriver( unsigned char serialNumber_card, unsigned char type, unsigned char channelNumber, unsigned char position )
{
//Get memory for ioctl buffer.
	unsigned long						size = sizeof(ioctl_channelProperty_header_s) + 1*sizeof(ioctl_channelProperty_s);

	ioctl_channelProperty_header_s*		pHeader = (ioctl_channelProperty_header_s*)new char[size];
	if ( pHeader == NULL )
	{
		MessageBox( "Not enough memory.", "Error", MB_OK | MB_ICONWARNING | MB_TOPMOST );
		return FALSE;
	}

//Set header.
	pHeader->m_Request = PROPERTY_SET_PROPERTY;
	pHeader->m_RequestBy = PROPERTY_REQUEST_BY_GUI;
	pHeader->m_SerialNumber = serialNumber_card;
	pHeader->m_nPropertyDefinitions = 1;

//Set property structures.
	ioctl_channelProperty_s*			pProperty = (ioctl_channelProperty_s*)( pHeader + 1 );

	pProperty[0].m_Property = type;
	pProperty[0].m_SerialNumber = channelNumber;
	pProperty[0].m_Position = position;
	pProperty[0].m_ControlEnabled = PROPERTY_CONTROL_ENABLED;

//Send channel settings to the driver.
	unsigned long	bytesReturned;

	BOOL	Status = DeviceIoControl(
	  m_hDeviceIoControl,
	  IOCTL_DATA_GET_SET_CHANNEL_PROPERTIES,
	  pHeader,
	  size,
	  pHeader,
	  size,
	  &bytesReturned,
	  NULL
	);

//Free resources...
	delete[] pHeader;

//All ok?
	if ( Status == FALSE )
	{
		MessageBox( "Failed to set channel control change.", "Error", MB_OK | MB_ICONWARNING | MB_TOPMOST );
	}

	return Status;
}

void CControlPanelDlg::ChannelStatus_UpdateGUI_ControlEnable( unsigned char serialNumber_card, unsigned char serialNumber_channel, unsigned char controlEnabled )
{
	for ( int i = 0; i < ELEMENTS_IN_ARRAY( m_Cards ); i++ )
	{
	//Update samplerate text if cards have the same serialNumber.
		if (
		  ( m_Cards[i].m_Type != ioctl_cardInfo_s::CARD_TYPE_NOT_SET )
		  &&
		  ( m_Cards[i].m_SerialNumber == serialNumber_card )
		)
		{
		//Monitoring...
			unsigned long	checkButton_ID = 0;
			unsigned char	bit;

			switch ( serialNumber_channel )
			{
			case 0: checkButton_ID = IDC_MON_1; bit = 0x01; break;
			case 1: checkButton_ID = IDC_MON_2; bit = 0x02; break;
			case 2: checkButton_ID = IDC_MON_3; bit = 0x04; break;
			case 3: checkButton_ID = IDC_MON_4; bit = 0x08; break;
			case 4: checkButton_ID = IDC_MON_5; bit = 0x10; break;
			case 5: checkButton_ID = IDC_MON_6; bit = 0x20; break;
			case 6: checkButton_ID = IDC_MON_7; bit = 0x40; break;
			case 7: checkButton_ID = IDC_MON_8; bit = 0x80; break;
			default:
				break;
			}

		//Update control.
			if ( checkButton_ID != 0 )
			{
				CButton*		hwndCheck;

				hwndCheck = (CButton*)GetDlgItem( checkButton_ID );
				hwndCheck->EnableWindow( controlEnabled );

			//Was ASIO channel opened or closed?
				if ( controlEnabled == PROPERTY_CONTROL_DISABLED )
				{
				//It was opened -> save the old checkBox state and disable monitoring.
					if ( BST_UNCHECKED == hwndCheck->GetCheck() )
					{
					//Unchecked...
						m_Monitoring_oldState[i] &= ~bit;
					}
					else
					{
					//Checked...
						m_Monitoring_oldState[i] |= bit;
					}

				//Remove check state...
				//
				//Notice:
				//ASIO driver will reset all monitoring flags after openeing the ASIO channels,
				//so the check state's of the buttons is out of the scope of this procedure.
					//hwndCheck->SetCheck( BST_UNCHECKED );
				}
				else
				{
				//It was closed -> set the old checkBox state back to what it was.
					BOOL	Status = ChannelStatus_SendToDriver(
					  serialNumber_card,
					  PROPERTY_TYPE_MONITORING,
					  serialNumber_channel,
					  ( ( m_Monitoring_oldState[i] & bit ) == 0 ) ? PROPERTY_MONITORING_DISABLE : PROPERTY_MONITORING_ENABLE
					);
					if ( Status == TRUE )
					{
					//Set current check state .
						hwndCheck->SetCheck( ( ( m_Monitoring_oldState[i] & bit ) == 0 ) ? BST_UNCHECKED : BST_CHECKED );
					}
				}
			}
			break;
		}
	}
}

void CControlPanelDlg::ChannelStatus_UpdateGUI_Monitoring( unsigned char serialNumber_card, unsigned char serialNumber_channel, unsigned char monitoring )
{
	for ( int i = 0; i < ELEMENTS_IN_ARRAY( m_Cards ); i++ )
	{
	//Update samplerate text if cards have the same serialNumber.
		if (
		  ( m_Cards[i].m_Type != ioctl_cardInfo_s::CARD_TYPE_NOT_SET )
		  &&
		  ( m_Cards[i].m_SerialNumber == serialNumber_card )
		)
		{
		//Monitoring...
			unsigned long	checkButton_ID = 0;

			switch ( serialNumber_channel )
			{
			case 0: checkButton_ID = IDC_MON_1; break;
			case 1: checkButton_ID = IDC_MON_2; break;
			case 2: checkButton_ID = IDC_MON_3; break;
			case 3: checkButton_ID = IDC_MON_4; break;
			case 4: checkButton_ID = IDC_MON_5; break;
			case 5: checkButton_ID = IDC_MON_6; break;
			case 6: checkButton_ID = IDC_MON_7; break;
			case 7: checkButton_ID = IDC_MON_8; break;
			default:
				break;
			}

		//Update control.
			if ( checkButton_ID != 0 )
			{
				CButton*		hwndCheck;

				hwndCheck = (CButton*)GetDlgItem( checkButton_ID );
				hwndCheck->SetCheck( monitoring );
			}
			break;
		}
	}
}

