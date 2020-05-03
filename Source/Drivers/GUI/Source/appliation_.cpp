#include "main.h"
#include "application.h"
#include "dlg_main.h"


BEGIN_MESSAGE_MAP(CApplication, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

const UINT			GWM_REFRESH = ::RegisterWindowMessage( _T( "{E1A1ECDE-1294-4d91-B28A-E93458C9D8DE}" ) );
CApplication		GL_GUI;


CApplication::CApplication()
{
//Initialize variables...
	m_hMutex = NULL;
}

CApplication::~CApplication()
{
}

BOOL CApplication::InitInstance()
{
//Is this installation 'RunOnce' request?
//
//Why is this made like this?
//inf installation will wait until 'RunOnce' request it will return.
//So at 'RunOnce' we'll create another proces and return... with this GUI will be started and inf setup will continue.
	if ( TRUE == IsInstallationRequest() )
	{
		return TRUE;
	}

//Check if current application is already started.
	if ( FALSE == Mutex_Lock() )
	{
	//Already started -> send refresh request to opened GUI.
		if ( GWM_REFRESH != 0 )
		{
			PostMessage( HWND_BROADCAST, GWM_REFRESH, 0, 0 );
		}

	//Exit...
		return TRUE;
	}

//Start application.
	//InitCommonControls() is required on Windows XP if an application manifest specifies
	//use of ComCtl32.dll version 6 or later to enable visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	//Start application...
	CWinApp::InitInstance();

//Create main window(in our case dialog) and display it.
//
//Notice:
//it wont exit until user or system shutdown will request that.
	CControlPanelDlg	dlg;
	m_pMainWnd = &dlg;

	dlg.DoModal();

//Unlock mutex.
	Mutex_Unlock();

	return TRUE;
}
