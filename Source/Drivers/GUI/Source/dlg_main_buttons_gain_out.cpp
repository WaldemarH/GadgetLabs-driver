#include "main.h"
#include "dlg_main.h"


void CControlPanelDlg::OnBnClickedOutGain1()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_1 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_GAIN_DAC, 0, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_GAIN_4dBu : PROPERTY_GAIN_10dBv );
}

void CControlPanelDlg::OnBnClickedOutGain2()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_2 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_GAIN_DAC, 1, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_GAIN_4dBu : PROPERTY_GAIN_10dBv );
}

void CControlPanelDlg::OnBnClickedOutGain3()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_3 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_GAIN_DAC, 2, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_GAIN_4dBu : PROPERTY_GAIN_10dBv );
}

void CControlPanelDlg::OnBnClickedOutGain4()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_4 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_GAIN_DAC, 3, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_GAIN_4dBu : PROPERTY_GAIN_10dBv );
}

void CControlPanelDlg::OnBnClickedOutGain5()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_5 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_GAIN_DAC, 4, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_GAIN_4dBu : PROPERTY_GAIN_10dBv );
}

void CControlPanelDlg::OnBnClickedOutGain6()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_6 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_GAIN_DAC, 5, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_GAIN_4dBu : PROPERTY_GAIN_10dBv );
}

void CControlPanelDlg::OnBnClickedOutGain7()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_7 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_GAIN_DAC, 6, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_GAIN_4dBu : PROPERTY_GAIN_10dBv );
}

void CControlPanelDlg::OnBnClickedOutGain8()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_OUT_GAIN_8 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_GAIN_DAC, 7, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_GAIN_4dBu : PROPERTY_GAIN_10dBv );
}
