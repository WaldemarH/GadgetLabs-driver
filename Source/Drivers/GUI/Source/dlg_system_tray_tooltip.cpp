#include "main.h"
#include "dlg_system_tray.h"

void CTrayDialog::Tray_SetToolTip( LPCTSTR lpszToolTip )
{
//Validate...
	ASSERT(strlen(lpszToolTip) > 0 && strlen(lpszToolTip) < 64);

//Set tooltip text.
	strcpy_s( m_Icon_NotificationData.szTip, sizeof( m_Icon_NotificationData.szTip ), lpszToolTip );
	m_Icon_NotificationData.uFlags |= NIF_TIP;
}
