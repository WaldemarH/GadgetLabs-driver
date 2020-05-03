#include "main.h"
#include "dlg_main.h"
#include "..\..\Kernel\Source\driver_ioctl_data.h"

//Procedure will refresh pCards tables.
//Return: TRUE/FALSE
BOOL CControlPanelDlg::Cards_RefreshInfo( void )
{
//Read data from driver.
	unsigned long	bytesReturned;

	BOOL Status = DeviceIoControl(
	  m_hDeviceIoControl,
	  IOCTL_DATA_GET_CARDS_INFORMATION,
	  NULL,
	  0,
	  &m_Cards,
	  sizeof( m_Cards ),
	  &bytesReturned,
	  NULL
	);
	if ( Status == FALSE )
	{
		MessageBox(
		  "Failed to refresh cards data.",
		  "Error",
		  MB_OK | MB_ICONWARNING | MB_TOPMOST
		);
	}

	return Status;
}

ioctl_cardInfo_s* CControlPanelDlg::Cards_Tree_GetSelected( void )
{
//Get selected card.
	CTreeCtrl*			hwndTree = (CTreeCtrl*)GetDlgItem( IDC_TREE );
	if ( hwndTree == NULL )
	{
		MessageBox(
		  "Internal error occured -> please contact author about this.",
		  "Error",
		  MB_OK | MB_ICONWARNING | MB_TOPMOST
		);
		return NULL;
	}

//Was selected parent or child?
	HTREEITEM			hSelected = hwndTree->GetSelectedItem();
	if ( hSelected == NULL )
	{
		//MessageBox(
		//  "Internal error occured -> please contact author about this.",
		//  "Error",
		//  MB_OK | MB_ICONWARNING | MB_TOPMOST
		//);
		return NULL;
	}

	//Check what was selected... the card item itself or it's child text.
	ioctl_cardInfo_s*	pCard = NULL;
	for ( int i = 0; i < 3; i++ )
	{
	//Get item data.
		pCard = (ioctl_cardInfo_s*)hwndTree->GetItemData( hSelected );
		if ( pCard != NULL )
		{
			break;
		}

	//One level up....
		hSelected = hwndTree->GetParentItem( hSelected );
		if ( hSelected == NULL )
		{
		//There is no parent available (we had parent the first time around).
			break;
		}
	}

	return pCard;	//it can be NULL
}

BOOL CControlPanelDlg::Cards_Tree_Set( void )
{
//Reset variables.
	ZeroMemory( m_Cards_Tree_sampleRate, sizeof( m_Cards_Tree_sampleRate ) );

//Get tree.
	CTreeCtrl*		hwndTree = (CTreeCtrl*)GetDlgItem( IDC_TREE );
	if ( hwndTree == NULL )
	{
		MessageBox( "Internal error occured -> please contact author about this.", "Error", MB_OK | MB_ICONWARNING | MB_TOPMOST );
		return FALSE;
	}

//Reset tree.
	hwndTree->SetRedraw( FALSE );
	hwndTree->DeleteAllItems();
	hwndTree->SetRedraw( TRUE );

	hwndTree->InvalidateRect( NULL );
	hwndTree->UpdateWindow();

//Set cards.
	TCHAR			buffer[64];
	HTREEITEM		hItem;
	HTREEITEM		hItem_first = NULL;
	HTREEITEM		hItem_opened;

	for ( int i = 0; i < ELEMENTS_IN_ARRAY( m_Cards ); i++ )
	{
		if ( m_Cards[i].m_Type != ioctl_cardInfo_s::CARD_TYPE_NOT_SET )
		{
		//Got a card -> add it to tree.
			switch ( m_Cards[i].m_Type )
			{
			case ioctl_cardInfo_s::CARD_TYPE_424:
				_stprintf( buffer, _T("Gadgetlabs 424 (ver: %d)"), m_Cards[i].m_Type_subVersion );
				hItem = hwndTree->InsertItem( buffer );
				break;
			case ioctl_cardInfo_s::CARD_TYPE_496:
				_stprintf( buffer, _T("Gadgetlabs 496 (ver: %d)"), m_Cards[i].m_Type_subVersion );
				hItem = hwndTree->InsertItem( buffer );
				break;
			case ioctl_cardInfo_s::CARD_TYPE_824:
				_stprintf( buffer, _T("Gadgetlabs 824 (ver: %d)"), m_Cards[i].m_Type_subVersion );
				hItem = hwndTree->InsertItem( buffer );
				break;
			default:
				MessageBox( "Internal error occured -> please contact author about this.", "Error", MB_OK | MB_ICONWARNING | MB_TOPMOST );
				return FALSE;
			}
			hwndTree->SetItemData( hItem, (DWORD_PTR)&m_Cards[i] );
			if ( hItem_first == NULL )
				hItem_first = hItem;

		//Set cards data.
			//Set clock source.
			switch ( m_Cards[i].m_ClockSource )
			{
			case ioctl_cardInfo_s::CLOCK_SOURCE_CARD:
				hwndTree->InsertItem( _T("Clock: local (on card)"), hItem );
				break;
			case ioctl_cardInfo_s::CLOCK_SOURCE_EXTERN:
				hwndTree->InsertItem( _T("Clock: sync cable"), hItem );
				break;
			default :
				hwndTree->InsertItem( _T("Clock: source unknown"), hItem );
				break;
			}

			//Has external clock?
			if ( m_Cards[i].m_SerialNumber_masterCard == 0 )
			{
			//It's a master card.
				hwndTree->InsertItem( _T("External clock: no"), hItem );
			}
			else
			{
			//It's a slave card.
				hwndTree->InsertItem( _T("External clock: yes"), hItem );
			}

			//Set sampleRate.
			switch ( m_Cards[i].m_SampleRate )
			{
			case 44100:
				m_Cards_Tree_sampleRate[i] = hwndTree->InsertItem( _T("Sample Rate: 44.1kHz"), hItem );
				break;
			case 48000:
				m_Cards_Tree_sampleRate[i] = hwndTree->InsertItem( _T("Sample Rate: 48kHz"), hItem );
				break;
			case 88200:
				m_Cards_Tree_sampleRate[i] = hwndTree->InsertItem( _T("Sample Rate: 88.2kHz"), hItem );
				break;
			case 96000:
				m_Cards_Tree_sampleRate[i] = hwndTree->InsertItem( _T("Sample Rate: 96kHz"), hItem );
				break;
			default:
				m_Cards_Tree_sampleRate[i] = hwndTree->InsertItem( _T("Sample Rate: unknown"), hItem );
				break;
			}

			//Channels opened.
			hItem_opened = hwndTree->InsertItem( _T("Channels opened:"), hItem );
			m_Cards_Tree_openedChannels[i] = hItem_opened;

			_stprintf( buffer, "ASIO: in: %d, out: %d", m_Cards[i].m_Channels_opened_INs_ASIO, m_Cards[i].m_Channels_opened_OUTs_ASIO );
			m_Cards_Tree_openedChannels_type[i][1] = hwndTree->InsertItem( buffer, hItem_opened );

			_stprintf( buffer, "WDM: in: %d, out: %d", m_Cards[i].m_Channels_opened_INs_AVStream, m_Cards[i].m_Channels_opened_OUTs_AVStream );
			m_Cards_Tree_openedChannels_type[i][2] = hwndTree->InsertItem( buffer, hItem_opened );
		}
		else
		{
			hwndTree->InsertItem( _T("Card Not Installed") );
		}
	}

//Select first item.
	if ( hItem_first == NULL )
		return FALSE;

	m_Cards_Tree_lastSelectedCard = NULL;
	hwndTree->SelectItem( hItem_first );
	return TRUE;
}

