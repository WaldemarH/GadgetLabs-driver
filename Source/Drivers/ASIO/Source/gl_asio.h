#pragma once

//Includes...
#include "..\..\Kernel\Source\driver_ioctl_asio.h"
#include "..\..\Kernel\Source\driver_ioctl_data.h"


__declspec(align(1)) struct channelInfo_s
{
//Card...
	unsigned short		m_SerialNumber_card;		//card's zero based serial number in m_Cards

//Channel...
	unsigned short		m_SerialNumber_channel;		//stereo channel's zero based serial number

//Left channel.
	ASIOChannelInfo		m_ChannelInfo_left;
	unsigned long*		m_pBuffer_A_left;
	unsigned long*		m_pBuffer_B_left;

//Right channel.
	ASIOChannelInfo		m_ChannelInfo_right;
	unsigned long*		m_pBuffer_A_right;
	unsigned long*		m_pBuffer_B_right;
};

struct registry_data_s;

class glAsio_c : public IASIO, public CUnknown
{
//Constructor/Destructor
	public: glAsio_c( LPUNKNOWN pUnk, HRESULT* phr );
	public: ~glAsio_c();

	protected: BOOL					m_Status_initialization;

//COM Object.
	public: DECLARE_IUNKNOWN

	//Factory method
	public: static CUnknown* CreateInstance( LPUNKNOWN pUnk, HRESULT* phr );

	//IUnknown
	public: virtual HRESULT STDMETHODCALLTYPE NonDelegatingQueryInterface( REFIID riid, void** ppvObject );

//ASIO buffers...
	public: virtual ASIOError getBufferSize( long* pMinSize, long* pMaxSize, long* pPreferredSize, long* pGranularity );

	public: virtual ASIOError createBuffers( ASIOBufferInfo* pBufferInfos, long numChannels, long bufferSize, ASIOCallbacks* pCallbacks );
	public: virtual ASIOError disposeBuffers( void );

	ASIOCallbacks*					m_pHostCallbacks;
	ioctl_asio_start_header_s*		m_pStart;

//ASIO channels...
	public: virtual ASIOError getChannelInfo( ASIOChannelInfo* pInfo );
	public: virtual ASIOError getChannels( long* pNumInputChannels, long* pNumOutputChannels );

	protected: BOOL SetChannels( registry_data_s* pData );

	protected: unsigned long		m_nChannels_INs;			//number of opened stereo channels
	protected: channelInfo_s*		m_pChannels_INs;

	protected: unsigned long		m_nChannels_OUTs;			//number of opened stereo channels
	protected: channelInfo_s*		m_pChannels_OUTs;

//ASIO clock...
	public: virtual ASIOError getClockSources( ASIOClockSource* pClocks, long* pNumSources );
	public: virtual ASIOError setClockSource( long index );

	public: virtual ASIOError getLatencies( long* pInputLatency, long* pOutputLatency );
	public: virtual ASIOError getSamplePosition( ASIOSamples* pSPosition, ASIOTimeStamp* pTStamp );

	protected: void SetLatencies( registry_data_s* pData );

	protected: unsigned long		m_Latency_IN;
	protected: unsigned long		m_Latency_OUT;

//ASIO common stuff...
	public: virtual ASIOBool init( void* pSysHandle );

	public: virtual void getDriverName( char* pName );
	public: virtual long getDriverVersion( void );
	public: virtual void getErrorMessage( char* pString );

	public: virtual ASIOError controlPanel( void );
	public: virtual ASIOError future( long selector, void* pOpt );

	private: char					m_ErrorMessage[128];
	private: BOOL					m_ErrorMessage_showControlPanel;

//ASIO sample rate...
	public: virtual ASIOError canSampleRate( ASIOSampleRate sampleRate );
	public: virtual ASIOError getSampleRate( ASIOSampleRate* pSampleRate );
	public: virtual ASIOError setSampleRate( ASIOSampleRate sampleRate );

//ASIO start/stop...
	public: virtual ASIOError start( void );
	public: virtual ASIOError stop( void );

//ASIO switch buffers...
	public: virtual ASIOError outputReady( void );

	public: static DWORD WINAPI Thread_BufferSwitch_ASIO_1( glAsio_c* pGlAsio );
	public: static DWORD WINAPI Thread_BufferSwitch_ASIO_2( glAsio_c* pGlAsio );

	protected: enum SWITCH_TYPE
	{
		SWITCH_TYPE_ASIO_1,
		SWITCH_TYPE_ASIO_2
	};
	protected: SWITCH_TYPE			m_Switch_type;

	protected: HANDLE				m_Switch_hThread;
	protected: BOOL					m_Switch_stopThread;

	protected: ASIOTime				m_AsioTime;

//Kernel ASIO...
	enum DRIVER_HANDLE_TYPE
	{
		DRIVER_HANDLE_TYPE_GET,
		DRIVER_HANDLE_TYPE_CLOSE
	};

	protected: BOOL Kernel_ASIO_DriverHandle( DRIVER_HANDLE_TYPE type );
	protected: BOOL Kernel_ASIO_EngineVersion();

	protected: BOOL Kernel_ASIO_Channels_Close();
	protected: BOOL Kernel_ASIO_Channels_Open( registry_data_s* pData );

	protected: BOOL Kernel_ASIO_LockMemory();
	protected: BOOL Kernel_ASIO_UnlockMemory();

	protected: BOOL Kernel_ASIO_ProtectingIRP_Remove();
	protected: BOOL Kernel_ASIO_ProtectingIRP_Set();

	protected: BOOL Kernel_ASIO_Start();
	protected: BOOL Kernel_ASIO_Stop();

	protected: BOOL Kernel_ASIO_SwitchEvent_Remove();
	protected: BOOL Kernel_ASIO_SwitchEvent_Set();


	protected: unsigned long		m_hAsio;

	protected: HANDLE				m_ProtectingIRP_hDeviceIoControl;
	protected: OVERLAPPED			m_ProtectingIRP_overllaped;

	protected: unsigned long*		m_pMemory_global;

	protected: HANDLE				m_Switch_hEvent;

//Kernel common...
	protected: HANDLE GetDeviceIoControl();

	protected: HANDLE				m_hDeviceIoControl;
	protected: HANDLE				m_hDeviceIoControl_1;
	protected: HANDLE				m_hDeviceIoControl_2;
	protected: HANDLE				m_hDeviceIoControl_3;
	protected: HANDLE				m_hDeviceIoControl_4;

//Kernel DATA...
	protected: BOOL Kernel_DATA_CardsInfo();
	protected: BOOL Kernel_DATA_EngineVersion();
	protected: BOOL Kernel_DATA_BufferSize();
	protected: BOOL Kernel_DATA_InputMonitoring( ASIOInputMonitor* pMonitoring );
	protected: BOOL Kernel_DATA_SampleRate_Get( unsigned long* pSampleRate );
	protected: BOOL Kernel_DATA_SampleRate_Set( unsigned long sampleRate );

	protected: ioctl_cardInfo_s* Kernel_GetMasterCard();

	protected: ioctl_cardInfo_s		m_Cards[IOCTL_MAX_CARDS];

	protected: unsigned long		m_BufferSize;
	protected: unsigned long		m_SampleRate;
};
