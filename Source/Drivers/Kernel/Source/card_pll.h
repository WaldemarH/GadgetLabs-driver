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

#pragma once

//Notice:
//Power down pin is set automatically, so to power down parts of the PLL all
//we need to do is to set appropriate bits in registers.

//***************************************************************************
//**  REGISTERS  ************************************************************
//***************************************************************************
#define PLL_REFDIV_A			0x00	//byte 0
#define PLL_FBKDIV_A			0x01	//byte 1
#define PLL_VARIOUS_A			0x02	//byte 2

#define PLL_REFDIV_B			0x03	//byte 3
#define PLL_FBKDIV_B			0x04	//byte 4
#define PLL_VARIOUS_B			0x05	//byte 5

#define PLL_REFDIV_C1			0x06	//byte 6
#define PLL_FBKDIV_C1			0x07	//byte 7
#define PLL_VARIOUS_C1			0x08	//byte 8

#define PLL_REFDIV_C2			0x09	//byte 9
#define PLL_FBKDIV_C2			0x0A	//byte 10
#define PLL_VARIOUS_C2_D1		0x0B	//byte 11

#define PLL_POST_AB				0x0C	//byte 12
#define PLL_POST_C1D1			0x0D	//byte 13
#define PLL_POST_C2D2			0x0E	//byte 14

#define PLL_PD_C2D2				0x0F	//byte 15

//***************************************************************************
//**  various defines  ******************************************************
//***************************************************************************
#define PLL_CHARGE_PUMP			0x08
#define PLL_LOOP_FILTER			0x10
#define PLL_POWER_DOWN_PLL		0x20

#define PLL_MUX_REFERENCE		0x00	//reference clock
#define PLL_MUX_PLL_A			0x40	//pll A
#define PLL_MUX_PLL_B			0x80	//pll B
#define PLL_MUC_PLL_C			0xC0	//pll C

#define PLL_MUX_REFERENCE_C2	0x00	//reference clock
#define PLL_MUX_PLL_A_C2		0x10	//pll A
#define PLL_MUX_PLL_B_C2		0x20	//pll B
#define PLL_MUC_PLL_C_C2		0x30	//pll C

#define PLL_MUX_REFERENCE_D2	0x00	//reference clock
#define PLL_MUX_PLL_A_D2		0x40	//pll A
#define PLL_MUX_PLL_B_D2		0x80	//pll B
#define PLL_MUC_PLL_C_D2		0xC0	//pll C

#define PLL_POWER_DOWN_POST_A	0x01
#define PLL_POWER_DOWN_POST_B	0x02
#define PLL_POWER_DOWN_POST_C	0x04
#define PLL_POWER_DOWN_POST_D	0x08

//*****************************************************************************
//**  PLL structs  ************************************************************
//*****************************************************************************
//A few next structures provides data need to program PLLs.
//It is assumed that the data bit and clock bit used to program the PLLs are
//in the same register.
struct pll_header_s
{
	WORD				dataBit;
	WORD				dataBit_highZ;
	WORD				clockBit;
	WORD				programRegister;
	WORD				I2CAddress;
};
//PLL register information. Contains both address and data.
//To terminate programming a table must end with an address of 0xFFFF.
struct pll_registerData_s
{
	WORD				address;
	WORD				data;
};
//Stucture used to provide data for PLL programming.
//Structure will contain HW information and the actual register information.
struct pll_info_s
{
	pll_header_s		header;
	pll_registerData_s	data[17];	//6377 16 regs + eol
};
struct pll_info_sampleRate_s
{
	pll_header_s		header;
	pll_registerData_s	data[5];	//4 registers + eol
};

