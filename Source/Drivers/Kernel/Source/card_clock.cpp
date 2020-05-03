//***************************************************************************
//***************************************************************************
//
//                          PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license agreement or
// nondisclosure agreement with Waldemar Haszlakiewicz and may not be
// copied or disclosed except in accordance with the terms of that agreement.
//
//               Copyright (C) 2010 Waldemar Haszlakiewicz
//                            All Rights Reserved.
//
//***************************************************************************
//***************************************************************************
//
// MODULE:
//		card.cpp
//
// PURPOSE:
//		File contains the card class functions.
//
// STATUS:
//		Unfinished/Untested.
//
//***************************************************************************
//***************************************************************************
#include "main.h"
#include "card_altera_firmware.h"


//***************************************************************************
// ProcedureName:
//		CanMasterAndSlaveCardsHandleRisedSampleRate
//
//		NOTICE:
//		Call only from master card!!
//
// Explanation:
//		Procedure will check if slave card are able to handle rised sample
//		rates (88.2kHz and 96kHz).
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return:
//		STATUS_SUCCESS(cards can handle it) or
//		STATUS_UNSUCCESSFUL(procedure was not called from master card) or
//		STATUS_NO_MATCH (slave cards can't handle the sample rate).
//
#pragma CODE_LOCKED
NTSTATUS card_c::CanMasterAndSlaveCardsHandleRisedSampleRate
(
	void
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	NTSTATUS		Status = STATUS_SUCCESS;

//Is this a master card?
	if ( m_pCard_master != NULL )
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//824 cards can't handle more then 48kHz of sample rate.
	if ( SUCCESS( Status ) )
	{
		card_c*		pCard = this;
		while ( pCard != NULL )
		{
		//Test for 824 card.
			if ( pCard->m_Type == CARD_TYPE_824 )
			{
				Status = STATUS_NO_MATCH;
				break;
			}

		//Next...
			pCard = pCard->m_pCard_slave;
		}
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

//***************************************************************************
// ProcedureName:
//		SetClockSource
//
// Explanation:
//		Procedure will check if we can change clock source (if the sampleRate
//		is owned) and change it if we are allowed.
//
//		If sample rate is owned procedure will return
//		STATUS_UNSUCCESSFUL.
//
//		Notice:
//		Set clock source from master card down to the last slave card and
//		not in any other order.
//
//		Notice2:
//		Procedure must be protected with glClass.LockDriver().
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// clockSource - IN
//		Clock source (use one of CLOCK_SOURCE_xxxx).
//
// --------------------------------------------------------------------------
// Return:
//		STATUS_SUCCESS/STATUS_UNSUCCESSFUL
//
#pragma CODE_LOCKED
NTSTATUS card_c::SetClockSource
(
	IN CLOCK_SOURCE		clockSource_new
)
{
	NTSTATUS	Status = STATUS_SUCCESS;

//Is sample rate owned?
	if ( m_SampleRate_owned > 0 )
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//Check if we need to change the clock source.
	if ( SUCCESS( Status ) )
	{
		if ( m_ClockSource != clockSource_new )
		{
		//Set new clock source.
			const pll_header_s*				p4xx_Header = NULL;
			const pll_registerData_s*		p4xx_RegisterData = NULL;

			WORD	data = ReadWord( m_IoWave, SC_ENGINE );
			data &= ~( SC_SAMPLE_RATE_MASK );

			switch ( clockSource_new )
			{
			case CLOCK_SOURCE_CARD:
			{
				if ( m_Type != CARD_TYPE_824 )
				{
				//4xx
					//data |= SC_SAMPLE_RATE_4xx_PLL_A;

				//Set the new sample rate to 4xx card.
					p4xx_Header = &m_Pll_4xx_44100Hz.header;
					p4xx_RegisterData = &m_Pll_4xx_44100Hz.data[0];
				}
				else
				{
				//824
					//data |= SC_SAMPLE_RATE_824_44100Hz;
				}
				m_SampleRate = 44100;

			//Set new clockSource.
				m_ClockSource = CLOCK_SOURCE_CARD;
				break;
			}
			case CLOCK_SOURCE_EXTERN:
			{
			//Set clock to syncCable.
				data |= SC_SAMPLE_RATE_SYNC_IN;

				//Set new clockSource.
				m_ClockSource = CLOCK_SOURCE_EXTERN;
				m_SampleRate = m_pCard_master->m_SampleRate;
				break;
			}
			default:
				Status = STATUS_UNSUCCESSFUL;
				break;
			}
			if ( SUCCESS( Status ) )
			{
			//Set new sample rate.
				SetSampleRate_ToHardware( data, p4xx_Header, p4xx_RegisterData );
			}
		}
	}
	return Status;
}

//*****************************************************************************
// ProcedureName:
//		SetSampleRate
//
// Explanation:
//		Procedure will try to change the sampleRate of the card clock.
//
//		Notice:
//		Procedure must be protected with glClass.LockDriver().
//
//		Notice:
//		Can only be called on master card.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// ----------------------------------------------------------------------------
// sampleRateL - IN
//		Sample rate to set (use one of SAMPLERATE_xxxxx).
//
// ----------------------------------------------------------------------------
// Return:
//		STATUS_SUCCESS,
//		STATUS_UNSUCCESSFUL (sample rate is owned, external source is enabled,
//		unknown sample rate)
//
#pragma CODE_LOCKED
NTSTATUS card_c::SetSampleRate
(
	IN unsigned long	sampleRate_new
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
	pLogger->Log( "  sampleRate requested: %d\n", sampleRate_new );
#endif

	NTSTATUS				Status = STATUS_SUCCESS;

//Is this a master card?
	if ( m_pCard_master != NULL )
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//Validate new value...
	if ( SUCCESS( Status ) )
	{
		switch ( sampleRate_new )
		{
		case 44100:
		case 48000:
		case 88200:
		case 96000:
			break;
		default:
			Status = STATUS_UNSUCCESSFUL;
			break;
		}
	}

//Check if current sample rate is owned or not.
	if ( SUCCESS( Status ) )
	{
		if ( m_SampleRate_owned > 0 )
		{
			if ( sampleRate_new != (unsigned long)m_SampleRate )
			{
				Status = STATUS_UNSUCCESSFUL;
			}
		}
	}

//Check if currently set sample rate is the same as requested.
	if ( SUCCESS( Status ) )
	{
		if ( sampleRate_new == (unsigned long)m_SampleRate )
		{
		//STATUS_SUCCESS -> we don't have to set the same sampleRate again -> jump over.
			goto SetSampleRate_Exit;
		}
	}

//Can we change the sampleRate-> is clockSource set to local card oscilator?
	if ( SUCCESS( Status ) )
	{
		if ( m_ClockSource != CLOCK_SOURCE_CARD )
		{
			Status = STATUS_UNSUCCESSFUL;
		}
	}

//Is the sample rate to high for card?
	if ( SUCCESS( Status ) )
	{
		if ( m_Type == CARD_TYPE_824 )
		{
			if ( sampleRate_new > 48000 )
			{
				Status = STATUS_UNSUCCESSFUL;
			}
		}
	}

//Set new sample rate.
	if ( SUCCESS( Status ) )
	{
	#ifdef DBG_AND_LOGGER
		pLogger->Log( "  Set new sample rate: %d\n", sampleRate_new );
	#endif

		const pll_header_s*				p4xx_Header = NULL;
		const pll_registerData_s*		p4xx_RegisterData = NULL;

		WORD	data = ReadWord( m_IoWave, SC_ENGINE );
		data &= ~( SC_SAMPLE_RATE_MASK );

		switch ( sampleRate_new )
		{
		case 44100 :
			if ( m_Type != CARD_TYPE_824 )
			{
			//4xx
				//data |= SC_SAMPLE_RATE_4xx_PLL_A;

			//Set the new sample rate to 4xx card.
				p4xx_Header = &m_Pll_4xx_44100Hz.header;
				p4xx_RegisterData = &m_Pll_4xx_44100Hz.data[0];
			}
			else
			{
			//824
				//data |= SC_SAMPLE_RATE_824_44100Hz;
			}
			break;

		case 48000 :
			if ( m_Type != CARD_TYPE_824 )
			{
			//4xx
				//data |= SC_SAMPLE_RATE_4xx_PLL_A;

			//Set the new sample rate to 4xx card.
				p4xx_Header = &m_Pll_4xx_48000Hz.header;
				p4xx_RegisterData = &m_Pll_4xx_48000Hz.data[0];
			}
			else
			{
			//824
				data |= SC_SAMPLE_RATE_824_48000Hz;
			}
			break;

		case 88200 :
		//4xx
			//data |= SC_SAMPLE_RATE_4xx_PLL_A;

		//Set the new sample rate to 4xx card.
			p4xx_Header = &m_Pll_4xx_88200Hz.header;
			p4xx_RegisterData = &m_Pll_4xx_88200Hz.data[0];
			break;

		case 96000 :
		//4xx
			//data |= SC_SAMPLE_RATE_4xx_PLL_A;

		//Set the new sample rate to 4xx card.
			p4xx_Header = &m_Pll_4xx_96000Hz.header;
			p4xx_RegisterData = &m_Pll_4xx_96000Hz.data[0];
			break;

		default:
			Status = STATUS_UNSUCCESSFUL;
			break;
		}
		if ( SUCCESS( Status ) )
		{
			m_SampleRate = sampleRate_new;

		//Set new sample rate.
			SetSampleRate_ToHardware( data, p4xx_Header, p4xx_RegisterData );
		}
	}

SetSampleRate_Exit:
#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

//*****************************************************************************
// ProcedureName:
//		SetSampleRate_ForceDesync
//
// Explanation:
//		Procedure will change clock from sync to local oscilator and set
//		requested samplerate.
//
//		Notice:
//		If GUI fails to desync card in a normal way, we have to force desync
//		or we can come into serious problems if synced cards get uninstalled...
//		the real problems happens if the same card gets installed back while
//		other cards are still running synced.
//
// IRQ Level:
//		ANY
//
// ----------------------------------------------------------------------------
// sampleRateL - IN
//		Sample rate to set (use one of SAMPLERATE_xxxxx).
//
// ----------------------------------------------------------------------------
// Return:
//		STATUS_SUCCESS,
//		STATUS_UNSUCCESSFUL (sample rate is owned, external source is enabled,
//		unknown sample rate)
//
#pragma CODE_LOCKED
NTSTATUS card_c::SetSampleRate_ForceDesync
(
	IN unsigned long		sampleRate_new
)
{
	NTSTATUS		Status = STATUS_SUCCESS;

//Change clockSource to local oscillator and set the new sampleRate.
	const pll_header_s*				p4xx_Header = NULL;
	const pll_registerData_s*		p4xx_RegisterData = NULL;

	//Reset sampleRate flags.
	WORD	data = ReadWord( m_IoWave, SC_ENGINE );
	data &= ~( SC_SAMPLE_RATE_MASK );

	//Set local oscilator sampleRate.
	switch ( sampleRate_new )
	{
	case 44100:
	{
		if ( m_Type != CARD_TYPE_824 )
		{
		//4xx
			//data |= SC_SAMPLE_RATE_4xx_PLL_A;

		//Set the new sample rate to 4xx card.
			p4xx_Header = &m_Pll_4xx_44100Hz.header;
			p4xx_RegisterData = &m_Pll_4xx_44100Hz.data[0];
		}
		else
		{
		//824
			//data |= SC_SAMPLE_RATE_824_44100Hz;
		}
		break;
	}
	case 48000:
	{
		if ( m_Type != CARD_TYPE_824 )
		{
		//4xx
			//data |= SC_SAMPLE_RATE_4xx_PLL_A;

		//Set the new sample rate to 4xx card.
			p4xx_Header = &m_Pll_4xx_48000Hz.header;
			p4xx_RegisterData = &m_Pll_4xx_48000Hz.data[0];
		}
		else
		{
		//824
			data |= SC_SAMPLE_RATE_824_48000Hz;
		}
		break;
	}
	case 88200:
	{
	//4xx
		//data |= SC_SAMPLE_RATE_4xx_PLL_A;

	//Set the new sample rate to 4xx card.
		p4xx_Header = &m_Pll_4xx_88200Hz.header;
		p4xx_RegisterData = &m_Pll_4xx_88200Hz.data[0];
		break;
	}
	case 96000:
	{
	//4xx
		//data |= SC_SAMPLE_RATE_4xx_PLL_A;

	//Set the new sample rate to 4xx card.
		p4xx_Header = &m_Pll_4xx_96000Hz.header;
		p4xx_RegisterData = &m_Pll_4xx_96000Hz.data[0];
		break;
	}
	default:
		Status = STATUS_UNSUCCESSFUL;
		break;
	}

//Force new clockSource.
	if ( SUCCESS( Status ) )
	{
		m_SampleRate = sampleRate_new;

		SetSampleRate_ToHardware( data, p4xx_Header, p4xx_RegisterData );
	}

	return Status;
}

//***************************************************************************
// ProcedureName:
//		SetSampleRate_ToHardware
//
// Explanation:
//		Procedure will set the requested sampleRate(clock source) on the
//		sound card.
//
//		Notice:
//		Don't call this procedure directly -> use SetSampleRate or
//		SetClockSource.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// clockSource - IN
//		Clock source (use one of CLOCKSOURCE_xxxx).
//
//WALDEMAR
//Popravi text.
//
// --------------------------------------------------------------------------
// Return:
//		STATUS_SUCCESS/STATUS_UNSUCCESSFUL
//
#pragma CODE_LOCKED
void card_c::SetSampleRate_ToHardware
(
	IN WORD							data,
	IN const pll_header_s*			p4xx_Header,
	IN const pll_registerData_s*	p4xx_RegisterData
)
{
	NTSTATUS	Status = STATUS_SUCCESS;

//1)Stop card.
	WORD		data_temp = ReadWord( m_IoWave, SC_ENGINE );
	data_temp &= ~( SC_START_AUDIO_ENGINE );
	WriteWord( m_IoWave, SC_ENGINE, data_temp );
	DelayThreadShort( DELAY_THREAD_SHORT_1us );

//2)Set new sampleRate (without starting the engine).
	data &= ~( SC_START_AUDIO_ENGINE );
	WriteWord( m_IoWave, SC_ENGINE, data );

	if ( p4xx_Header != NULL )
	{
	//This is a 4xx type of card -> reprogram pll.
		while ( p4xx_RegisterData->address != 0xFFFF )
		{
		//Try to program current register->if it fails try a few more times.
			long i = 4;
			while ( i != 0 )
			{
				Status = PLL_RegisterWrite_Random( p4xx_Header, p4xx_RegisterData );
				if ( SUCCESS( Status ) )
				{
					break;
				}
				--i;
			}

		//Everything ok?
			if ( FAILED( Status ) )
			{
			//We can't do anything more now as something serious must be wrong.
				break;
			}

		//Update.
			++p4xx_RegisterData;
		}
	}

//3)Reset all cards counters.
//We'll do this later at channel/s start.

//4) Set the master card interrupt downcounter value to bufferSize+1.
//We'll do this later at channel/s start.

//5) Start card.
	if ( SUCCESS( Status ) )
	{
		data |= SC_START_AUDIO_ENGINE;
		WriteWord( m_IoWave, SC_ENGINE, data );
		DelayThreadShort( DELAY_THREAD_SHORT_1us );
	}

	return;
}

//***************************************************************************
// ProcedureName:
// 		AddressPointers_Reset
//
//		Notice:
//		Can only be called on master card.
//
// Explanation:
//		Procedure will re-set ADC/DAC address pointers.
//
//		Notice:
//		You can call this procedure only if wave interrupt is disabled.
//
//		Notice 2:
//		This procedure is intended only for master cards.
//
// IRQ Level:
//		only DISPATCH_LEVEL
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return:
// 		void
//
#pragma CODE_LOCKED
NTSTATUS card_c::AddressPointers_Reset()
{
	NTSTATUS		Status = STATUS_SUCCESS;

//Validate...
	//Is it a master card?
	if ( m_pCard_master != NULL )
	{
		Status = STATUS_UNSUCCESSFUL;
	}
	
//Start streaming.
	if ( SUCCESS( Status ) )
	{
	//Nothing should interrupt this process!!!!
	//Because CODECs don't like to wait for a long time, well for 824 card they will reset
	//converters automatically when clock dissapears.
		KIRQL	spinLock_oldLevel = KeAcquireInterruptSpinLock( m_pIRQObject );

		//Stop master card.
			WORD	data = ReadWord( m_IoWave, SC_ENGINE );
			data &= ~( SC_START_AUDIO_ENGINE );
			WriteWord( m_IoWave, SC_ENGINE, data );
			DelayThreadShort( DELAY_THREAD_SHORT_1us );

		//Reset all cards counters.
			card_c*		pCard = this;
			while ( pCard != NULL )
			{
				WORD	data_temp = ReadWord( pCard->m_IoWave, SC_ENGINE );
				data_temp &= ~( SC_RELEASE_COUNTERS );
				WriteWord( pCard->m_IoWave, SC_ENGINE, data_temp );
				DelayThreadShort( DELAY_THREAD_SHORT_1us );

				data_temp |= SC_RELEASE_COUNTERS;
				WriteWord( pCard->m_IoWave, SC_ENGINE, data_temp );

			//Next...
				pCard = pCard->m_pCard_slave;
			}
			DelayThreadShort( DELAY_THREAD_SHORT_1us );

		//4) Start master card.
			data |= SC_START_AUDIO_ENGINE;
			WriteWord( m_IoWave, SC_ENGINE, data );

	//Release spinLock.
		KeReleaseInterruptSpinLock( m_pIRQObject, spinLock_oldLevel );
	}

	return Status;
}