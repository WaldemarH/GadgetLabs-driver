#include "main.h"
#include "dlg_system_tray.h"


BOOL CTrayDialog::Tray_SetMenu( UINT nResourceID, UINT nDefaultPos )
{
	return m_Menu.LoadMenu( nResourceID );
}

BOOL CTrayDialog::Tray_SetMenu( LPCTSTR lpszMenuName, UINT nDefaultPos )
{
	return m_Menu.LoadMenu( lpszMenuName );
}

BOOL CTrayDialog::Tray_SetMenu( HMENU hMenu, UINT nDefaultPos )
{
	m_Menu.Attach( hMenu );
	return TRUE;
}
