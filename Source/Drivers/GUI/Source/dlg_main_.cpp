#include "main.h"
#include "dlg_main.h"
#include "..\..\Kernel\Source\driver_ioctl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP( CControlPanelDlg, CTrayDialog )
	ON_WM_CLOSE()
	ON_WM_QUERYENDSESSION()
	ON_WM_WINDOWPOSCHANGING()

	ON_BN_CLICKED(ID_FILE_MINIMIZE_TO_SYSTEM_TRAY, OnBnClickedMinimizeToTray)
	ON_BN_CLICKED(ID_FILE_EXIT, OnBnClickedExit)
	ON_BN_CLICKED(ID_BUFFERSIZE_32SAMPLES, OnBnClickedBufferSize32)
	ON_BN_CLICKED(ID_BUFFERSIZE_64SAMPLES, OnBnClickedBufferSize64)
	ON_BN_CLICKED(ID_BUFFERSIZE_128SAMPLES, OnBnClickedBufferSize128)
	ON_BN_CLICKED(ID_BUFFERSIZE_256SAMPLES, OnBnClickedBufferSize256)
	ON_BN_CLICKED(ID_BUFFERSIZE_512SAMPLES, OnBnClickedBufferSize512)
	ON_BN_CLICKED(ID_BUFFERSIZE_1024SAMPLES, OnBnClickedBufferSize1024)
	ON_BN_CLICKED(ID_SYNC_CARDS, OnBnClickedSync)
	ON_BN_CLICKED(ID_ABOUT_ABOUT, OnBnClickedAbout)

	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE, OnTreeSelectionChanged)
	ON_BN_CLICKED(IDC_IN_GAIN_1, OnBnClickedInGain1)
	ON_BN_CLICKED(IDC_IN_GAIN_2, OnBnClickedInGain2)
	ON_BN_CLICKED(IDC_IN_GAIN_3, OnBnClickedInGain3)
	ON_BN_CLICKED(IDC_IN_GAIN_4, OnBnClickedInGain4)
	ON_BN_CLICKED(IDC_IN_GAIN_5, OnBnClickedInGain5)
	ON_BN_CLICKED(IDC_IN_GAIN_6, OnBnClickedInGain6)
	ON_BN_CLICKED(IDC_IN_GAIN_7, OnBnClickedInGain7)
	ON_BN_CLICKED(IDC_IN_GAIN_8, OnBnClickedInGain8)
	ON_BN_CLICKED(IDC_OUT_GAIN_1, OnBnClickedOutGain1)
	ON_BN_CLICKED(IDC_OUT_GAIN_2, OnBnClickedOutGain2)
	ON_BN_CLICKED(IDC_OUT_GAIN_3, OnBnClickedOutGain3)
	ON_BN_CLICKED(IDC_OUT_GAIN_4, OnBnClickedOutGain4)
	ON_BN_CLICKED(IDC_OUT_GAIN_5, OnBnClickedOutGain5)
	ON_BN_CLICKED(IDC_OUT_GAIN_6, OnBnClickedOutGain6)
	ON_BN_CLICKED(IDC_OUT_GAIN_7, OnBnClickedOutGain7)
	ON_BN_CLICKED(IDC_OUT_GAIN_8, OnBnClickedOutGain8)
	ON_BN_CLICKED(IDC_MON_1, OnBnClickedMon1)
	ON_BN_CLICKED(IDC_MON_2, OnBnClickedMon2)
	ON_BN_CLICKED(IDC_MON_3, OnBnClickedMon3)
	ON_BN_CLICKED(IDC_MON_4, OnBnClickedMon4)
	ON_BN_CLICKED(IDC_MON_5, OnBnClickedMon5)
	ON_BN_CLICKED(IDC_MON_6, OnBnClickedMon6)
	ON_BN_CLICKED(IDC_MON_7, OnBnClickedMon7)
	ON_BN_CLICKED(IDC_MON_8, OnBnClickedMon8)
	ON_BN_CLICKED(IDC_MUTE_1, OnBnClickedMute1)
	ON_BN_CLICKED(IDC_MUTE_2, OnBnClickedMute2)
	ON_BN_CLICKED(IDC_MUTE_3, OnBnClickedMute3)
	ON_BN_CLICKED(IDC_MUTE_4, OnBnClickedMute4)
	ON_BN_CLICKED(IDC_MUTE_5, OnBnClickedMute5)
	ON_BN_CLICKED(IDC_MUTE_6, OnBnClickedMute6)
	ON_BN_CLICKED(IDC_MUTE_7, OnBnClickedMute7)
	ON_BN_CLICKED(IDC_MUTE_8, OnBnClickedMute8)

	ON_BN_CLICKED(ID_SYSTEMTRAYMENU_OPEN, OnBnClickedSystemTrayOpen)
	ON_BN_CLICKED(ID_SYSTEMTRAYMENU_EXIT, OnBnClickedSystemTrayExit)

	ON_NOTIFY_EX( TTN_NEEDTEXT, 0, OnTooltip_NeedText )

	ON_REGISTERED_MESSAGE( GWM_REFRESH, DialogBox_FullRefresh )
