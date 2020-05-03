#pragma once

class CTrayDialog : public CDialog
{
	DECLARE_MESSAGE_MAP()

//Constructor/Destructor...
	public: CTrayDialog( UINT uIDD, CWnd* pParent = NULL );

	protected: afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
	protected: afx_msg void OnDestroy();
	protected: afx_msg void OnSysCommand( UINT nID, LPARAM lParam );

//Icon...
	public: void Tray_SetIcon( HICON hIcon );
	public: void Tray_SetIcon( UINT nResourceID );
	public: void Tray_SetIcon( LPCTSTR lpszResourceName );

	public: BOOL Tray_Hide();
	public: BOOL Tray_Show();
	public: BOOL Tray_Update();

	public: BOOL Tray_IsVisible();

	private: BOOL				m_Icon_IsVisible;
	private: NOTIFYICONDATA		m_Icon_NotificationData;

//Menu...
	public: BOOL Tray_SetMenu( UINT nResourceID, UINT nDefaultPos = 0 );
	public: BOOL Tray_SetMenu( HMENU hMenu, UINT nDefaultPos = 0 );
	public: BOOL Tray_SetMenu( LPCTSTR lpszMenuName, UINT nDefaultPos = 0 );

	private: CMenu				m_Menu;

//Notify...
	private: afx_msg LRESULT Tray_OnNotify( WPARAM wParam, LPARAM lParam );

	protected: virtual void Tray_OnNotify_LButtonDown( CPoint pt );
	protected: virtual void Tray_OnNotify_LButtonUp( CPoint pt );
	protected: virtual void Tray_OnNotify_LButtonDblClk( CPoint pt );
	protected: virtual void Tray_OnNotify_RButtonDown( CPoint pt );
	protected: virtual void Tray_OnNotify_RButtonUp( CPoint pt );
	protected: virtual void Tray_OnNotify_RButtonDblClk( CPoint pt );

	protected: virtual void Tray_OnNotify_MouseMove( CPoint pt );

	private: bool				m_Mouse_IsButtonDown;
	private: HWND				m_Window_TopMost;

	#define WM_TRAY_ICON_NOTIFY_MESSAGE			( WM_USER + 1 )

//Tooltip...
	public: void Tray_SetToolTip( LPCTSTR lpszToolTip );
};
