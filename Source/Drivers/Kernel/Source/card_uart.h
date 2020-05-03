/*
*****************************************************************************
*****************************************************************************
**
**                   GADGET LABS INC PROPRIETARY INFORMATION
**
** This software is supplied under the terms of a license agreement or
** nondisclosure agreement with Gadget Labs Inc and may not be copied or
** disclosed except in accordance with the terms of that agreement.
**
**               Copyright (C) 1997, 1998, 1999 Gadget Labs Inc.
**                            All Rights Reserved.
**
*****************************************************************************
*****************************************************************************
**
** @doc INTERNAL
**
** @MODULE
**		N16550.H |
**		
**		Contains macros, structures and typedefs for the 16550 UART register
**		and bit definitions.
**
** @doc
**
** CONTENTS:
**		
**
** NOTES: 
**		* Tabstops are set to 4 and 80 character limit per source line.
**
** REVISION:
**		$Revision: 1.1 $
**			$Date: Fri Apr 09 17:43:06 1999 $
** 
*****************************************************************************
*****************************************************************************
*/

#pragma once

/***************************************************************************/
/********************************  MACROS  *********************************/
/***************************************************************************/

/***************************************************************************/
/***************************  16550  REGISTERS  ****************************/
/***************************************************************************/

#define UART_RBR		0x00	// DLAB=0, R: Receive buffer reg
#define UART_THR		0x00	// DLAB=0, W: Transmit holding reg
#define UART_IER		0x01	// DLAB=0, R/W: Interrupt enable reg
#define UART_DLL		0x00	// DLAB=1, R/W: Low divisor latch
#define UART_DLM		0x01	// DLAB=1, R/W: High divisor latch
#define UART_IIR		0x02	// R: Interrupt ID reg
#define UART_FCR		0x02	// W: FIFO control reg
#define UART_LCR		0x03	// R/W: Line control reg
#define UART_MCR		0x04	// R/W: Modem control reg
#define UART_LSR		0x05	// R/W: Line status reg
#define UART_MSR		0x06	// R/W: Modem status reg
#define UART_SCR		0x07	// R/W: Scratch reg

#define UART_MAX_REGS	0x08	// 8 regs * 4 bytes (32 bit regs)


/***************************************************************************/
/**************************  16550 REGISTERS BITS  *************************/
/***************************************************************************/

#define UART_IER_ERBFI	0x01	// Enable received data available int
#define UART_IER_ETBEI	0x02	// Enable transmit holding reg empty int
#define UART_IER_ELSI	0x04	// Enable receive line status int
#define UART_IER_EDSSI	0x08	// Enable modem status int

#define UART_IIR_PEND	0x01	// Interrupt bending bit
#define UART_IIR_INT_ID	0x0F	// Interrupt ID bits
#define UART_IIR_FEN	0xC0	// FIFO's enabled bits

#define UART_FCR_FEN	0x01	// FIFO enabled bit
#define UART_FCR_RF_RST	0x02	// Receive FIFO reset
#define UART_FCR_XF_RST	0x04	// Transmit FIFO reset
#define UART_FCR_DMA	0x08	// DMA mode select
#define UART_FCR_RF_1	0x00	// Receive interrupt trigger threshold at 1
#define UART_FCR_RF_4	0x40	// Receive interrupt trigger threshold at 4
#define UART_FCR_RF_8	0x80	// Receive interrupt trigger threshold at 8
#define UART_FCR_RF_14	0xC0	// Receive interrupt trigger threshold at 14
#define UART_FCR_NORMAL ( UART_FCR_FEN | UART_FCR_DMA | UART_FCR_RF_1 )		// FCR setting for normal operation

#define UART_LCR_WLS_5	0x00	// 5 bit word
#define UART_LCR_WLS_6	0x01	// 6 bit word
#define UART_LCR_WLS_7	0x02	// 7 bit word
#define UART_LCR_WLS_8	0x03	// 8 bit word
#define UART_LCR_STB_1	0x00	// Zero stop bits
#define UART_LCR_STB_2	0x04	// One stop bit
#define UART_LCR_PEN	0x08	// Parity enabled
#define UART_LCR_EPS	0x10	// Even parity
#define UART_LCR_SP		0x20	// Stick parity
#define UART_LCR_BRK	0x40	// Set break
#define UART_LCR_DLAB	0x80	// Divisor latch address bit

#define UART_MCR_DTR	0x01	// Activate DTR
#define UART_MCR_RTS	0x02	// Activate RTS
#define UART_MCR_OUT1	0x04	// Turn on out 1
#define UART_MCR_OUT2	0x08	// Turn on out 2
#define UART_MCR_LOOP	0x10	// Local loopback

#define UART_LSR_DR		0x01	// Data ready
#define UART_LSR_OE		0x02	// Overrun error
#define UART_LSR_PE		0x04	// Parity error
#define UART_LSR_FE		0x08	// Framing error
#define UART_LSR_BI		0x10	// Break
#define UART_LSR_THRE	0x20	// Transmit holding register
#define UART_LSR_TEMT	0x40	// Transmit empty
#define UART_LSR_RFIFO	0x80	// Receive FIFO error

#define UART_MSR_DCTS	0x01	// Clear to send change
#define UART_MSR_DDSR	0x02	// Data set ready has changed
#define UART_MSR_TERI	0x04	// Trailing edge of ring indicator
#define UART_MSR_DDCD	0x08	// Data carrier detect change
#define UART_MSR_CTS	0x10	// Clear to send
#define UART_MSR_DSR	0x20	// Data set ready
#define UART_MSR_RI		0x40	// Ring indicator
#define UART_MSR_DCD	0x80	// Data carrier detect

#define UART_XMIT_FIFO	16		// 16 bytes in UART transmit FIFO
#define UART_RCV_FIFO	16		// 16 bytes in UART receive FIFO


/***************************************************************************/
/*****************************  ALTERA PROG  *******************************/
/***************************************************************************/

#define ALTERA_PROG_REG		(UART_MCR)	// Altera is prog through modem ctrl
#define ALTERA_STS_REG		(UART_MSR)	// Altera status reg though modem sts

#define ALTERA_DCLK			(UART_MCR_OUT1)	// Out
#define ALTERA_DATA			(UART_MCR_OUT2)	// Out
#define ALTERA_NCONFIG		(UART_MCR_DTR)	// Out
#define ALTERA_NSTATUS		(UART_MSR_DCD)	// In
#define ALTERA_CONF_DONE	(UART_MSR_CTS)	// In


