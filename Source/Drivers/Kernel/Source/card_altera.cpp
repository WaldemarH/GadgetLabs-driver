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
//		altera.cpp
//
// PURPOSE:
//		File contains the card class Altera integrated circuit functions.
//
// STATUS:
//		Unfinished/Untested.
//
//***************************************************************************
//***************************************************************************
#include "main.h"
#include "card_altera_firmware_4xx.h"
#include "card_altera_firmware_824.h"
#include "card_uart.h"


//***************************************************************************
// ProcedureName:
//		Altera_LoadAndTestFirmware
//
// Explanation:
//		Procedure will program card PLD chip and test if it works.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return:
//		Should return STATUS_SUCCESS or error code returned from a
//		called procedure.
//
#pragma CODE_PAGED
NTSTATUS card_c::Altera_LoadAndTestFirmware
(
	void
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	NTSTATUS	Status;

//Program Main Altera chip.
	switch ( m_Type )
	{
	case CARD_TYPE_424 :
	case CARD_TYPE_496 :
		//If programming fails try to do it a few more times.
		for ( long i = 0; i < 3; i++ )
		{
			Status = Altera_LoadFirmware( (BYTE*)&bAlteraFirmware_4xx );
			if ( SUCCESS( Status ) )
			{
				break;
			}
		}
		break;

	case CARD_TYPE_824 :
		//If programming fails try to do it a few more times.
		for ( long i = 0; i < 3; i++ )
		{
			Status = Altera_LoadFirmware( (BYTE*)&bAlteraFirmware_824 );
			if ( SUCCESS( Status ) )
			{
				break;
			}
		}
		break;

	default:
		Status = STATUS_UNSUCCESSFUL;
		break;
	}

//Do a memory check.
	if ( SUCCESS( Status ) )
	{
		Status = Altera_MemoryTest_Analog();
	}

//Program daughter card Altera and do a memory test.
	//m_Type_daughter = CARD_TYPE_DAUGHTER_NONE;

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

//***************************************************************************
// ProcedureName:
//		Altera_LoadFirmware
//
// Explanation:
//		Procedure load firmware to Altera 6016.
//
//		Programming happens throught the modem control port in the MIDI UART.
//		The UART inverts signals so all the bits are inverted in the software.
//
//		Loading the firmware starts when the nConfig pin is toggled
//		from high-low-high. After that data is clocked out to the Altera
//		least significant bit first until the Altera returns dConfig high.
//		After that dClk must be clocked 10 more times
//		(this 10 clocks are included in data already).
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// chipFirmware - IN
//		Pointer to programming data for Altera chip (use bAlteraFirmware_4xx
//		or bAlteraFirmware_824).
//
// --------------------------------------------------------------------------
// Return:
//		STATUS_SUCCESS/STATUS_UNSUCCESSFUL
//
#pragma CODE_PAGED
NTSTATUS card_c::Altera_LoadFirmware
(
	IN BYTE*			chipFirmware
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	NTSTATUS	Status = STATUS_SUCCESS;

	BYTE		alteraStatus;
	BYTE		data;

//Initialize UART (disable diagnostic testing of the UART ).
	data = ReadByte( m_IoUart, UART_MCR );
	data &= ~( UART_MCR_LOOP );
	WriteByte( m_IoUart, UART_MCR, data );
	DelayThreadShort( DELAY_THREAD_SHORT_10us );

//Set dClk, data and nConfig to logic LOW and
//start Altera ( nConfig to logic HIGH ).
	data |= ( DATA_CLK_OUT | DATA_BIT_OUT | NCONFIG_OUT );
	WriteByte( m_IoUart, ALTERA_PROG_OUT, data );
	DelayThreadShort( DELAY_THREAD_SHORT_4us );

	//start Altera chip ( nConfig to logic HIGH ).
	data &= ~( NCONFIG_OUT );
	WriteByte( m_IoUart, ALTERA_PROG_OUT, data );
	DelayThreadShort( DELAY_THREAD_SHORT_10us );

//Output data to Altera.
	for ( long i = 0; i < MAX_SC_PROG_BYTES; i++ )
	{
	//Bit loop (least significant bit first).
		BYTE	currentByte = chipFirmware[i];
		for ( long j = 0; j < 8; j++ )
		{
		//Test and set bit.
			if ( ( currentByte & 0x01 ) == 0 )
			{
			//Bit = 0 -> convert to logic value (invert)
				data |= DATA_BIT_OUT;
			}
			else
			{
			//Bit = 1 -> convert to logic value (invert)
				data &= ~( DATA_BIT_OUT );
			}
			//Set bit.
			WriteByte( m_IoUart, ALTERA_PROG_OUT, data );
			DelayThreadShort( DELAY_THREAD_SHORT_2us );

		//Push data into Altera (create a clk pulse).
			//Clk -> HIGH.
			data &= ~( DATA_CLK_OUT );
			WriteByte( m_IoUart, ALTERA_PROG_OUT, data );
			DelayThreadShort( DELAY_THREAD_SHORT_2us );
			//Clk -> LOW.
			data |= DATA_CLK_OUT;
			WriteByte( m_IoUart, ALTERA_PROG_OUT, data );
			DelayThreadShort( DELAY_THREAD_SHORT_2us );

		//Update.
			currentByte >>= 1;	//Shift right 1 bit.
		}
	}

//Set defaults.
	//Set data bit to logic low.
	data |= DATA_BIT_OUT;
	WriteByte( m_IoUart, ALTERA_PROG_OUT, data );
	DelayThreadShort( DELAY_THREAD_SHORT_2us );

//Test if programming failed.
//
//Is CONF_DONE at logic HIGH? (If fails CONF_DONE is at logic LOW.)
	alteraStatus = ReadByte( m_IoUart, ALTERA_PROG_IN );
	if ( ( alteraStatus & CONFIG_DONE ) != 0 )
	{
		Status = STATUS_UNSUCCESSFUL;
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

//***************************************************************************
// ProcedureName:
//		Altera_MemoryTest_Analog
//
// Explanation:
//		Procedure tests ADC and DAC memory.
//		It sends different dwords values (0101,1001,..) to memory and
//		checks if it's the same when retrieving it back.
//
//		Notice:
//		At the end the transmit memory is filled with zero samples
//		so that when the cards starts the output channels will transmit
//		0 samples (no sound).
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return:
//		STATUS_SUCCESS/STATUS_INSUFFICIENT_RESOURCES/STATUS_UNSUCCESSFUL.
//
#pragma CODE_PAGED
NTSTATUS card_c::Altera_MemoryTest_Analog
(
	void
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	NTSTATUS	Status = STATUS_SUCCESS;

	PULONG		tempBuffer;

//Get temp buffer memory.
	tempBuffer = (PULONG) ExAllocatePool( NonPagedPool, m_Mem_Analog_length );
	if ( tempBuffer == NULL )
	{
		Status = STATUS_INSUFFICIENT_RESOURCES;
	}

//Audio engine is turned off by default -> test if you can write to memory or not.
//
//Test all memory(ADC and DAC).
	if ( SUCCESS( Status ) )
	{
		Status = Altera_MemoryTest( m_Mem_Analog, tempBuffer, m_Mem_Analog_length, 0x00555555 );
	}
	if ( SUCCESS( Status ) )
	{
		Status = Altera_MemoryTest( m_Mem_Analog, tempBuffer, m_Mem_Analog_length, 0x00FFFFFF );
	}
	if ( SUCCESS( Status ) )
	{
		Status = Altera_MemoryTest( m_Mem_Analog, tempBuffer, m_Mem_Analog_length, 0x00AAAAAA );
	}
	if ( SUCCESS( Status ) )
	{
		Status = Altera_MemoryTest( m_Mem_Analog, tempBuffer, m_Mem_Analog_length, 0x00000000 );
	}

//Release memory.
	if ( Status != STATUS_INSUFFICIENT_RESOURCES )
	{
		ExFreePool( tempBuffer );
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

//***************************************************************************
// ProcedureName:
//		Altera_MemoryTest
//
// Explanation:
//		Procedure tests card memory.
//		It sends/retrieves dword memory and test it if it's the same.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// MemCard - IN
//		Pointer to card memory to test.
//
// Mem_Buffer - IN
//		Pointer to a buffer in which we will test if everything is ok.
//
// bufferLength - IN
//		Size of buffer in bytes.
//
// data - IN
//		DWORD fill data (the lower 3 bytes in the DWORD must be the same).
//
// --------------------------------------------------------------------------
// Return:
// 		STATUS_SUCCESS/STATUS_UNSUCCESSFUL
//
#pragma CODE_PAGED
NTSTATUS card_c::Altera_MemoryTest
(
	IN PULONG			pMem_Card,
	IN PULONG			pMem_Buffer,
	IN unsigned long	bufferLength,
	IN unsigned long	data
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	NTSTATUS			Status = STATUS_SUCCESS;

	unsigned long		bufferLength_samples = bufferLength/sizeof( unsigned long );
	unsigned long		offsetInBuffer;
	SAMPLE				sample;

//Create data buffer.
	RtlFillMemory( pMem_Buffer, bufferLength, (UCHAR)data );

//Fill the card memory.
	for ( unsigned long i = 0; i < bufferLength_samples; i++ )
	{
		pMem_Card[i] = pMem_Buffer[i];
	}

//Reset buffer memory.
	RtlZeroMemory( pMem_Buffer, bufferLength );

//Get memory from card.
	for ( unsigned long i = 0; i < bufferLength_samples; i++ )
	{
		pMem_Buffer[i] = pMem_Card[i];
	}

//Check retrieved memory if everything is the same.
	for ( unsigned long i = 0; i < bufferLength_samples; i++ )
	{
	//Get lower 24bits and test if the sample is the same as data.
		sample.data.dwords[0] = 0x00FFFFFF & pMem_Buffer[i];
		if ( sample.data.dwords[0] != data )
		{
			offsetInBuffer = i;
			Status = STATUS_UNSUCCESSFUL;
			break;
		}
	}

#ifdef DBG_AND_LOGGER
	if ( FAILED( Status ) )
	{
		pLogger->Log(
		  "  Error: Faulty sample: 0x%X (at offset: 0x%X/0x%X)\n",
		  sample,
		  offsetInBuffer,
		  bufferLength/sizeof(SAMPLE)
		);
	}
	Procedure_Exit( Status );
#endif
	return Status;
}
