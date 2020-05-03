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
//		pll.cpp
//
// PURPOSE:
//		File contains the card class PLL integrated circuit functions.
//
// STATUS:
//		Unfinished/Untested.
//
//***************************************************************************
//***************************************************************************
#include "main.h"
#include "card_altera_firmware.h"
#include "card_pll.h"


//*****************************************************************************
//**  PLL structs  ************************************************************
//*****************************************************************************
//The programming sequence is the same for all PLL's since they use the
//I2C bus. A few next structures provides data need to program PLLs.
//It is assumed that the data bit and clock bit used to program the PLLs are
//in the same register.

//Data used to program the FS6377 PLL on the 4xx cards.
//
//Input clock to PLL is 24.576MHz which was meant for 256*fs
//clock system. But as 824 cards work with 384*fs and as we should
//be able to sync the cards together the 4xx cards will be
//set to 384*fs too.
//
//Next: in PLL datasheet the AC Timing Specifications table shows us that the
//smallest jitter available is when only pll A is active and all others
//are disabled. So to change the sample rate we'll reprogram the A pll every
//time the sample rate change request will come.
//In this way we'll bring as clean clock to the CODECs as it gets and with
//that the sound we'll get better.
//Also to get lower jitter we need to have as high VCO clock as it gets.
//
//Coefficients:
// - 44100Hz => 16.9344MHz = 44100 * 384 = 24.576 * (441/64) * (1/10); CP = 0, LFTC = 0
// - 48000Hz => 18.4320MHz = 48000 * 384 = 24.576 * (240/32) * (1/10); CP = 0, LFTC = 0
// - 88200Hz => 33.8688MHz = 88200 * 384 = 24.576 * (441/64) * (1/5);  CP = 0, LFTC = 0
// - 96000Hz => 36.8640MHz = 96000 * 384 = 24.576 * (240/32) * (1/5);  CP = 0, LFTC = 0
//
//NOTES:    Input clock for 4xx PLL is 24.576 MHz.
//          Assumes all registers in the 4XX FS6377 are 0 on power up.
//
//NOTICE:
//Power down pin is set automatically, so to power down parts of the PLL all
//we need to do is to set appropriate bits in registers.
//
#pragma DATA_LOCKED
//Default registers settings -> PLL A is set to 16.9344MHz (for 44.1kHz) and
//all others are disabled.
pll_info_s Pll_4xx_default  =
{
//pll_header_s
	{
		SC4_PLL_INTERFACE_DATA,
		SC4_PLL_SET_DATA_PIN_TO_HIGH_Z,
		SC4_PLL_INTERFACE_CLK,
		SC_DEVICES,
		SC4_PLL_ADDR
	},
//pll_registerData_s
	{
	//PLL_A - 44100Hz => 16.9344MHz = 44100 * 384 = 24.576 * (441/64) * (1/10); CP = 0, LFTC = 0
	//Mux: PLL_A
		{ PLL_REFDIV_A,			0x40 },  //BYTE 0 (64)
		{ PLL_FBKDIV_A,			0xB9 },  //BYTE 1 (BYTE2 * BYTE1 = 441)
		{ PLL_VARIOUS_A,		PLL_MUX_PLL_A | 0x01 },  //BYTE 2

	//PLL_B - power down
	//Mux: PLL_B
		{ PLL_REFDIV_B,			0x00 },  //BYTE 3
		{ PLL_FBKDIV_B,			0x00 },  //BYTE 4
		{ PLL_VARIOUS_B,		PLL_MUX_PLL_B | PLL_POWER_DOWN_PLL },  //BYTE 5

	//PLL_C1 - power down
	//Mux: PLL_C
		{ PLL_REFDIV_C1,		0x00 },  //BYTE 6
		{ PLL_FBKDIV_C1,		0x00 },  //BYTE 7
		{ PLL_VARIOUS_C1,		PLL_MUX_PLL_B | PLL_POWER_DOWN_PLL },  //BYTE 8

	//PLL_C2 - 24.576MHz = 96000 * 256 = 24.576 * (8/8) * (1/1)
	//Mux: PLL_B, Power: On (probably it's turned off when everything else will be turned off)
		{ PLL_REFDIV_C2,		0x00 },  //BYTE 9
		{ PLL_FBKDIV_C2,		0x00 },  //BYTE 10
		{ PLL_VARIOUS_C2_D1,	PLL_MUX_PLL_B },  //BYTE 11

	//Post dividers
		{ PLL_POST_AB,			0x08 },  //BYTE 12 (divide A by 10)
		{ PLL_POST_C1D1,		0x00 },  //BYTE 13
		{ PLL_POST_C2D2,		0x00 },  //BYTE 14

	//Mux C2, D2
		{ PLL_PD_C2D2,			PLL_MUX_PLL_B_D2 | PLL_MUX_PLL_B_C2 | PLL_POWER_DOWN_POST_D | PLL_POWER_DOWN_POST_C | PLL_POWER_DOWN_POST_B },  //BYTE 15

	//End
		{ 0xFFFF, 0 }
	}
};
const pll_info_sampleRate_s		card_c::m_Pll_4xx_44100Hz =
{
//pll_header_s
	{
		SC4_PLL_INTERFACE_DATA,
		SC4_PLL_SET_DATA_PIN_TO_HIGH_Z,
		SC4_PLL_INTERFACE_CLK,
		SC_DEVICES,
		SC4_PLL_ADDR
	},
//pll_registerData_s
	{
	//PLL_A - 44100Hz => 16.9344MHz = 44100 * 384 = 24.576 * (441/64) * (1/10); CP = 0, LFTC = 0	-> VCO Clock = 169.344 MHz
	//VCO clock = 169.344MHz
	//Mux: PLL_A
		{ PLL_REFDIV_A,			0x40 },  //BYTE 0 (64)
		{ PLL_FBKDIV_A,			0xB9 },  //BYTE 1 (BYTE2 * BYTE1 = 441)
		{ PLL_VARIOUS_A,		PLL_MUX_PLL_A | 0x01 },  //BYTE 2

	//Post dividers
		{ PLL_POST_AB,			0x08 },  //BYTE 12 (divide A by 10)

	//End
		{ 0xFFFF, 0 }
	}
};
const pll_info_sampleRate_s		card_c::m_Pll_4xx_48000Hz =
{
//pll_header_s
	{
		SC4_PLL_INTERFACE_DATA,
		SC4_PLL_SET_DATA_PIN_TO_HIGH_Z,
		SC4_PLL_INTERFACE_CLK,
		SC_DEVICES,
		SC4_PLL_ADDR
	},
//pll_registerData_s
	{
	//PLL_A - 48000Hz => 18.4320MHz = 48000 * 384 = 24.576 * (240/32) * (1/10); CP = 0, LFTC = 0	-> VCO Clock = 184.32 MHz
	//VCO clock = 184.32MHz
	//Mux: PLL_A
		{ PLL_REFDIV_A,			0x20 },  //BYTE 0 (32)
		{ PLL_FBKDIV_A,			0xF0 },  //BYTE 1 (BYTE2 * BYTE1 = 240)
		{ PLL_VARIOUS_A,		PLL_MUX_PLL_A | 0x00 },  //BYTE 2

	//Post dividers
		{ PLL_POST_AB,			0x08 },  //BYTE 12 (divide A by 10)

	//End
		{ 0xFFFF, 0 }
	}
};
const pll_info_sampleRate_s		card_c::m_Pll_4xx_88200Hz =
{
//pll_header_s
	{
		SC4_PLL_INTERFACE_DATA,
		SC4_PLL_SET_DATA_PIN_TO_HIGH_Z,
		SC4_PLL_INTERFACE_CLK,
		SC_DEVICES,
		SC4_PLL_ADDR
	},
//pll_registerData_s
	{
	//PLL_A - 88200Hz => 33.8688MHz = 88200 * 384 = 24.576 * (441/64) * (1/5);  CP = 0, LFTC = 0	-> VCO Clock = 169.344 MHz
	//VCO clock = 169.344MHz
	//Mux: PLL_A
		{ PLL_REFDIV_A,			0x40 },  //BYTE 0 (64)
		{ PLL_FBKDIV_A,			0xB9 },  //BYTE 1 (BYTE2 * BYTE1 = 441)
		{ PLL_VARIOUS_A,		PLL_MUX_PLL_A | 0x01 },  //BYTE 2

	//Post dividers
		{ PLL_POST_AB,			0x04 },  //BYTE 12 (divide A by 5)

	//End
		{ 0xFFFF, 0 }
	}
};
const pll_info_sampleRate_s		card_c::m_Pll_4xx_96000Hz =
{
//pll_header_s
	{
		SC4_PLL_INTERFACE_DATA,
		SC4_PLL_SET_DATA_PIN_TO_HIGH_Z,
		SC4_PLL_INTERFACE_CLK,
		SC_DEVICES,
		SC4_PLL_ADDR
	},
//pll_registerData_s
	{
	//PLL_A - 96000Hz => 36.8640MHz = 96000 * 384 = 24.576 * (240/32) * (1/5);  CP = 0, LFTC = 0	-> VCO Clock = 184.32 MHz
	//VCO clock = 184.32MHz
	//Mux: PLL_A
		{ PLL_REFDIV_A,			0x20 },  //BYTE 0 (32)
		{ PLL_FBKDIV_A,			0xF0 },  //BYTE 1 (BYTE2 * BYTE1 = 240)
		{ PLL_VARIOUS_A,		PLL_MUX_PLL_A | 0x00 },  //BYTE 2

	//Post dividers
		{ PLL_POST_AB,			0x04 },  //BYTE 12 (divide A by 5)

	//End
		{ 0xFFFF, 0 }
	}
};


