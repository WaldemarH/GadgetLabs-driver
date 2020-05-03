#include "main.h"
#include "gl_asio.h"
#include "..\..\Kernel\Source\driver_ioctl_data.h"


ASIOError glAsio_c::canSampleRate( ASIOSampleRate sampleRate )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Global();
	pLogger->Log( "  sampleRate: %f\n", sampleRate );
#endif

	ASIOError	AStatus;

	switch ( (unsigned long)sampleRate )
	{
	case 44100:
	case 48000:
	{
		AStatus = ASE_OK;
		break;
	}
	case 88200:
	case 96000:
	{
	//Only 4xx cards can handle this sample rates... is there an 824 card present?
		AStatus = ASE_OK;

		for ( int i = 0; i < ELEMENTS_IN_ARRAY( m_Cards ); i++ )
		{
			if ( m_Cards[i].m_Type == ioctl_cardInfo_s::CARD_TYPE_824 )
			{
			//Found a 824 card -> sample rate unsupported.
				AStatus = ASE_NoClock;
				break;
			}
		}
		break;
	}
	default:
	{
		AStatus = ASE_NoClock;
		break;
	}
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( AStatus, -1 );
#endif
	return AStatus;
}

ASIOError glAsio_c::getSampleRate( ASIOSampleRate* pSampleRate )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Global();
#endif

	ASIOError	AStatus;

//Get master card sample rate.
	unsigned long		sampleRate = 0;

	BOOL	Status = Kernel_DATA_SampleRate_Get( &sampleRate );
	if ( SUCCESS( Status ) )
	{
	//Convert double to unsigned long.
		*pSampleRate = (ASIOSampleRate)sampleRate;
		AStatus = ASE_OK;
	}
	else
	{
		AStatus = ASE_HWMalfunction;
	}

#ifdef ASIO_LOGGER
	pLogger->Log( "  sampleRate: %f\n", *pSampleRate );
	Procedure_Exit( AStatus, -1 );
#endif
	return AStatus;
}

ASIOError glAsio_c::setSampleRate( ASIOSampleRate sampleRate )
{
#ifdef ASIO_LOGGER
	Procedure_Start_Global();
	pLogger->Log( "  sampleRate: %f\n", sampleRate );
#endif

	ASIOError	AStatus;

//Get master card sample rate.
	BOOL	Status = Kernel_DATA_SampleRate_Set( (unsigned long)sampleRate );
	if ( SUCCESS( Status ) )
	{
	//Update ASIO time.
		m_AsioTime.timeInfo.sampleRate = sampleRate;

		AStatus = ASE_OK;
	}
	else
	{
		AStatus = ASE_HWMalfunction;
	}

#ifdef ASIO_LOGGER
	Procedure_Exit( AStatus, -1 );
#endif
	return AStatus;
}
