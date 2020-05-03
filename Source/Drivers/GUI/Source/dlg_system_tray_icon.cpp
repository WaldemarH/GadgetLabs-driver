#include "main.h"
#include "dlg_system_tray.h"


BOOL CTrayDialog::Tray_IsVisible()
{
	return m_Icon_IsVisible;
}

void CTrayDialog::Tray_SetIcon( HICON hIcon )
{
//Validate.
	ASSERT(hIcon);

//Set icon.
	m_Icon_NotificationData.hIcon = hIcon;
	m_Icon_NotificationData.uFlags |= NIF_ICON;
}

void CTrayDialog::Tray_SetIcon(UINT nResourceID)
{
//Validate.
	ASSERT( nResourceID > 0 );

//Set icon.
	HICON	hIcon = AfxGetApp()->LoadIcon(nResourceID);
	if ( hIcon != NULL )
	{
		m_Icon_NotificationData.hIcon = hIcon;
		m_Icon_NotificationData.uFlags |= NIF_ICON;
	}
	else
	{
		TRACE0("FAILED TO LOAD ICON\n");
	}
}

void CTrayDialog::Tray_SetIcon( LPCTSTR lpszResourceName )
{
//Set icon.
	HICON	hIcon = AfxGetApp()->LoadIcon( lpszResourceName );
	if ( hIcon != NULL )
	{
		m_Icon_NotificationData.hIcon = hIcon;
		m_Icon_NotificationData.uFlags |= NIF_ICON;
	}
	else
	{
		TRACE0("FAILED TO LOAD ICON\n");
	}
}

BOOL CTrayDialog::Tray_Hide()
{
	BOOL	bSuccess = TRUE;

	if ( m_Icon_IsVisible == TRUE )
	{
		bSuccess = Shell_NotifyIcon( NIM_DELETE, &m_Icon_NotificationData );
		if ( bSuccess )
		{
			m_Icon_IsVisible= FALSE;
		}
	}

	return bSuccess;
}

BOOL CTrayDialog::Tray_Show()
{
	BOOL	bSuccess = TRUE;

	if ( m_Icon_IsVisible == FALSE )
	{
		bSuccess = Shell_NotifyIcon( NIM_ADD, &m_Icon_NotificationData );
		if ( bSuccess )
		{
			m_Icon_IsVisible= TRUE;
		}
	}

	return bSuccess;
}

BOOL CTrayDialog::Tray_Update()
{
	BOOL	bSuccess = FALSE;

	if ( m_Icon_IsVisible == TRUE )
	{
		bSuccess = Shell_NotifyIcon( NIM_MODIFY, &m_Icon_NotificationData );
	}

	return bSuccess;
}
