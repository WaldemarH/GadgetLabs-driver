#include "main.h"
#include "dlg_main.h"

void CControlPanelDlg::OnBnClickedSync()
{
	CMenu*			pMenu = GetMenu();
	if ( pMenu != NULL )
	{
		if ( 0 == ( MF_CHECKED & pMenu->GetMenuState( ID_SYNC_CARDS, MF_BYCOMMAND ) ) )
		{
		//Currently unsynced -> sync.
			SyncCards( TRUE );
		}
		else
		{
		//Currently synced -> unsync.
			SyncCards( FALSE );
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