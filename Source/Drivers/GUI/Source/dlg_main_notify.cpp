#include "main.h"
#include "dlg_main.h"
#include "..\..\Kernel\Source\driver_ioctl_gui.h"


void CControlPanelDlg::Notification_Release()
{
//Protect m_Notification_Event variable.
	EnterCriticalSection( &m_Notification_CriticalSection );

//Do the honky tonk.
	if ( m_Notification_Event != NULL )
	{
	//Release notification event.
		unsigned long		bytesReturned = 0;

		DeviceIoControl(
		  m_hDeviceIoControl,
		  IOCTL_GUI_RELEASE_EVENT_NOTIFICATION,
		  NULL,
		  0,
		  NULL,
		  0,
		  &bytesReturned,
		  NULL
		);

	//Stop notification thread.
		m_Notification_Stop = TRUE;
		SetEvent( m_Notification_Event );

	//Was thread stopped?
		if ( m_Notification_Thread != NULL )
		{
		//Wait for thread to finish.
			Sleep( 100 );

		//Check if thread has finished.
			DWORD	threadTerminationStatus = -1;

			BOOL	Status = GetExitCodeThread( m_Notification_Thread, &threadTerminationStatus );
			if ( Status == FALSE )
			{
			//Something is wrong -> try to stop the thread again.
				SetEvent( m_Notification_Event );

			//Give it enough time to finish.
				Sleep( 200 );
			}
		}

	//Close notification event.
		CloseHandle( m_Notification_Event );
		m_Notification_Event = NULL;
	}
	if ( m_Notification_Thread != NULL )
	{
		CloseHandle( m_Notification_Thread );
		m_Notification_Thread = NULL;
	}

//Leave protection.
	LeaveCriticalSection( &m_Notification_CriticalSection );
}

BOOL CControlPanelDlg::Notification_Set()
{
	BOOL	Status = TRUE;

//Protect m_Notification_Event variable.
	EnterCriticalSection( &m_Notification_CriticalSection );

//Set notification if not set already.
	if ( m_Notification_Event == NULL )
	{
	//Create event.
		m_Notification_Event = CreateEvent( NULL, FALSE, FALSE, NULL );
		if ( m_Notification_Event == NULL )
		{
			Status = FALSE;
		}

	//Set notification.
		if ( Status == TRUE )
		{
			unsigned long		bytesReturned = 0;
			__int64				hEvent = (__int64)m_Notification_Event;		//this must work for 32 and 64 bit systems

			Status = DeviceIoControl(
			  m_hDeviceIoControl,
			  IOCTL_GUI_SET_EVENT_NOTIFICATION,
			  &hEvent,
			  sizeof( hEvent ),
			  NULL,
			  0,
			  &bytesReturned,
			  NULL
			);
		}

	//Set notification thread.
		if ( Status == TRUE )
		{
			m_Notification_Thread = CreateThread( NULL, 0, ( LPTHREAD_START_ROUTINE )CControlPanelDlg::Notification_Thread, this, 0, 0 );
			if ( m_Notification_Thread == NULL )
			{
				Status = FALSE;
			}
		}
	}

//Leave protection.
	LeaveCriticalSection( &m_Notification_CriticalSection );

//All ok?
	if ( Status == FALSE )
	{
		MessageBox(
		  "Failed to set notication events.\n\nInternal error... please contact author about this.",
		  "Error",
		  MB_OK | MB_ICONWARNING | MB_TOPMOST
		);
	}

	return Status;
}

DWORD WINAPI CControlPanelDlg::Notification_Thread( CControlPanelDlg* pDialog )
{
	while ( 1 )
	{
	//Wait for event to fire.
		WaitForSingleObject( pDialog->m_Notification_Event, INFINITE );

	//Should we stop the notification thread?
		if ( pDialog->m_Notification_Stop == TRUE )
		{
			break;
		}

	//Get notification event.
		ioctl_notify_event_s		notification;
		notification.m_Type = ioctl_notify_event_s::EVENT_TYPE_NO_EVENT;

		do
		{
			unsigned long			bytesReturned = 0;

			BOOL Status = DeviceIoControl(
			  pDialog->m_hDeviceIoControl,
			  IOCTL_GUI_GET_NOTIFICATION_EVENT,
			  NULL,
			  0,
			  &notification,
			  sizeof( notification ),
			  &bytesReturned,
			  NULL
			);
			if ( Status == FALSE )
			{
				notification.m_Type = ioctl_notify_event_s::EVENT_TYPE_NO_EVENT;
			}

		//Update notification.
			switch ( notification.m_Type )
			{
			case ioctl_notify_event_s::EVENT_TYPE_SAMPLE_RATE_CHANGED:
			{
				pDialog->Cards_Tree_Update_SampleRate( notification.m_Data.m_SampleRate.m_Card_serialNumber, notification.m_Data.m_SampleRate.m_SampleRate );
				break;
			}
			case ioctl_notify_event_s::EVENT_TYPE_CHANNEL_PROPERTY_CONTROL_ENABLED:
			{
				pDialog->ChannelStatus_UpdateGUI_ControlEnable(
				  notification.m_Data.m_ChannelProperty.m_Card_serialNumber,
				  notification.m_Data.m_ChannelProperty.m_Channel_serialNumber,
				  notification.m_Data.m_ChannelProperty.m_Property
				);
				break;
			}
			case ioctl_notify_event_s::EVENT_TYPE_CHANNEL_PROPERTY_MONITOR:
			{
				pDialog->ChannelStatus_UpdateGUI_Monitoring(
				  notification.m_Data.m_ChannelProperty.m_Card_serialNumber,
				  notification.m_Data.m_ChannelProperty.m_Channel_serialNumber,
				  notification.m_Data.m_ChannelProperty.m_Property
				);
				break;
			}
			case ioctl_notify_event_s::EVENT_TYPE_CHANNEL_NUMBER_CHANGED:
			{
				pDialog->Cards_Tree_Update_OpenedChannels(
				  notification.m_Data.m_ChannelsOpened.m_Card_serialNumber,
				  notification.m_Data.m_ChannelsOpened.m_Channels_opened_INs_ASIO,
				  notification.m_Data.m_ChannelsOpened.m_Channels_opened_OUTs_ASIO,
				  notification.m_Data.m_ChannelsOpened.m_Channels_opened_INs_AVStream,
				  notification.m_Data.m_ChannelsOpened.m_Channels_opened_OUTs_AVStream
				);
				break;
			}
			}
		}
		while ( notification.m_Type != ioctl_notify_event_s::EVENT_TYPE_NO_EVENT );
	}

	return 0;
}



