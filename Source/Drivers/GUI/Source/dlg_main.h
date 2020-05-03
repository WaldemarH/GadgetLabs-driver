#pragma once

//Includes...
#include <winioctl.h>
#include "dlg_system_tray.h"
#include "registry.h"

extern HANDLE GetDeviceIoControlHeader( const GUID* interfaceGuid, BOOL overlapped );


class CControlPanelDlg : public CTrayDialog
{
	enum { IDD = IDD_CONTROLPANEL_DIALOG };
	DECLARE_MESSAGE_MAP()

//Constructor/Desctructor...
	public: CControlPanelDlg( CWnd* pParent = NULL );

	//protected: virtual void OnClose();
	public: virtual BOOL OnInitDialog();

	protected: afx_msg void OnClose();
	protected: afx_msg BOOL OnQueryEndSession();
	protected: afx_msg void OnWindowPosChanging( WINDOWPOS* lpwndpos );

	protected: void SaveAndFreeResources();

	protected: BOOL						m_System_saved;
	protected: BOOL						m_System_shutDown;

	protected: HICON					m_hIcon;
	protected: unsigned long			m_Visible;

//About...
	protected: afx_msg void OnBnClickedAbout();

	protected: unsigned long			m_RequestDisplayed;

//Application communication...
	protected: afx_msg LRESULT DialogBox_FullRefresh( WPARAM wParam, LPARAM lParam );

//BufferSize...
	protected: afx_msg void OnBnClickedBufferSize32();
	protected: afx_msg void OnBnClickedBufferSize64();
	protected: afx_msg void OnBnClickedBufferSize128();
	protected: afx_msg void OnBnClickedBufferSize256();
	protected: afx_msg void OnBnClickedBufferSize512();
	protected: afx_msg void OnBnClickedBufferSize1024();

	protected: BOOL BufferSize_Get( void );
	protected: BOOL BufferSize_Set( unsigned long bufferSize );

	protected: unsigned long BufferSize_GetValue();

//Cards...
	protected: afx_msg void OnTreeSelectionChanged( NMHDR *pNMHDR, LRESULT *pResult );

	protected: BOOL Cards_RefreshInfo( void );

	protected: ioctl_cardInfo_s* Cards_Tree_GetSelected( void );
	protected: BOOL Cards_Tree_Set( void );
	protected: void Cards_Tree_Update_ClockSource();
	protected: void Cards_Tree_Update_OpenedChannels( unsigned long serialNumber, unsigned char ASIO_input, unsigned char ASIO_output, unsigned char WDM_input, unsigned char WDM_output );
	protected: void Cards_Tree_Update_SampleRate( unsigned long serialNumber, unsigned long sampleRate );

	protected: ioctl_cardInfo_s			m_Cards[IOCTL_MAX_CARDS];
	protected: ioctl_cardInfo_s*		m_Cards_Tree_lastSelectedCard;
	protected: HTREEITEM				m_Cards_Tree_openedChannels[IOCTL_MAX_CARDS];
	protected: HTREEITEM				m_Cards_Tree_openedChannels_type[IOCTL_MAX_CARDS][2];	//one for ASIO and one for WDM
	protected: HTREEITEM				m_Cards_Tree_sampleRate[IOCTL_MAX_CARDS];

//Channels...
	protected: BOOL ChannelStatus_GetAll( void );
	protected: BOOL ChannelStatus_SetAll( registry_data_s* pSettings );
	protected: void ChannelStatus_PrepareForAll( ioctl_channelProperty_s pProperty[32], ioctl_cardInfo_s::CARD_TYPE type );

	protected: void ChannelStatus_Set( unsigned char type, unsigned char channelNumber, unsigned char position );
	protected: BOOL ChannelStatus_SendToDriver( unsigned char serialNumber_card, unsigned char type, unsigned char channelNumber, unsigned char position );
	protected: void ChannelStatus_UpdateGUI_ControlEnable( unsigned char serialNumber_card, unsigned char serialNumber_channel, unsigned char controlEnabled );
	protected: void ChannelStatus_UpdateGUI_Monitoring( unsigned char serialNumber_card, unsigned char serialNumber_channel, unsigned char monitoring );

