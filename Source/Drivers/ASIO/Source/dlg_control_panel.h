#pragma once

//Includes...
#include "..\..\Kernel\Source\driver_ioctl.h"
#include "..\..\Kernel\Source\driver_ioctl_data.h"


//This ControlPanel communicates with ASIO driver through registry and it's completely indepeneded and multy client safe.
class dlg_controlPanel_c
{
//Messages...
	public: static INT_PTR CALLBACK DialogProc( __in HWND hwndDlg, __in UINT uMsg, __in WPARAM wParam, __in LPARAM lParam );

//Initialize dialog.
	protected: static void Initialize_Cards( HWND hwndDlg, ioctl_cardInfo_s cards[IOCTL_MAX_CARDS] );
	protected: static BOOL Initialize_Channels( HWND hwndDlg, ioctl_cardInfo_s cards[IOCTL_MAX_CARDS] );
	protected: static BOOL Initialize_Components( HWND hwndDlg );

//Driver communication.
	protected: static BOOL GetData_Cards( HWND hwndDlg, ioctl_cardInfo_s cards[IOCTL_MAX_CARDS] );

//Save.
	protected: static BOOL Save_Channels( HWND hwndDlg, ioctl_cardInfo_s cards[IOCTL_MAX_CARDS] );
};