#include "main.h"
#include "gl_asio.h"
#include "..\..\Kernel\Source\driver_ioctl_asio.h"


ASIOError glAsio_c::outputReady( void )
{
//Signal lower driver that it should copy the buffers.
//
//In this mode we can get 1 buffer size of output latency -> in other case we would get 2.
//Read in GLAsio::getLatencies about this.
//
//Notice:
//Also note that the engine may fail to call ASIOOutputReady() in time in overload cases.
//As already mentioned, bufferSwitch should be called for every block regardless of whether
//a block could be processed in time or not.
//
//In this case the driver will signal to the buffer switch that there was a CPU overrun.
//
	unsigned long	bytesReturned;
	unsigned long	outputData = 0;

	BOOL	Status = DeviceIoControl(
	  m_hDeviceIoControl,
	  IOCTL_ASIO_COPY_OUTPUT_BUFFERS,
	  &m_hAsio,
	  sizeof( m_hAsio ),
	  NULL,
	  0,
	  &bytesReturned,
	  NULL
	);
	if ( Status == FALSE )
	{
		strcpy_s( m_ErrorMessage, sizeof( m_ErrorMessage ), "OutputReady can't access lower driver!" );
		return ASE_HWMalfunction;
	}

	return ASE_OK;
}

DWORD WINAPI glAsio_c::Thread_BufferSwitch_ASIO_1( glAsio_c* pGlAsio )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Global();
#endif

//Rise thread priority -> Priority = highest
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL );

//Initialize variables.
	ASIOCallbacks*			pHostCallbacks = pGlAsio->m_pHostCallbacks;
	ioctl_asio_switch*		pSwitch = (ioctl_asio_switch*)pGlAsio->m_pMemory_global;
	BOOL*					pSwitch_stopThread = &pGlAsio->m_Switch_stopThread;
	HANDLE					switch_hEvent = pGlAsio->m_Switch_hEvent;

//Set working loop
	do
	{
	//Wait for switch event.
		WaitForSingleObject( switch_hEvent, INFINITE );

	//We wont request this as if user is playing a gig it's better to get a short click/pop then the stop of the audio engine.
	//If something will be really wrong user will stop and start the playback manually.
	//
	////Do we need synhronization? Did we lost some buffers?
	//	if ( pSwitch->m_Synhronize == TRUE )
	//	{
	//		pSwitch->m_Synhronize = FALSE;

	//	//Set sync request to host.
	//		pHostCallbacks->asioMessage( kAsioResyncRequest, 0, NULL, 0 );
	//	}

	//Switch buffers.
		pHostCallbacks->bufferSwitch( pSwitch->m_BufferSelect, ASIOFalse );

	} while ( *pSwitch_stopThread == FALSE );

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, TRUE );
#endif
	return 0;
}

DWORD WINAPI glAsio_c::Thread_BufferSwitch_ASIO_2( glAsio_c* pGlAsio )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Global();
#endif

//Rise thread priority -> Priority = highest
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL );

//Initialize variables.
	ASIOTime*				pAsioTime = &pGlAsio->m_AsioTime;
	ASIOCallbacks*			pHostCallbacks = pGlAsio->m_pHostCallbacks;
	ioctl_asio_switch*		pSwitch = (ioctl_asio_switch*)pGlAsio->m_pMemory_global;
	BOOL*					pSwitch_stopThread = &pGlAsio->m_Switch_stopThread;
	HANDLE					switch_hEvent = pGlAsio->m_Switch_hEvent;

	sampleClock_s			timeStamp;

//Set working loop
	do
	{
	//Wait for switch event.
		WaitForSingleObject( switch_hEvent, INFINITE );

	//We wont request this as if user is playing a gig it's better to get a short click/pop then the stop of the audio engine.
	//If something will be really wrong user will stop and start the playback manually.
	//
	////Do we need synhronization? Did we lost some buffers?
	//	if ( pSwitch->m_Synhronize == TRUE )
	//	{
	//		pSwitch->m_Synhronize = FALSE;

	//	//Set sync request to host.
	//		pHostCallbacks->asioMessage( kAsioResyncRequest, 0, NULL, 0 );
	//	}

	//Get systemTime.
		timeStamp.clock = (ULONGLONG)timeGetTime() * (ULONGLONG)1000000;		//in 1ns

	//Set clock and timestamp.
		//Time stamp.
		pAsioTime->timeInfo.systemTime.lo = timeStamp.dwords.dword_lo;
		pAsioTime->timeInfo.systemTime.hi = timeStamp.dwords.dword_hi;

		//Sample position.
		pAsioTime->timeInfo.samplePosition.lo = pSwitch->m_Clock_samples.dwords.dword_lo;
		pAsioTime->timeInfo.samplePosition.hi = pSwitch->m_Clock_samples.dwords.dword_hi;

	//#ifdef ASIO_PRINT
		//char		buffer[256];
		//sprintf( buffer, "\nAsioTimeInfo Run( bufferSelect: %d )\n", pSwitch->m_BufferSelect );
		//OutputDebugString( buffer );
		//sprintf( buffer, "  speed: %f\n", pAsioTime->timeInfo.speed );
		//OutputDebugString( buffer );
		//sprintf( buffer, "  timeInfo.systemTime.lo: 0x%X\n", pAsioTime->timeInfo.systemTime.lo );
		//OutputDebugString( buffer );
		//sprintf( buffer, "  timeInfo.systemTime.hi: 0x%X\n", pAsioTime->timeInfo.systemTime.hi );
		//OutputDebugString( buffer );
		//sprintf( buffer, "  timeInfo.samplePosition.lo: 0x%X\n", pAsioTime->timeInfo.samplePosition.lo );
		//OutputDebugString( buffer );
		//sprintf( buffer, "  timeInfo.samplePosition.hi: 0x%X\n", pAsioTime->timeInfo.samplePosition.hi );
		//OutputDebugString( buffer );
		//sprintf( buffer, "  sampleRate: %f\n", pAsioTime->timeInfo.sampleRate );
		//OutputDebugString( buffer );
		//sprintf( buffer, "  flags: 0x%X\n", pAsioTime->timeInfo.flags );
		//OutputDebugString( buffer );
	//#endif

	//Switch buffers.
		pHostCallbacks->bufferSwitchTimeInfo( pAsioTime, pSwitch->m_BufferSelect, ASIOFalse );

	} while ( *pSwitch_stopThread == FALSE );

#ifdef ASIO_LOGGER
	Procedure_Exit( -1, TRUE );
#endif
	return 0;
}
