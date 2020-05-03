#include "main.h"
#include "dlg_main.h"
#include "dlg_about.h"

void CControlPanelDlg::OnBnClickedAbout()
{
//Show About dialog was requested... remove the request form the main dialog tittle.
	if ( m_RequestDisplayed == FALSE )
	{
		SetWindowText( "GadgetLabs Control Panel" );
		m_RequestDisplayed = TRUE;
	}

//Show About dialog.
	CAboutDlg	dlgAbout;
	dlgAbout.DoModal();
}