	protected: void ChannelStatus_DisableAll();

	protected: BOOL ChannelStatus_GetAll_GetValue( registry_data_s* pSettings );


	protected: unsigned char			m_Monitoring_oldState[IOCTL_MAX_CARDS];

//Check buttons...
	protected: afx_msg void OnBnClickedInGain1();
	protected: afx_msg void OnBnClickedInGain2();
	protected: afx_msg void OnBnClickedInGain3();
	protected: afx_msg void OnBnClickedInGain4();
	protected: afx_msg void OnBnClickedInGain5();
	protected: afx_msg void OnBnClickedInGain6();
	protected: afx_msg void OnBnClickedInGain7();
	protected: afx_msg void OnBnClickedInGain8();

	protected: afx_msg void OnBnClickedOutGain1();
	protected: afx_msg void OnBnClickedOutGain2();
	protected: afx_msg void OnBnClickedOutGain3();
	protected: afx_msg void OnBnClickedOutGain4();
	protected: afx_msg void OnBnClickedOutGain5();
	protected: afx_msg void OnBnClickedOutGain6();
	protected: afx_msg void OnBnClickedOutGain7();
	protected: afx_msg void OnBnClickedOutGain8();

	protected: afx_msg void OnBnClickedMon1();
	protected: afx_msg void OnBnClickedMon2();
	protected: afx_msg void OnBnClickedMon3();
	protected: afx_msg void OnBnClickedMon4();
	protected: afx_msg void OnBnClickedMon5();
	protected: afx_msg void OnBnClickedMon6();
	protected: afx_msg void OnBnClickedMon7();
	protected: afx_msg void OnBnClickedMon8();

	protected: afx_msg void OnBnClickedMute1();
	protected: afx_msg void OnBnClickedMute2();
	protected: afx_msg void OnBnClickedMute3();
	protected: afx_msg void OnBnClickedMute4();
	protected: afx_msg void OnBnClickedMute5();
	protected: afx_msg void OnBnClickedMute6();
	protected: afx_msg void OnBnClickedMute7();
	protected: afx_msg void OnBnClickedMute8();

//File menu...
	protected: afx_msg void OnBnClickedMinimizeToTray();
	protected: afx_msg void OnBnClickedExit();

//Ioctl...
	protected: HANDLE GetDeviceIoControl();
	protected: BOOL TestIoEngineVersions( void );

	protected: HANDLE					m_hDeviceIoControl;
	protected: HANDLE					m_hDeviceIoControl_1;
	protected: HANDLE					m_hDeviceIoControl_2;
	protected: HANDLE					m_hDeviceIoControl_3;
	protected: HANDLE					m_hDeviceIoControl_4;

//Notify...
	protected: void Notification_Release();
	protected: BOOL Notification_Set();

	public: static DWORD WINAPI CControlPanelDlg::Notification_Thread( CControlPanelDlg* pDialog );

	protected: CRITICAL_SECTION			m_Notification_CriticalSection;
	protected: HANDLE					m_Notification_Event;
	protected: BOOL						m_Notification_Stop;
	protected: HANDLE					m_Notification_Thread;

//Protecting IRP...
	protected: void ProtectingIRP_Remove();
	protected: BOOL ProtectingIRP_Set();

	protected: HANDLE					m_ProtectingIRP_hDeviceIoControl;
	protected: OVERLAPPED				m_ProtectingIRP_overllaped;

//Sync...
	protected: afx_msg void OnBnClickedSync();

	protected: BOOL SyncCards( unsigned long sync );	//it's BOOL, but it's like this because of IOCTL_DATA_SET_SYNC_CARDS
	protected: void SyncCard_EnableMenu();

	protected: unsigned long			m_Synced;

//System tray...
	protected: afx_msg void OnBnClickedSystemTrayOpen();
	protected: afx_msg void OnBnClickedSystemTrayExit();

//Tooltips...
	protected: afx_msg BOOL OnTooltip_NeedText( UINT id, NMHDR* pNMHDR, LRESULT* pResult );
};
