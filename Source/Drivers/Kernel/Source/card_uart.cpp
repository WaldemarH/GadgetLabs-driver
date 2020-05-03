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
//		uart.cpp
//
// PURPOSE:
//		File contains the card class UART integrated circuit functions.
//
// STATUS:
//		Unfinished/Untested.
//
//***************************************************************************
//***************************************************************************
#include "main.h"
#include "card_uart.h"

//***************************************************************************
// ProcedureName:
//		UART_SetFlagsForMIDI
//
// Explanation:
//		Procedure will set MIDI transfer parameters on UART.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return:
//		None.
//
#pragma CODE_PAGED
void card_c::UART_SetFlagsForMIDI
(
	void
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	BYTE		data;

//Init to 8 data bits, 1 start and 1 stop bit and enable divisor latch access.
	WriteByte( m_IoUart, UART_LCR, UART_LCR_WLS_8 | UART_LCR_STB_1 | UART_LCR_DLAB );

// MIDI_CLK(31250) * 16 = 4 MHz / 8.
	WriteByte( m_IoUart, UART_DLM, 0 );
	WriteByte( m_IoUart, UART_DLL, 8 );

//Disable access to divisor latches.
	WriteByte( m_IoUart, UART_LCR, UART_LCR_WLS_8 | UART_LCR_STB_1 );

//Enable received data interrupt for input FIFO.
	WriteByte( m_IoUart, UART_IER, UART_IER_ERBFI );

//Setup FIFOs for operation. Enable and set receive threshold to 1 byte.
//
//Also put the UART in DMA mode which will tell us when there is space
//in the transmit FIFO to put data. TXRDY is tied to DSR, so DSR
//can be tested for TXRDY.
	data = UART_FCR_RF_1 | UART_FCR_DMA | UART_FCR_FEN;
	WriteByte( m_IoUart, UART_FCR, data );

//Clear input and output FIFOs.
	data |= ( UART_FCR_XF_RST | UART_FCR_RF_RST );
	WriteByte( m_IoUart, UART_FCR, data );

	data &= ~( UART_FCR_XF_RST | UART_FCR_RF_RST );
	WriteByte( m_IoUart, UART_FCR, data );

#ifdef DBG_AND_LOGGER
	Procedure_Exit( STATUS_SUCCESS );
#endif
	return;
}

//***************************************************************************
// ProcedureName:
//		UART_Reset_FIFO_Input/UART_Reset_FIFO_Output
//
// Explanation:
//		Procedure will reset UART FIFO.
//
// IRQ Level:
//		ANY
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return:
//		None.
//

#pragma CODE_LOCKED
void card_c::UART_Reset_FIFO_Input
(
	void
)
{
//Setup FIFOs for operation. Enable and set receive threshold to 1 byte
//(not really needed as we wont be using interrupts).
//
//Also put the UART in DMA mode which will tell us when there is space
//in the transmit FIFO to put data. TXRDY is tied to DSR, so DSR
//can be tested for TXRDY.
	BYTE	data = UART_FCR_RF_1 | UART_FCR_DMA | UART_FCR_FEN;
	WriteByte( m_IoUart, UART_FCR, data );

//Clear input FIFO.
	data |= ( UART_FCR_RF_RST );
	WriteByte( m_IoUart, UART_FCR, data );

	data &= ~( UART_FCR_RF_RST );
	WriteByte( m_IoUart, UART_FCR, data );

	return;
}
void card_c::UART_Reset_FIFO_Output
(
	void
)
{
//Setup FIFOs for operation. Enable and set receive threshold to 1 byte
//(not really needed as we wont be using interrupts).
//
//Also put the UART in DMA mode which will tell us when there is space
//in the transmit FIFO to put data. TXRDY is tied to DSR, so DSR
//can be tested for TXRDY.
	BYTE	data = UART_FCR_RF_1 | UART_FCR_DMA | UART_FCR_FEN;
	WriteByte( m_IoUart, UART_FCR, data );

//Clear output FIFO.
	data |= ( UART_FCR_XF_RST );
	WriteByte( m_IoUart, UART_FCR, data );

	data &= ~( UART_FCR_XF_RST );
	WriteByte( m_IoUart, UART_FCR, data );

	return;
}
