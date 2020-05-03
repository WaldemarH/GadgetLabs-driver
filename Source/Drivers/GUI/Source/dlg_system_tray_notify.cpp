#include "main.h"
#include "dlg_system_tray.h"

LRESULT CTrayDialog::Tray_OnNotify( WPARAM wParam, LPARAM lParam )
{
	UINT	uID = (UINT)wParam;
	if ( uID != 1 )
		return 0;
	
	CPoint pt;

	switch ( (UINT)lParam )
	{
	case WM_MOUSEMOVE:
		GetCursorPos( &pt );
		ClientToScreen( &pt );
		Tray_OnNotify_MouseMove( pt );
		break;

	case WM_LBUTTONDOWN:
		GetCursorPos( &pt );
		ClientToScreen( &pt );
		Tray_OnNotify_LButtonDown( pt );
		break;
	case WM_LBUTTONUP:
		GetCursorPos( &pt );
		ClientToScreen( &pt );
		Tray_OnNotify_LButtonUp( pt );
		break;
	case WM_LBUTTONDBLCLK:
		GetCursorPos( &pt );
		ClientToScreen( &pt );
		Tray_OnNotify_LButtonDblClk( pt );
		break;
	
	case WM_RBUTTONDOWN:
		GetCursorPos( &pt );
		ClientToScreen( &pt );
		Tray_OnNotify_RButtonDown( pt );
		break;
	case WM_RBUTTONUP:
	case WM_CONTEXTMENU:
		GetCursorPos( &pt );
		//ClientToScreen( &pt );
		Tray_OnNotify_RButtonUp( pt );
		break;
	case WM_RBUTTONDBLCLK:
		GetCursorPos( &pt );
		ClientToScreen( &pt );
		Tray_OnNotify_RButtonDblClk( pt );
		break;
	}
	return 0;
}

void CTrayDialog::Tray_OnNotify_MouseMove(CPoint pt)
{
//Get top most window, before user presses any button.
	if ( m_Mouse_IsButtonDown == false )
	{
		m_Window_TopMost = ::GetForegroundWindow();
	}
}

void CTrayDialog::Tray_OnNotify_LButtonDown( CPoint pt )
{
	m_Mouse_IsButtonDown = true;
}

void CTrayDialog::Tray_OnNotify_LButtonUp( CPoint pt )
{
//Show if GUI is not topmost window/Hide if it is.
	if (
	  ( TRUE == IsWindowVisible() )
	  &&
	  ( m_Window_TopMost == this->m_hWnd )
	)
	{
	//Window is topmost -> hide.
		ShowWindow( SW_HIDE );
	}
	else
	{
	//Window is not topmost -> show it.
		this->SetForegroundWindow();
		this->SetWindowPos( &CWnd::wndTop, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE );
		SetActiveWindow();
	}

	m_Mouse_IsButtonDown = false;
}

void CTrayDialog::Tray_OnNotify_LButtonDblClk( CPoint pt )
{
	this->SetForegroundWindow();
	this->SetWindowPos( &CWnd::wndTop, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE );
	SetActiveWindow();
}

void CTrayDialog::Tray_OnNotify_RButtonDown( CPoint pt )
{
	m_Mouse_IsButtonDown = true;
}

void CTrayDialog::Tray_OnNotify_RButtonUp( CPoint pt )
{
	CMenu*		pMenu = m_Menu.GetSubMenu(0);
	if ( pMenu != NULL )
	{
		this->SetForegroundWindow();
		pMenu->TrackPopupMenu( TPM_BOTTOMALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, this );
		pMenu->SetDefaultItem( 0, TRUE );
	}

	m_Mouse_IsButtonDown = false;
}

void CTrayDialog::Tray_OnNotify_RButtonDblClk(CPoint pt)
{
}

