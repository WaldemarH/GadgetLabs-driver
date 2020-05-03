#include "main.h"
#include "dlg_main.h"


void CControlPanelDlg::OnBnClickedMon1()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_MON_1 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_MONITORING, 0, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_MONITORING_ENABLE : PROPERTY_MONITORING_DISABLE );
}

void CControlPanelDlg::OnBnClickedMon2()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_MON_2 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_MONITORING, 1, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_MONITORING_ENABLE : PROPERTY_MONITORING_DISABLE );
}

void CControlPanelDlg::OnBnClickedMon3()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_MON_3 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_MONITORING, 2, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_MONITORING_ENABLE : PROPERTY_MONITORING_DISABLE );
}

void CControlPanelDlg::OnBnClickedMon4()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_MON_4 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_MONITORING, 3, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_MONITORING_ENABLE : PROPERTY_MONITORING_DISABLE );
}

void CControlPanelDlg::OnBnClickedMon5()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_MON_5 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_MONITORING, 4, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_MONITORING_ENABLE : PROPERTY_MONITORING_DISABLE );
}

void CControlPanelDlg::OnBnClickedMon6()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_MON_6 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_MONITORING, 5, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_MONITORING_ENABLE : PROPERTY_MONITORING_DISABLE );
}

void CControlPanelDlg::OnBnClickedMon7()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_MON_7 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_MONITORING, 6, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_MONITORING_ENABLE : PROPERTY_MONITORING_DISABLE );
}

void CControlPanelDlg::OnBnClickedMon8()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_MON_8 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_MONITORING, 7, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_MONITORING_ENABLE : PROPERTY_MONITORING_DISABLE );
}
