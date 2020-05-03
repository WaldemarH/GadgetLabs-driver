#include "main.h"
#include "dlg_main.h"


void CControlPanelDlg::OnBnClickedMute1()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_1 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_MUTE, 0, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_MUTE_ON : PROPERTY_MUTE_OFF );
}

void CControlPanelDlg::OnBnClickedMute2()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_2 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_MUTE, 1, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_MUTE_ON : PROPERTY_MUTE_OFF );
}

void CControlPanelDlg::OnBnClickedMute3()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_3 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_MUTE, 2, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_MUTE_ON : PROPERTY_MUTE_OFF );
}

void CControlPanelDlg::OnBnClickedMute4()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_4 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_MUTE, 3, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_MUTE_ON : PROPERTY_MUTE_OFF );
}

void CControlPanelDlg::OnBnClickedMute5()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_5 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_MUTE, 4, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_MUTE_ON : PROPERTY_MUTE_OFF );
}

void CControlPanelDlg::OnBnClickedMute6()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_6 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_MUTE, 5, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_MUTE_ON : PROPERTY_MUTE_OFF );
}

void CControlPanelDlg::OnBnClickedMute7()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_7 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_MUTE, 6, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_MUTE_ON : PROPERTY_MUTE_OFF );
}

void CControlPanelDlg::OnBnClickedMute8()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_MUTE_8 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_MUTE, 7, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_MUTE_ON : PROPERTY_MUTE_OFF );
}