//***************************************************************************
// ProcedureName:
//		Set4xxPll
//
// Explanation:
//		Procedure will program FS6377 PLL on 4xx cards via the I2C bus.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return:
//		STATUS_SUCCESS/STATUS_UNSUCCESSFUL
//
#pragma CODE_PAGED
NTSTATUS card_c::PLL_Set4xxPll
(
	void
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	NTSTATUS	Status;

//Load PLL registers.
	Status = PLL_LoadRegisters( &Pll_4xx_default );

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

//***************************************************************************
// ProcedureName:
//		PLL_LoadRegisters
//
// Explanation:
//		Procedure will load FS6377 PLL registers via the I2C bus.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// pPllInfo - IN
//		Pointer to pll info structure.
//
// --------------------------------------------------------------------------
// Return:
//		STATUS_SUCCESS/STATUS_UNSUCCESSFUL
//
#pragma CODE_PAGED
NTSTATUS card_c::PLL_LoadRegisters
(
	IN pll_info_s*	pPllInfo
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	NTSTATUS				Status;

	pll_header_s*			pHeader = (pll_header_s*)&( pPllInfo->header );
	pll_registerData_s*		pRegisterData = (pll_registerData_s*)&( pPllInfo->data[0] );

//Loop through pll_registerData_s tables and output data to chip.
	while ( pRegisterData->address != 0xFFFF )
	{
	//Try to program current register->if it fails try a few more times.
		for ( int i = 0; i < 4; i++ )
		{
			Status = PLL_RegisterWrite_Random( pHeader, pRegisterData );
			if ( SUCCESS( Status ) )
			{
				break;
			}
		}

	//Everything ok?
		if ( FAILED( Status ) )
		{
			break;
		}

	//Update.
		++pRegisterData;
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

//***************************************************************************
// ProcedureName:
//		PLL_RegisterWrite_Random
//
// Explanation:
//		Procedure will program FS6377 PLL via the I2C bus.
//
//		Notice:
//		At start of this procedure DATA and CLOCK bit must be in LOW position.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// pHeader - IN
//		Pointer to header structure(pll_header_s) defining main info for
//		programming pll.
//
// pRegisterData - IN
//		Pointer to register data structure(pll_registerData_s) defining
//		pll register values.
//
// --------------------------------------------------------------------------
// Return:
// 		Should return STATUS_SUCCESS or STATUS_UNSUCCESSFUL.
//
#pragma CODE_LOCKED
NTSTATUS card_c::PLL_RegisterWrite_Random
(
	IN const pll_header_s*				pHeader,
	IN const pll_registerData_s*		pRegisterData
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
	pLogger->Log(
	  "  register address: %d, data: 0x%X\n",
	  pRegisterData->address,
	  pRegisterData->data
	);
#endif

	NTSTATUS		Status = STATUS_SUCCESS;

	WORD			clockBit;
	BYTE			currentByte;
	WORD			data;
	WORD			dataBit;
	WORD			dataBit_highZ;
	PUSHORT			programmingPort;

//Initialize.
	clockBit = pHeader->clockBit;
	dataBit = pHeader->dataBit;
	dataBit_highZ = pHeader->dataBit_highZ;
	programmingPort = (PUSHORT) ( (char*)m_IoWave + pHeader->programRegister );

	data = ReadWord( programmingPort );

//Program register.
//
//Send START Data Transfer command.
//
//dataBit: HIGH->LOW (while clock bit is HIGH)
	//start command
	data &= ~( dataBit );
	WriteWord( programmingPort, data );
	DelayThreadShort( DELAY_THREAD_SHORT_10us );

	//Take clock from high to low.
	data &= ~( clockBit );
	WriteWord( programmingPort, data );
	DelayThreadShort( DELAY_THREAD_SHORT_10us );

//Send Device Address (MSB first - 7bit address).
	currentByte = (BYTE) pHeader->I2CAddress;
	for ( WORD currentBit = 0x40; currentBit != 0; currentBit >>= 1 )
	{
	//Test bit.
		if ( ( currentByte & currentBit ) != 0 )
		{
		//Bit = 1
			data |= dataBit;
		}
		else
		{
		//Bit = 0
			data &= ~( dataBit );
		}

	//Set bit.
		WriteWord( programmingPort, data );
		DelayThreadShort( DELAY_THREAD_SHORT_10us );

	//Push data into pll (create a clk pulse).
		data |= clockBit;
		WriteWord( programmingPort, data );
		DelayThreadShort( DELAY_THREAD_SHORT_10us );

		data &= ~(clockBit);
		WriteWord( programmingPort, data );
		DelayThreadShort( DELAY_THREAD_SHORT_10us );
	}

//Write command bit -> data low.
	data &= ~( dataBit );
	WriteWord( programmingPort, data );
	DelayThreadShort( DELAY_THREAD_SHORT_10us );

	//Push data into pll (create a clk pulse).
	data |= clockBit;
	WriteWord( programmingPort, data );
	DelayThreadShort( DELAY_THREAD_SHORT_10us );

	data &= ~( clockBit );
	WriteWord( programmingPort, data );
	DelayThreadShort( DELAY_THREAD_SHORT_10us );

//Get acknowledge from PLL.
//
//Put the data line into high position, then into highZ and then
//the clock line.
//If not done this way toggle will be interpreted as a stop command.
//The first 'data line into high' command could be skipped as there
//is a pull up resistor connected.
	data |= dataBit;
	WriteWord( programmingPort, data );
	DelayThreadShort( DELAY_THREAD_SHORT_10us );

	data |= dataBit_highZ;
	WriteWord( programmingPort, data );
	DelayThreadShort( DELAY_THREAD_SHORT_10us );

	data |= clockBit;
	WriteWord( programmingPort, data );
	DelayThreadShort( DELAY_THREAD_SHORT_35us );

	//Poll data line and see if it goes low.
	//If it doesn't after a certain period of time -> fail.
	data = ReadWord( programmingPort );
	if ( ( data & dataBit ) != 0 )
	{
	//No acknowledge -> failed.
	//
	//Create Stop signal.
		//Prepare lines.
		data &= ~( clockBit );
		WriteWord( programmingPort, data );
		DelayThreadShort( DELAY_THREAD_SHORT_10us );

		data &= ~( dataBit | dataBit_highZ );
		WriteWord( programmingPort, data );
		DelayThreadShort( DELAY_THREAD_SHORT_10us );

		//Stop command.
		data |= clockBit;
		WriteWord( programmingPort, data );
		DelayThreadShort( DELAY_THREAD_SHORT_10us );

		data |= dataBit;
		WriteWord( programmingPort, data );
		DelayThreadShort( DELAY_THREAD_SHORT_10us );

		Status = STATUS_UNSUCCESSFUL;
	}
	else
	{
	//All ok -> clock line low and enable data bit back.
		data &= ~( clockBit );
		WriteWord( programmingPort, data );
		DelayThreadShort( DELAY_THREAD_SHORT_10us );

		data &= ~( dataBit_highZ );
		WriteWord( programmingPort, data );
		DelayThreadShort( DELAY_THREAD_SHORT_10us );
	}

	if ( SUCCESS( Status ) )
	{
	//Send Register Address.
		currentByte = (BYTE) pRegisterData->address;

		//Send
		for ( WORD currentBit = 0x80; currentBit != 0; currentBit >>= 1 )
		{
		//Test bit.
			if ( ( currentByte & currentBit ) != 0 )
			{
			//Bit = 1
				data |= dataBit;
			}
			else
			{
			//Bit = 0
				data &= ~(dataBit);
			}

		//Set bit.
			WriteWord( programmingPort, data );
			DelayThreadShort( DELAY_THREAD_SHORT_10us );

		//Push data into pll (create a clk pulse).
			data |= clockBit;
			WriteWord( programmingPort, data );
			DelayThreadShort( DELAY_THREAD_SHORT_10us );

			data &= ~( clockBit );
			WriteWord( programmingPort, data );
			DelayThreadShort( DELAY_THREAD_SHORT_10us );
		}

	//Get acknowledge from PLL.
	//
	//Put the data line into high position, then into highZ and then
	//the clock line.
	//If not done this way toggle will be interpreted as a stop command.
	//The first 'data line into high' command could be skipped as there
	//is a pull up resistor connected.
		data |= dataBit;
		WriteWord( programmingPort, data );
		DelayThreadShort( DELAY_THREAD_SHORT_10us );

		data |= dataBit_highZ;
		WriteWord( programmingPort, data );
		DelayThreadShort( DELAY_THREAD_SHORT_10us );

		data |= clockBit;
		WriteWord( programmingPort, data );
		DelayThreadShort( DELAY_THREAD_SHORT_35us );

		//Poll data line and see if it goes low.
		//If it doesn't after a certain period of time -> fail.
		data = ReadWord( programmingPort );
		if ( ( data & dataBit ) != 0 )
		{
		//No acknowledge -> failed.
		//
		//Create Stop signal.
			//Prepare lines.
			data &= ~( clockBit );
			WriteWord( programmingPort, data );
			DelayThreadShort( DELAY_THREAD_SHORT_10us );

			data &= ~( dataBit | dataBit_highZ );
			WriteWord( programmingPort, data );
			DelayThreadShort( DELAY_THREAD_SHORT_10us );

			//Stop command.
			data |= clockBit;
			WriteWord( programmingPort, data );
			DelayThreadShort( DELAY_THREAD_SHORT_10us );

			data |= dataBit;
			WriteWord( programmingPort, data );
			DelayThreadShort( DELAY_THREAD_SHORT_10us );

			Status = STATUS_UNSUCCESSFUL;
		}
		else
		{
		//All ok -> clock line low and enable data bit back.
			data &= ~( clockBit );
			WriteWord( programmingPort, data );
			DelayThreadShort( DELAY_THREAD_SHORT_10us );

			data &= ~( dataBit_highZ );
			WriteWord( programmingPort, data );
			DelayThreadShort( DELAY_THREAD_SHORT_10us );
		}
	}

	if ( SUCCESS( Status ) )
	{
	//Send data.
		currentByte = (BYTE) pRegisterData->data;

		//Send
		for ( WORD currentBit = 0x80; currentBit != 0; currentBit >>= 1 )
		{
		//Test bit.
			if ( ( currentByte & currentBit ) != 0 )
			{
			//Bit = 1
				data |= dataBit;
			}
			else
			{
			//Bit = 0
				data &= ~(dataBit);
			}

		//Set bit.
			WriteWord( programmingPort, data );
			DelayThreadShort( DELAY_THREAD_SHORT_10us );

		//Push data into pll (create a clk pulse).
			data |= clockBit;
			WriteWord( programmingPort, data );
			DelayThreadShort( DELAY_THREAD_SHORT_10us );

			data &= ~( clockBit );
			WriteWord( programmingPort, data );
			DelayThreadShort( DELAY_THREAD_SHORT_10us );
		}

	//Get acknowledge from PLL.
	//
	//Put the data line into high position, then into highZ and then
	//the clock line.
	//If not done this way toggle will be interpreted as a stop command.
	//The first 'data line into high' command could be skipped as there
	//is a pull up resistor connected.
		data |= dataBit;
		WriteWord( programmingPort, data );
		DelayThreadShort( DELAY_THREAD_SHORT_10us );

		data |= dataBit_highZ;
		WriteWord( programmingPort, data );
		DelayThreadShort( DELAY_THREAD_SHORT_10us );

		data |= clockBit;
		WriteWord( programmingPort, data );
		DelayThreadShort( DELAY_THREAD_SHORT_35us );

		//Poll data line and see if it goes low.
		//If it doesn't after a certain period of time -> fail.
		data = ReadWord( programmingPort );
		if ( ( data & dataBit ) != 0 )
		{
			Status = STATUS_UNSUCCESSFUL;
		}

	//Send STOP Data Transfer command in both cases ( SUCCESS or FAILED ).
		data &= ~( clockBit );
		WriteWord( programmingPort, data );
		DelayThreadShort( DELAY_THREAD_SHORT_10us );

		data &= ~( dataBit | dataBit_highZ );
		WriteWord( programmingPort, data );
		DelayThreadShort( DELAY_THREAD_SHORT_10us );

		//Stop command.
		data |= clockBit;
		WriteWord( programmingPort, data );
		DelayThreadShort( DELAY_THREAD_SHORT_10us );

		data |= dataBit;
		WriteWord( programmingPort, data );
		DelayThreadShort( DELAY_THREAD_SHORT_35us );
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

