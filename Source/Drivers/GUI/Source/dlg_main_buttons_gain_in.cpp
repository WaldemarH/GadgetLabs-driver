#include "main.h"
#include "dlg_main.h"

void CControlPanelDlg::OnBnClickedInGain1()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_1 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_GAIN_ADC, 0, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_GAIN_4dBu : PROPERTY_GAIN_10dBv );
}

void CControlPanelDlg::OnBnClickedInGain2()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_2 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_GAIN_ADC, 1, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_GAIN_4dBu : PROPERTY_GAIN_10dBv );
}

void CControlPanelDlg::OnBnClickedInGain3()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_3 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_GAIN_ADC, 2, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_GAIN_4dBu : PROPERTY_GAIN_10dBv );
}

void CControlPanelDlg::OnBnClickedInGain4()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_4 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_GAIN_ADC, 3, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_GAIN_4dBu : PROPERTY_GAIN_10dBv );
}

void CControlPanelDlg::OnBnClickedInGain5()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_5 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_GAIN_ADC, 4, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_GAIN_4dBu : PROPERTY_GAIN_10dBv );
}

void CControlPanelDlg::OnBnClickedInGain6()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_6 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_GAIN_ADC, 5, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_GAIN_4dBu : PROPERTY_GAIN_10dBv );
}

void CControlPanelDlg::OnBnClickedInGain7()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_7 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_GAIN_ADC, 6, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_GAIN_4dBu : PROPERTY_GAIN_10dBv );
}

void CControlPanelDlg::OnBnClickedInGain8()
{
	CButton*	hwndCheck = (CButton*)GetDlgItem( IDC_IN_GAIN_8 );
	if ( hwndCheck != NULL )
		ChannelStatus_Set( PROPERTY_TYPE_GAIN_ADC, 7, ( BST_CHECKED == hwndCheck->GetCheck() ) ? PROPERTY_GAIN_4dBu : PROPERTY_GAIN_10dBv );
}
