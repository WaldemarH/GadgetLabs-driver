//***************************************************************************
//***************************************************************************
//
//                          PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license agreement or
// nondisclosure agreement with Waldemar Haszlakiewicz and may not be
// copied or disclosed except in accordance with the terms of that agreement.
//
//               Copyright (C) 2009 Waldemar Haszlakiewicz
//                            All Rights Reserved.
//
//***************************************************************************
//***************************************************************************
//
// MODULE:
//		codec.cpp
//
// PURPOSE:
//		File contains the card class CODECS functions.
//
// STATUS:
//		Unfinished/Untested.
//
//***************************************************************************
//***************************************************************************
#include "main.h"
#include "card_altera_firmware.h"
#include "card_codec.h"


//***************************************************************************
// ProcedureName:
//		CODEC_Set4xxCodecs
//
// Explanation:
//		Procedure will program 4xx cards codecs (AK4524).
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return:
// 		void
//
#pragma CODE_PAGED
void card_c::CODEC_Set4xxCodecs
(
	void
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	WORD	data;

//Power Up the codecs.
//
//LRCK = fs ( 44100Hz <- default for SoundCache )
//(1/LRCK)*516cycles = 11.7ms <= time needed for Codecs to power up
//We will delay it for 20ms just to be sure.
	data = ReadWord( m_IoWave, SC_DEVICES );
	data |= SC4_CODEC_POWER_UP;
	WriteWord( m_IoWave, SC_DEVICES, data );
	DelayThreadLong_PassiveLevel( DELAY_THREAD_LONG_20ms );

//Program codecs.
//1st: Set clock mode and audio data interface mode.
	CODEC_WriteCodecsRegister(
	  SC4_CODEC_CS1 | SC4_CODEC_CS2,
	  AK_CLK,
	  AK_CLK_FMT_I2S | AK_CLK_384FSN | AK_CLK_FSD
	);

//2nd: Cancel the reset state by setting RSTAD and RSTDA to "1".
	CODEC_WriteCodecsRegister( SC4_CODEC_CS1 | SC4_CODEC_CS2, AK_RST, AK_RST_ADC );
	CODEC_WriteCodecsRegister( SC4_CODEC_CS1 | SC4_CODEC_CS2, AK_RST, AK_RST_DAC | AK_RST_ADC );

//Delay a little just to be 100% that everything is stable.
	DelayThreadLong_PassiveLevel( DELAY_THREAD_LONG_20ms );

#ifdef DBG_AND_LOGGER
	Procedure_Exit( STATUS_SUCCESS );
#endif
	return;
}

//***************************************************************************
// ProcedureName:
//		CODEC_WriteCodecsRegister
//
// Explanation:
//		Procedure will program AK4524 codec.
//
//		Notice:
//		Procedure is working with CIF = 0.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// codecSelect - IN
//		Defines which codec to program (use one of of SC4_CCR_CODEC_CSx flags).
//
// registerAddress - IN
//		Defines address of AK4524 register.
//
// registerData - IN
//		Defines data that will be send to register (control data).
//
// --------------------------------------------------------------------------
// Return:
//		void
//
#pragma CODE_LOCKED
void card_c::CODEC_WriteCodecsRegister
(
	IN WORD		codecSelect,
	IN WORD		registerAddress,
	IN WORD		registerData
)
{
	WORD	data;
	WORD	sendWord;

//Create 'send to AK4524' word.
//
//bit           name            meaning
//0,..,7        D0,..,D7        controlData
//8,9,10,11,12  A0,A1,A2,A3,A4  register address
//13            R/W             read/write (fixed to '1' ->write)
//14,15         C1,C0           chip address (fixed at '10')
	sendWord = 0xA000;	//1010  0000 0000 0000 (10 1 00000 00000000)
	registerAddress <<= 8;		//shift left 8 bits
	sendWord |= registerAddress;
	sendWord |= registerData;

//Select codec ( CS flag LOW->HIGH->LOW ).
	data = ReadWord( m_IoWave, SC_DEVICES );
	data |= codecSelect;
	WriteWord( m_IoWave, SC_DEVICES, data );
	DelayThreadShort( DELAY_THREAD_SHORT_1us );

	data &= ~( codecSelect );
	WriteWord( m_IoWave, SC_DEVICES, data );
	DelayThreadShort( DELAY_THREAD_SHORT_1us );

//Program the register.
	for ( WORD currentBit = 0x8000; currentBit != 0; currentBit >>= 1 )
	{
	//Clock HIGH->LOW.
		data &= ~( SC4_CODEC_CONTROL_CLK );
		WriteWord( m_IoWave, SC_DEVICES, data );
		DelayThreadShort( DELAY_THREAD_SHORT_1us );

	//Test bit.
		if ( ( sendWord & currentBit ) != 0 )
		{
		//Bit=1
			data |= SC4_CODEC_CONTROL_DATA;
		}
		else
		{
		//Bit=0
			data &= ~( SC4_CODEC_CONTROL_DATA );
		}

	//Set bit.
		WriteWord( m_IoWave, SC_DEVICES, data );
		DelayThreadShort( DELAY_THREAD_SHORT_1us );

	//Push data. Clock LOW->HIGH.
		data |= SC4_CODEC_CONTROL_CLK;
		WriteWord( m_IoWave, SC_DEVICES, data );
		DelayThreadShort( DELAY_THREAD_SHORT_1us );
	}

//Latch data (CS flag LOW->HIGH->LOW).
	data |= codecSelect;
	WriteWord( m_IoWave, SC_DEVICES, data );
	DelayThreadShort( DELAY_THREAD_SHORT_1us );

	data &= ~( codecSelect );
	WriteWord( m_IoWave, SC_DEVICES, data );
	DelayThreadShort( DELAY_THREAD_SHORT_1us );

	//Not really needed but I want that input and output SC_DEVICES register values are the same.
	data &= ~( SC4_CODEC_CONTROL_DATA );
	WriteWord( m_IoWave, SC_DEVICES, data );
	DelayThreadShort( DELAY_THREAD_SHORT_1us );

	return;
}