END_MESSAGE_MAP()


CControlPanelDlg::CControlPanelDlg( CWnd* pParent ): CTrayDialog( CControlPanelDlg::IDD, pParent )
{
//Initialize variables...
	//Constructor/Desctructor...
	m_System_saved = FALSE;
	m_System_shutDown = FALSE;

	m_hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
	m_Visible = 0;

	//About...
	m_RequestDisplayed = FALSE;

	//Ioctl...
	m_hDeviceIoControl = INVALID_HANDLE_VALUE;
	m_hDeviceIoControl_1 = INVALID_HANDLE_VALUE;
	m_hDeviceIoControl_2 = INVALID_HANDLE_VALUE;
	m_hDeviceIoControl_3 = INVALID_HANDLE_VALUE;
	m_hDeviceIoControl_4 = INVALID_HANDLE_VALUE;

	//Cards...
	ZeroMemory( m_Cards, sizeof( m_Cards ) );
	m_Cards_Tree_lastSelectedCard = NULL;
	ZeroMemory( m_Cards_Tree_openedChannels, sizeof( m_Cards_Tree_openedChannels ) );
	ZeroMemory( m_Cards_Tree_openedChannels_type, sizeof( m_Cards_Tree_openedChannels_type ) );
	ZeroMemory( m_Cards_Tree_sampleRate, sizeof( m_Cards_Tree_sampleRate ) );

	//Channels...
	ZeroMemory( m_Monitoring_oldState, sizeof( m_Monitoring_oldState ) );

	//Notification...
	InitializeCriticalSection( &m_Notification_CriticalSection );
	m_Notification_Event = NULL;
	m_Notification_Stop = FALSE;
	m_Notification_Thread = NULL;

	//Protecting IRP...
	m_ProtectingIRP_hDeviceIoControl = NULL;
	ZeroMemory( &m_ProtectingIRP_overllaped, sizeof( m_ProtectingIRP_overllaped ) );

	//Sync...
	m_Synced = FALSE;
}

BOOL CControlPanelDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

//Set dialog icon.
	if ( m_hIcon != NULL )
	{
	//Set big and small icon.
		SetIcon( m_hIcon, TRUE );
		SetIcon( m_hIcon, FALSE );
	}

//ToolTip.
	EnableToolTips( TRUE );

//Set system tray stuff.
	Tray_SetIcon( m_hIcon );
	Tray_SetToolTip( "Gadgetlabs WavePro Series" );
	Tray_SetMenu( IDR_SYSTEM_TRAY_MENU );
	Tray_Show();

//Get handle to driver.
	m_hDeviceIoControl_1 = GetDeviceIoControlHeader( &DEVICE_IO_GUID_1, FALSE );
	m_hDeviceIoControl_2 = GetDeviceIoControlHeader( &DEVICE_IO_GUID_2, FALSE );
	m_hDeviceIoControl_3 = GetDeviceIoControlHeader( &DEVICE_IO_GUID_3, FALSE );
	m_hDeviceIoControl_4 = GetDeviceIoControlHeader( &DEVICE_IO_GUID_4, FALSE );

	m_hDeviceIoControl = GetDeviceIoControl();
	if ( m_hDeviceIoControl == INVALID_HANDLE_VALUE )
	{
	//Failed...
		MessageBox(
		  "Failed to get driver handle.\nDo you have Gadgetlabs driver installed?",
		  "Error",
		  MB_OK | MB_ICONWARNING | MB_TOPMOST
		);
		EndDialog(0);
		return TRUE;
	}

//Test engine versions.
	if ( FALSE == TestIoEngineVersions() )
	{
	//Wrong engine version.
		EndDialog(0);
		return TRUE;
	}

//Set up protecting IRP.
	if ( FALSE == ProtectingIRP_Set() )
	{
	//Failed...
		EndDialog(0);
		return TRUE;
	}

//Initialize dialogBox.
	SendMessage( GWM_REFRESH, 0, 0 );

	return TRUE;  // return TRUE unless you set the focus to a control
}

