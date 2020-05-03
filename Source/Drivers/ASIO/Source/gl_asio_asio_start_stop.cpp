#include "main.h"
#include "gl_asio.h"


ASIOError glAsio_c::start( void )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Global();
#endif

	ASIOError		AStatus = ASE_OK;

//Validate.
	if (
	  ( m_pHostCallbacks == NULL )
	  ||
	  ( m_pMemory_global == NULL )
	  ||
	  ( m_pStart == NULL )
	)
	{
		strcpy_s( m_ErrorMessage, sizeof( m_ErrorMessage ), "Start: something wasn't initialized... can not start." );
		AStatus = ASE_InvalidMode;
	}

//Reset variables.
	if ( SUCCESS_ASIO( AStatus ) )
	{
	//Thread related...
		m_Switch_stopThread = FALSE;

		ResetEvent( m_Switch_hEvent );
	}

//Set bufferSwitch thread.
	if ( SUCCESS_ASIO( AStatus ) )
	{
		if ( m_Switch_type == SWITCH_TYPE_ASIO_1 )
		{
		//ASIO 1.0
			m_Switch_hThread = CreateThread( NULL, 0, ( LPTHREAD_START_ROUTINE )&Thread_BufferSwitch_ASIO_1, this, 0, 0 );
		}
		else
		{
		//ASIO 2.0
			m_Switch_hThread = CreateThread( NULL, 0, ( LPTHREAD_START_ROUTINE )&Thread_BufferSwitch_ASIO_2, this, 0, 0 );
		}
		if ( m_Switch_hThread == NULL )
		{
			strcpy_s( m_ErrorMessage, sizeof( m_ErrorMessage ), "Start function failed to create switch thread." );
			AStatus = ASE_InvalidMode;
		}
	}

//Start channels.
	if ( SUCCESS_ASIO( AStatus ) )
	{
		if ( FALSE == Kernel_ASIO_Start() )
		{
			AStatus = ASE_HWMalfunction;
		}
	}

//Was all ok?
	if ( FAILED_ASIO( AStatus ) )
	{
	//Stop thread.
		if ( m_Switch_hThread != NULL )
		{
		//Signal that thread must exit.
			m_Switch_stopThread = TRUE;

		//Fire proceed event.
			SetEvent( m_Switch_hEvent );

		//Give it enough time to finish.
			Sleep( 20 );

		//At this time the thread should exit already -> close handle.
			CloseHandle( m_Switch_hThread );
			m_Switch_hThread = NULL;
		}
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( AStatus, -1 );
#endif
	return AStatus;
}

ASIOError glAsio_c::stop( void )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Global();
#endif

	ASIOError		AStatus = ASE_OK;

//Signal that thread must exit.
	m_Switch_stopThread = TRUE;

	//Give it enough time to finish.
	Sleep( 20 );

//Stop channels.
	if ( FALSE == Kernel_ASIO_Stop() )
	{
		AStatus = ASE_HWMalfunction;
	}

//Stop switch thread.
	if ( SUCCESS_ASIO( AStatus ) )
	{
		if ( m_Switch_hThread != NULL )
		{
		//At this time the thread should be finished already, but we just must make sure.
			DWORD	exitValue = -1;

			if ( FALSE == GetExitCodeThread( m_Switch_hThread, &exitValue ) )
			{
			//Something is wrong or thread is at sleep -> stop the thread.
				SetEvent( m_Switch_hEvent );

			//Give it enough time to finish.
				Sleep( 100 );
			}

		//At this time the thread should exit already -> close handle.
			CloseHandle( m_Switch_hThread );
			m_Switch_hThread = NULL;
		}
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( AStatus, -1 );
#endif
	return AStatus;
}
