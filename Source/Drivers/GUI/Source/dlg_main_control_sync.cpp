#include "main.h"
#include "dlg_main.h"


//Procedure will send a sync cards request to the driver.
//It will sync all the cards that have sync cable installed and properly working.
//
//return TRUE/FALSE
BOOL CControlPanelDlg::SyncCards( unsigned long sync )
{
//How many cards are installed?
	unsigned long	count = 0;
	for ( int i = 0; i < ELEMENTS_IN_ARRAY( m_Cards ); i++ )
	{
		if ( m_Cards[i].m_Type != ioctl_cardInfo_s::CARD_TYPE_NOT_SET )
		{
			++count;
		}
	}
	if ( count <= 1 )
	{
	//Nothing to sync/desync...
		return TRUE;
	}

	//MessageBox( ( sync == TRUE ) ? "SyncCards -> Sync." : "SyncCards -> Unsync.", "Error", MB_OK | MB_TOPMOST );

//Send sync request to driver.
	unsigned long		bytesReturned;

	BOOL	Status = DeviceIoControl(
	  m_hDeviceIoControl,
	  IOCTL_DATA_SET_SYNC_CARDS,
	  &sync,
	  sizeof( sync ),
	  NULL,
	  0,
	  &bytesReturned,
	  NULL
	);
	//Special case for system shutdown.
	if ( m_System_shutDown == TRUE )
	{
		return Status;
	}

	if ( Status == TRUE )
	{
		m_Synced = sync;

	//Update menu item.
		CMenu*		pMenu = GetMenu();
		if ( pMenu != NULL )
		{
			pMenu->CheckMenuItem( ID_SYNC_CARDS, ( ( sync == FALSE ) ? MF_UNCHECKED : MF_CHECKED ) | MF_BYCOMMAND );
		}
		else
		{
			MessageBox(
			  "Internal error occured -> please contact author about this.",
			  "Error",
			  MB_OK | MB_ICONWARNING | MB_TOPMOST
			);

			Status = FALSE;
		}
	}
	else
	{
		MessageBox(
			"Failed to change synhronization.\n\nPossible problems:\n - channels are opened... close them and try again\n - sync between cards was not detected.",
		  "Error",
		  MB_OK | MB_ICONWARNING | MB_TOPMOST
		);
	}

//Update tree with data.
	if ( Status == TRUE )
	{
		Status = Cards_RefreshInfo();
		if ( Status == FALSE )
		{
			MessageBox(
			  "Failed to refresh cards info.",
			  "Error",
			  MB_OK | MB_ICONWARNING | MB_TOPMOST
			);
		}
	}

//Set tree.
	if ( Status == TRUE )
	{
		Status = Cards_Tree_Set();
		if ( Status == FALSE )
		{
			MessageBox(
			  "Failed to refresh tree.",
			  "Error",
			  MB_OK | MB_ICONWARNING | MB_TOPMOST
			);
		}
	}

	return Status;
}

void CControlPanelDlg::SyncCard_EnableMenu()
{
//How many cards are installed?
	unsigned long	count = 0;
	for ( int i = 0; i < ELEMENTS_IN_ARRAY( m_Cards ); i++ )
	{
		if ( m_Cards[i].m_Type != ioctl_cardInfo_s::CARD_TYPE_NOT_SET )
		{
			++count;
		}
	}

//Enable/Disable menu item.
	CMenu*		pMenu = GetMenu();
	if ( pMenu != NULL )
	{
		if ( count > 1 )
		{
			pMenu->EnableMenuItem( ID_SYNC_CARDS, MF_ENABLED | MF_BYCOMMAND );
		}
		else
		{
			pMenu->EnableMenuItem( ID_SYNC_CARDS, MF_DISABLED | MF_GRAYED | MF_BYCOMMAND );
		}
	}
	else
	{
		MessageBox(
		  "Internal error occured -> please contact author about this.",
		  "Error",
		  MB_OK | MB_ICONWARNING | MB_TOPMOST
		);
	}
}