LRESULT CControlPanelDlg::DialogBox_FullRefresh( WPARAM wParam, LPARAM lParam )
{
//This message is send when another instance of GUI tries to start.
//
//Why is this used?
//On mutlicard installation the driver installs the GUI for the first card already,
//so when the second,third,... card gets installed the GUI is already present and a new
//one wont be opened.
//
//But the first GUI only has data for 1 card, so we need to inform the first GUI that it's
//data is old and that it should refresh it self.

//Remove notification if it's set.
	Notification_Release();

//Load cards data.
	if ( FALSE == Cards_RefreshInfo() )
	{
	//Failed...
		EndDialog(0);	//force termination
		return TRUE;
	}

//Set tree.
	if ( FALSE == Cards_Tree_Set() )
	{
	//Failed...
		EndDialog(0);	//force termination
		return TRUE;
	}

//Get bufferSize.
	if ( FALSE == BufferSize_Get() )
	{
	//Failed...
		EndDialog(0);	//force termination
		return TRUE;
	}

//Enable SyncCard Menu.
	SyncCard_EnableMenu();

//Load settings from registry and apply them to the cards.
	registry_data_s		settings;
	registry_c			registry;

	if ( TRUE == registry.Read( &settings ) )
	{
	//Was about request already displayed?
		m_RequestDisplayed = settings.m_RequestDisplayed;
		if ( m_RequestDisplayed == FALSE )
		{
			SetWindowText( "GadgetLabs Control Panel                           Please check Help->About dialog." );
		}

	//Set new bufferSize.
		BufferSize_Set( settings.m_BufferSize );

	//Sync cards.
		//SyncCards( settings.m_Synced ); -> disabled as Win7 opens channels at start and so sync fails -> need to come up with some idea how to make it work

	//Set channel properties.
		ChannelStatus_SetAll( &settings );

	//Get current setting from driver... set GUI.
		ChannelStatus_GetAll();
		m_Cards_Tree_lastSelectedCard = Cards_Tree_GetSelected();
	}

//Set notifier.
	if ( FALSE == Notification_Set() )
	{
	//Failed...
		EndDialog(0);	//force termination
		return TRUE;
	}

	return TRUE;
}

void CControlPanelDlg::SaveAndFreeResources()
{
//Save GUI properties.
	if ( m_System_saved == FALSE )
	{
		m_System_saved = TRUE;

	//Get GUI settings...
		registry_data_s		settings;

		settings.m_RequestDisplayed = m_RequestDisplayed;

		settings.m_BufferSize = (unsigned short)BufferSize_GetValue();
		settings.m_Synced = (unsigned short)m_Synced;

		if ( TRUE == ChannelStatus_GetAll_GetValue( &settings ) )
		{
			registry_c		registry;
			registry.Save( &settings );
		}

	//Desync cards...
		SyncCards( FALSE );

	//Stop notify thread.
		Notification_Release();

	//Remove protecting IRP.
		if ( m_ProtectingIRP_hDeviceIoControl != NULL )
		{
			ProtectingIRP_Remove();
		}

	//Close interface to driver.
		if ( m_hDeviceIoControl_4 != INVALID_HANDLE_VALUE )
		{
			CloseHandle( m_hDeviceIoControl_4 );
			m_hDeviceIoControl_4 = INVALID_HANDLE_VALUE;
		}
		if ( m_hDeviceIoControl_3 != INVALID_HANDLE_VALUE )
		{
			CloseHandle( m_hDeviceIoControl_3 );
			m_hDeviceIoControl_3 = INVALID_HANDLE_VALUE;
		}
		if ( m_hDeviceIoControl_2 != INVALID_HANDLE_VALUE )
		{
			CloseHandle( m_hDeviceIoControl_2 );
			m_hDeviceIoControl_2 = INVALID_HANDLE_VALUE;
		}
		if ( m_hDeviceIoControl_1 != INVALID_HANDLE_VALUE )
		{
			CloseHandle( m_hDeviceIoControl_1 );
			m_hDeviceIoControl_1 = INVALID_HANDLE_VALUE;
		}
	}
}

void CControlPanelDlg::OnClose()
{
//Is system in shut down mode or did user close the GUI.
	if (
	  ( m_System_shutDown == FALSE )
	  &&
	  ( m_Synced == TRUE )
	)
	{
		HRESULT		result = MessageBox(
		  "GUI can not be closed while cards are synced.\nIf you will proceed the cards will be desynced\n(even if channels are playing).\n\nContinue?",
		  "Error",
		  MB_YESNO | MB_ICONWARNING | MB_TOPMOST
		);
		if ( result == IDNO )
		{
		//Don't close the application.
			return;
		}
	}

//Save and free...
	SaveAndFreeResources();

//Close dialog.
	CDialog::OnClose();
}

BOOL CControlPanelDlg::OnQueryEndSession()
{
	m_System_shutDown = TRUE;

//Windows 7:
//When system is shutting down or logging off user this is the only message that is send, so we better save things here...
//But the code can handle normal behaviour too...
	SaveAndFreeResources();

	return TRUE;
}

void CControlPanelDlg::OnWindowPosChanging( WINDOWPOS* lpwndpos )
{
//Do not allow dialog to show... needed to hide it at start.
	if ( m_Visible < 3 )
	{
	//Hide window.
		lpwndpos->flags &= ~SWP_SHOWWINDOW;

	//Call base class...
		CTrayDialog::OnWindowPosChanging( lpwndpos );
		
	//Increase counter.
		++m_Visible;
	}
	else
	{
	//Call base class...
		CTrayDialog::OnWindowPosChanging( lpwndpos );
	}
}