void CControlPanelDlg::Cards_Tree_Update_OpenedChannels
(
	unsigned long	serialNumber,
	unsigned char	ASIO_input,
	unsigned char	ASIO_output,
	unsigned char	WDM_input,
	unsigned char	WDM_output
)
{
//Get tree.
	CTreeCtrl*			hwndTree = (CTreeCtrl*)GetDlgItem( IDC_TREE );
	if ( hwndTree == NULL )
		return;

//Update number of channels opened.
	if (
	  ( m_Cards[serialNumber].m_Type != ioctl_cardInfo_s::CARD_TYPE_NOT_SET )
	  &&
	  ( m_Cards[serialNumber].m_SerialNumber == serialNumber )
	  &&
	  ( m_Cards_Tree_openedChannels_type[serialNumber][1] != NULL )
	  &&
	  ( m_Cards_Tree_openedChannels_type[serialNumber][2] != NULL )
	)
	{
	//Update text.
		TCHAR		buffer[64];

		_stprintf( buffer, "ASIO: in: %d, out: %d", ASIO_input, ASIO_output );
		hwndTree->SetItemText( m_Cards_Tree_openedChannels_type[serialNumber][1], buffer );

		_stprintf( buffer, "WDM: in: %d, out: %d", WDM_input, WDM_output );
		hwndTree->SetItemText( m_Cards_Tree_openedChannels_type[serialNumber][2], buffer );
	}

	return;
}

void CControlPanelDlg::Cards_Tree_Update_SampleRate( unsigned long serialNumber, unsigned long sampleRate )
{
//Get tree.
	CTreeCtrl*			hwndTree = (CTreeCtrl*)GetDlgItem( IDC_TREE );
	if ( hwndTree == NULL )
		return;

//Update sampleRate.
	for ( int i = 0; i < ELEMENTS_IN_ARRAY( m_Cards ); i++ )
	{
	//Update samplerate text if cards have the same serialNumber or if cards are synced.
		if (
		  ( m_Cards[i].m_Type != ioctl_cardInfo_s::CARD_TYPE_NOT_SET )
		  &&
		  (
		    ( m_Cards[i].m_SerialNumber == serialNumber )
		    ||
		    ( m_Synced == TRUE )
		  )
		)
		{
			m_Cards[i].m_SampleRate = sampleRate;

		//Update text.
			switch ( sampleRate )
			{
			case 44100:
				hwndTree->SetItemText( m_Cards_Tree_sampleRate[i], _T("Sample Rate: 44.1kHz") );
				break;
			case 48000:
				hwndTree->SetItemText( m_Cards_Tree_sampleRate[i], _T("Sample Rate: 48kHz") );
				break;
			case 88200:
				hwndTree->SetItemText( m_Cards_Tree_sampleRate[i], _T("Sample Rate: 88.2kHz") );
				break;
			case 96000:
				hwndTree->SetItemText( m_Cards_Tree_sampleRate[i], _T("Sample Rate: 96kHz") );
				break;
			default:
				hwndTree->SetItemText( m_Cards_Tree_sampleRate[i], _T("Sample Rate: unknown") );
				break;
			}
			break;
		}
	}

	return;
}
