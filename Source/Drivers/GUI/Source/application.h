#pragma once

//Includes...


class CApplication : public CWinApp
{
	DECLARE_MESSAGE_MAP()

//Constructor/Destructor...
	public: CApplication();
	public: ~CApplication();

	public: virtual BOOL InitInstance();

//Installation...
	private: BOOL IsInstallationRequest();

//Mutex...
	private: BOOL Mutex_Lock();
	private: void Mutex_Unlock();
	
	HANDLE				m_hMutex;

	#define CPA_MUTEX_NAME		"{DF00BD89-AE64-4635-9244-00CDACF7B32C}"
};
