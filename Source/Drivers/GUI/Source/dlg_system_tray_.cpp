#include "main.h"
#include "dlg_system_tray.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CTrayDialog, CDialog)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SYSCOMMAND()
	ON_MESSAGE( WM_TRAY_ICON_NOTIFY_MESSAGE, Tray_OnNotify )
END_MESSAGE_MAP()

CTrayDialog::CTrayDialog( UINT uIDD, CWnd* pParent ) : CDialog(uIDD, pParent)
{
//Initialize variables...
	//Icon...
	m_Icon_IsVisible = FALSE;

	ZeroMemory( &m_Icon_NotificationData, sizeof( m_Icon_NotificationData ) );

	m_Icon_NotificationData.cbSize = sizeof(NOTIFYICONDATA);
	//m_Icon_NotificationData.hWnd = 0;
	m_Icon_NotificationData.uID = 1;
	m_Icon_NotificationData.uFlags = NIF_MESSAGE;
	m_Icon_NotificationData.uCallbackMessage = WM_TRAY_ICON_NOTIFY_MESSAGE;
	//m_Icon_NotificationData.hIcon = 0;
	//m_Icon_NotificationData.szTip[0] = 0;

	m_Mouse_IsButtonDown = false;
	m_Window_TopMost = NULL;
}

int CTrayDialog::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
//Call base class.
	if ( CDialog::OnCreate( lpCreateStruct ) == -1 )
		return -1;
	
//Update NOTIFYICONDATA structure.
	m_Icon_NotificationData.hWnd = this->m_hWnd;
	m_Icon_NotificationData.uID = 1;
	
	return 0;
}

void CTrayDialog::OnDestroy()
{
	CDialog::OnDestroy();

//Destroy system tray icon.
	if (
	  ( m_Icon_NotificationData.hWnd != NULL )
	  &&
	  ( m_Icon_NotificationData.uID > 0 )
	  &&
	  ( TRUE == Tray_IsVisible() )
	)
	{
	//Dialog is closing -> destroy system tray icon.
		Shell_NotifyIcon( NIM_DELETE, &m_Icon_NotificationData );
	}
}

void CTrayDialog::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (
	  ( ( nID & 0xFFF0 ) == SC_MINIMIZE )
	  &&
	  ( TRUE == Tray_Show() )
	)
	{
	//Minimize was pressed -> hide dialog.
		this->ShowWindow( SW_HIDE );
	}
	else
	{
	//Not interesting to us...
		CDialog::OnSysCommand( nID, lParam );
	}
}

