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
#include "main.h"
#include "card_uart.h"
#include "midi.h"

//***************************************************************************
// ProcedureName:
//		Set_CopyBuffersProcedure_AVStream
//
// Explanation:
//		Procedure will select proper DX(WDM) procedure from the required type.
//
// IRQ Level:
//		ANY
//
// --------------------------------------------------------------------------
//
// dwChannelMask: IN
//		Defines which of the channels are enabled.
//
// --------------------------------------------------------------------------
// Return:
//		void
//
#pragma CODE_LOCKED
NTSTATUS channel_midi_c::Set_CopyBuffersProcedure()
{
	NTSTATUS	Status = STATUS_SUCCESS;

//Set procedure.
	if ( m_Type == CHANNEL_TYPE_IN )
	{
	//Input...
		switch ( m_Type_client )
		{
		case CHANNEL_TYPE_CLIENT_MME: m_pCopyBuffersProcedure = (PCOPY_BUFFER_MIDI)&channel_midi_c::CopyBuffer_MME_In; break;
		case CHANNEL_TYPE_CLIENT_DX: m_pCopyBuffersProcedure = (PCOPY_BUFFER_MIDI)&channel_midi_c::CopyBuffer_DX_In; break;
		default:
			Status = STATUS_UNSUCCESSFUL;
			break;
		}
	}
	else
	{
	//Output...
		switch ( m_Type_client )
		{
		case CHANNEL_TYPE_CLIENT_MME: m_pCopyBuffersProcedure = (PCOPY_BUFFER_MIDI)&channel_midi_c::CopyBuffer_MME_Out; break;
		case CHANNEL_TYPE_CLIENT_DX: m_pCopyBuffersProcedure = (PCOPY_BUFFER_MIDI)&channel_midi_c::CopyBuffer_DX_Out; break;
		default:
			Status = STATUS_UNSUCCESSFUL;
			break;
		}
	}

	return Status;
}

#pragma CODE_LOCKED
NTSTATUS channel_midi_c::CopyBuffer_MME_In( ULONGLONG timestamp )
{
//Initialize.
	PUCHAR		IoUart_LSR = m_pCard->Get_IoUart() + UART_LSR;
	PUCHAR		IoUart_RBR = m_pCard->Get_IoUart() + UART_RBR;

//Is there anything new in input FIFO?
	if ( ( ReadByte( IoUart_LSR ) & UART_LSR_DR ) == 0 )
	{
	//There is nothing new in input FIFO -> skip current call.
		return STATUS_PENDING;
	}

//Get all the new data from UART.
//
//Notice:
//LSR DR will be high when ever there is data in the input FIFO.
//
//Notice:
//On todays computers we'll more or less get just 1 byte of data, as ISR and DPC are really fast.
//Well it depends on how much CPU resources are used.
//
//Notice:
//As data can come without any command bytes we'll have a secondary space available in the buffer in case we'll have to
//insert command bytes also.
	BYTE			data[20+20/2];
	unsigned long	data_size = 0;

	while ( ( ReadByte( IoUart_LSR ) & UART_LSR_DR ) != 0 )
	{
	//Is buffer full?
	//
	//Notice:
	//There will never be 20 bytes available, so this is here just to make sure that some error wont cause driver crash.
		if ( data_size >= 20 )
		{
		//No more space -> leave rest for the next time.
			break;
		}

	//There is data available -> read it to driver buffer.
		data[data_size] = ReadByte( IoUart_RBR );
		++data_size;
	}

//Update timestamp.
//
//As timestamp was acquired in ISR just after the first byte interrupt flag was signaled, all we have to do is to
//substract the 320us from it and we'll have a timestamp of precision of +-320us.
//
//Notice:
//Timestamp is in 100ns -> 320us /100ns = 320000 ns/100ns = 3200
	timestamp -= 3200;
	
	//The timestamp is already correct -> skip the first update.
	bool	timestamp_update = false;	

//Process retrieved data.
	for ( unsigned long i = 0; i < data_size; i++ )
	{
	//Update timestamp.
		if ( timestamp_update == true )
		{
			timestamp += 3200;
		}
		else
		{
		//Skip just one timestamp update event.
			timestamp_update = true;
		}

	//Get data byte.
		BYTE	data_byte = data[i];
	//#ifdef GL_LOGGER
	//	pLogger->LogFast( "I: 0x%X\n", data_byte );
	//#endif

	//Process new data byte.
		switch ( data_byte >> 4 )
		{
		case 0xC:	//Program Change
		case 0xD:	//Channel Pressure (After-touch)
		{
		//A new command -> length: 2 bytes
			m_Data_In.m_Data[0] = data_byte;
			++m_Data_In.m_Data_size;

			m_Data_In.m_Command = data_byte;
			m_Data_In.m_Command_bytesNeeded = 1;	//1 more byte needed

			m_Data_In.m_Command_old = 0;

			m_Data_In.m_TimeStamp = timestamp;
			break;
		}
		case 0x8:	//Note Off
		case 0x9:	//Note On
		case 0xA:	//Polyphonic Key Pressure (Aftertouch)
		case 0xB:	//Control Change
		case 0xE:	//Pitch Wheel Change
		{
		//A new command -> length: 3 bytes
			m_Data_In.m_Data[0] = data_byte;
			m_Data_In.m_Data_size = 1;

			m_Data_In.m_Command = data_byte;
			m_Data_In.m_Command_bytesNeeded = 2;	//2 more byte needed

			m_Data_In.m_Command_old = 0;

			m_Data_In.m_TimeStamp = timestamp;
			break;
		}
		case 0xF:	//System Common Messages or System Real-Time Messages
		{
			switch ( data_byte )
			{
			case MIDI_COMMAND_SYSTEM_COMMON_EXCLUSIVE_START:
			{
			//A new command -> length: n bytes or MIDI_COMMAND_SYSTEM_EXCLUSIVE_END
				m_Data_In.m_Data[0] = data_byte;
				++m_Data_In.m_Data_size;

				m_Data_In.m_Command = data_byte;
				m_Data_In.m_Command_bytesNeeded = 11;	//we still need 11 bytes to fill up current buffer

				m_Data_In.m_Command_old = 0;

				m_Data_In.m_TimeStamp = timestamp;
				break;
			}
			case MIDI_COMMAND_SYSTEM_COMMON_EXCLUSIVE_END:
			{
			//End of system exlusive command -> send the remaining data to system.
				//Append byte to the end of buffer.
				m_Data_In.m_Data[m_Data_In.m_Data_size] = data_byte;
				++m_Data_In.m_Data_size;

				//Send it to the system.
				CopyBuffer_MME_In_SendCommand( &m_Data_In );

			//Reset command.
				m_Data_In.m_Data_size = 0;
				m_Data_In.m_Command = 0;
				m_Data_In.m_Command_old = 0;
				break;
			}
			case MIDI_COMMAND_SYSTEM_COMMON_MIDI_TIME_CODE:
			case MIDI_COMMAND_SYSTEM_COMMON_SONG_SELECT:
			{
			//A new command -> length: 2 bytes
				m_Data_In.m_Data[0] = data_byte;
				m_Data_In.m_Data_size = 1;

				m_Data_In.m_Command = data_byte;
				m_Data_In.m_Command_bytesNeeded = 1;	//1 more byte needed

				m_Data_In.m_Command_old = 0;

				m_Data_In.m_TimeStamp = timestamp;
				break;
			}
			case MIDI_COMMAND_SYSTEM_COMMON_SONG_POSITION:
			{
			//A new command -> length: 3 bytes
				m_Data_In.m_Data[0] = data_byte;
				m_Data_In.m_Data_size = 1;

				m_Data_In.m_Command = data_byte;
				m_Data_In.m_Command_bytesNeeded = 2;	//2 more byte needed

				m_Data_In.m_Command_old = 0;

				m_Data_In.m_TimeStamp = timestamp;
				break;
			}
			case MIDI_COMMAND_SYSTEM_COMMON_UNDEFINED_1:
			case MIDI_COMMAND_SYSTEM_COMMON_UNDEFINED_2:
			case MIDI_COMMAND_SYSTEM_COMMON_TUNE_REQUEST:
			{
			//A new command -> length: 1 bytes
				m_Data_In.m_Data[0] = data_byte;
				m_Data_In.m_Data_size = 1;

				m_Data_In.m_Command = data_byte;
				m_Data_In.m_Command_bytesNeeded = 0;

				m_Data_In.m_Command_old = 0;

				m_Data_In.m_TimeStamp = timestamp;

			//As message is complete we can send it to the system.
				CopyBuffer_MME_In_SendCommand( &m_Data_In );

			//Reset command.
				m_Data_In.m_Data_size = 0;
				m_Data_In.m_Command = 0;
				m_Data_In.m_Command_old = 0;
				break;
			}
			case MIDI_COMMAND_SYSTEM_REALTIME_TIMING_CLOCK:
			case MIDI_COMMAND_SYSTEM_REALTIME_UNDEFINED_3:
			case MIDI_COMMAND_SYSTEM_REALTIME_START:
			case MIDI_COMMAND_SYSTEM_REALTIME_CONTINUE:
			case MIDI_COMMAND_SYSTEM_REALTIME_STOP:
			case MIDI_COMMAND_SYSTEM_REALTIME_UNDEFINED_4:
			case MIDI_COMMAND_SYSTEM_REALTIME_ACTIVE_SENSING:
			case MIDI_COMMAND_SYSTEM_REALTIME_RESET:
			{
			//A new realtime command -> length: 1 bytes
				midiCommand_s		data_In_realTime;

				data_In_realTime.m_Data[0] = data_byte;
				data_In_realTime.m_Data_size = 1;

				data_In_realTime.m_Command = data_byte;
				data_In_realTime.m_Command_bytesNeeded = 0;

				data_In_realTime.m_Command_old = 0;

				data_In_realTime.m_TimeStamp = timestamp;

			//As message is complete we can send it to the system.
				CopyBuffer_MME_In_SendCommand( &data_In_realTime );
				break;
			}
			default:
			{
			//Can't really happen, but is here just to be sure.
				break;
			}
			}
			break;
		}
		default:
		{
		//Data byte -> does it belong to a waiting command, an old command or is a leftover of a message before MIDI channel got opened?
			if ( m_Data_In.m_Command != 0 )
			{
			//It's part of the command -> append byte to the end of buffer.
				m_Data_In.m_Data[m_Data_In.m_Data_size] = data_byte;
				++m_Data_In.m_Data_size;

				--m_Data_In.m_Command_bytesNeeded;

			//Is message complete.
				if ( m_Data_In.m_Command_bytesNeeded <= 0 )
				{
				//Message complete -> send it to the system.
					CopyBuffer_MME_In_SendCommand( &m_Data_In );

				//Reset command.
				//
				//Notice:
				//System Exclusive Message can span on multiple buffers.
					if ( m_Data_In.m_Command != MIDI_COMMAND_SYSTEM_COMMON_EXCLUSIVE_START )
					{
					//A normal command -> full reset.
						m_Data_In.m_Command_old = m_Data_In.m_Command;

						m_Data_In.m_Data_size = 0;
						m_Data_In.m_Command = 0;
					}
					else
					{
					//It's an exclusive message -> partial reset.
						m_Data_In.m_Data_size = 0;
						m_Data_In.m_Command_bytesNeeded = 12;
						m_Data_In.m_TimeStamp = 3200 * 12;		//advance timestamp for 12 bytes
					}
				}
			}
			else if ( m_Data_In.m_Command_old != 0 )
			{
			//It's an old command -> "set up the new command".
			//
			//Notice:
			//We'll do a little for loop patching so that we wont have to double the code.
				//Shift all the remaining bytes 1 byte to the right.
				for ( unsigned long j = i; j < data_size; j++ )
				{
					data[j+1] = data[j];
				}
				++data_size;

				//Insert command byte.
				data[i] = m_Data_In.m_Command_old;
				m_Data_In.m_Command_old = 0;

				//Signal that next time the for loop starts the timestamp should not be updated.
				timestamp_update = false;

				//Move for loop 1 step back, so that on the next step we'll come to current byte again.
				--i;	
			}
			//else
			//{
			////It's a leftover -> ignore it.
			//}
			break;
		}
		}
	}

	return STATUS_SUCCESS;
}
#pragma CODE_LOCKED
void channel_midi_c::CopyBuffer_MME_In_SendCommand( midiCommand_s* pCommand )
{
//#ifdef GL_LOGGER
//	pLogger->LogFast( "New command: 0x%X; length: %d; timestamp: %.8I64d\n", pCommand->m_Data[0], pCommand->m_Data_size, pCommand->m_TimeStamp/10 );
//#endif

//Get streamPointer.
//
//Notice:
//The frame size of MME streamPointer is 20 bytes.
//So 20bytes - sizeof( KSMUSICFORMAT ) = 12 byte available for data
	PKSSTREAM_POINTER		pStreamPointer = KsPinGetLeadingEdgeStreamPointer( m_pPin, KSSTREAM_POINTER_STATE_LOCKED );
	if ( pStreamPointer != NULL )
	{
	//Validate.
		if ( pStreamPointer->OffsetOut.Count < 20 )
		{
		//Advance streamPointer and unlock it (force eject).
			KsStreamPointerAdvanceOffsetsAndUnlock( pStreamPointer, 0, 0, TRUE );

		//Try luck with the next streamPointer.
			pStreamPointer = KsPinGetLeadingEdgeStreamPointer( m_pPin, KSSTREAM_POINTER_STATE_LOCKED );
			if ( pStreamPointer == NULL )
			{
			//Failed to get the next one.
				return;
			}

		//Check count again...
			if ( pStreamPointer->OffsetOut.Count < 20 )
			{
			//Nothing we can do -> we'll loose MIDI message.
				return;
			}
		}

	//MME driver uses KSMUSICFORMAT.
	//
	//typedef struct
	//{
	//    ULONG  TimeDeltaMs;
	//    ULONG  ByteCount;
	//} KSMUSICFORMAT, *PKSMUSICFORMAT;
	//
	//Initialize.
		KSMUSICFORMAT*					pFormat = (KSMUSICFORMAT*)pStreamPointer->OffsetOut.Data;
		BYTE*							pData_midi = (BYTE*)( pFormat + 1 );

	//Set KSMUSICFORMAT.
		//DWORD alignment
		pStreamPointer->StreamHeader->DataUsed = ( sizeof( KSMUSICFORMAT ) + pCommand->m_Data_size + 3 ) & ( ~3 );
		pFormat->TimeDeltaMs = 0;
		pFormat->ByteCount = pCommand->m_Data_size;

	//Set data.
	//
	//We'll just do a 3 DWORD copy as that is the maximum data we can receive.
		*( (DWORD*)pData_midi + 0 ) = *( (DWORD*)&pCommand->m_Data[0] );
		*( (DWORD*)pData_midi + 1 ) = *( (DWORD*)&pCommand->m_Data[4] );
		*( (DWORD*)pData_midi + 2 ) = *( (DWORD*)&pCommand->m_Data[8] );

	//Set the streamPointer.
		pStreamPointer->StreamHeader->PresentationTime.Time = pCommand->m_TimeStamp;
		pStreamPointer->StreamHeader->PresentationTime.Numerator = 1;
		pStreamPointer->StreamHeader->PresentationTime.Denominator = 1;
		pStreamPointer->StreamHeader->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_TIMEVALID;

	//Advance streamPointer and unlock it (force eject).
		KsStreamPointerAdvanceOffsetsAndUnlock( pStreamPointer, 0, pStreamPointer->StreamHeader->DataUsed, TRUE );
	}
}

#pragma CODE_LOCKED
NTSTATUS channel_midi_c::CopyBuffer_MME_Out( ULONGLONG timestamp )
{
	NTSTATUS	Status = STATUS_SUCCESS;

//Initialize.
	PUCHAR		IoUart_MSR = m_pCard->Get_IoUart() + UART_MSR;
	PUCHAR		IoUart_THR = m_pCard->Get_IoUart() + UART_THR;

//Is there any space in output FIFO?
	if ( ( ReadByte( IoUart_MSR ) & UART_MSR_DSR ) == 0 )
	{
	//There is no space in output FIFO -> skip current call.
		Status = STATUS_PENDING;
	}

//Get streamPointer.
	if ( SUCCESS( Status ) )
	{
		PKSSTREAM_POINTER		pStreamPointer = KsPinGetLeadingEdgeStreamPointer( m_pPin, KSSTREAM_POINTER_STATE_LOCKED );
		if ( pStreamPointer != NULL )
		{
		//Initialize variables.
			PKSSTREAM_POINTER_OFFSET		pOffset = pStreamPointer->Offset;
			KSMUSICFORMAT*					pFormat = (KSMUSICFORMAT*)pOffset->Data;
			BYTE*							pData_midi = (BYTE*)( pFormat + 1 );
			unsigned long					data_size = pFormat->ByteCount;

		//#ifdef GL_LOGGER
		//	unsigned long	dataUsed = ( sizeof( KSMUSICFORMAT ) + pFormat->ByteCount + 3 ) & ( ~3 );
		//	pLogger->LogFast(
		//	  "O: %d(%d)\n",
		//	  pOffset->Count,
		//	  pOffset->Count / dataUsed
		//	);
		//#endif

		//Transfer data to UART.
		//
		//Notice:
		//m_Data_LastOffset_Out is offset from previous DPC call and if it's not 0 it means that not all data was transfered.
			while ( m_Data_Out_lastOffset < data_size )
			{
			//#ifdef GL_LOGGER
			//	pLogger->LogFast( "OUT: 0x%X\n", pData_midi[m_Data_Out_lastOffset] );
			//#endif

			//Transfer data.
				WriteByte( IoUart_THR, pData_midi[m_Data_Out_lastOffset] );

			//Update.
				++m_Data_Out_lastOffset;

			//Is there any more space left in output FIFO?
				if ( ( ReadByte( IoUart_MSR ) & UART_MSR_DSR ) == 0 )
				{
				//No... stop transfering of data.
					break;
				}
			}

		//Release stream pointer.
			if ( m_Data_Out_lastOffset >= data_size )
			{
			//All the data was transfered and streamPointer should advance.
				KsStreamPointerAdvanceOffsetsAndUnlock( pStreamPointer, pOffset->Count, 0, FALSE );

			//Reset offset.
				m_Data_Out_lastOffset = 0;

			//All was transfered in current call -> return SUCCESS to the caller.
				Status = STATUS_SUCCESS;
			}
			else
			{
			//Not all data was transfered. Just unlock the streamPointer without any advancement.
				KsStreamPointerAdvanceOffsetsAndUnlock( pStreamPointer, 0, 0, FALSE );

			//If we are here then the transfer was stopped because there was not enough space in UART.
			//So stop the function as nothing else can be done.
				Status = STATUS_PENDING;
			}
		}
		else
		{
		//Failed to get streamPointer -> move it into PENDING state and leave the timer DPC to handle it.
		//
		//This can also handle the first time the Process function is called... there are no streams available,
		//but system calls the procedure to inform it of channel start -> it requires STATUS_PENDING as return.
			Status = STATUS_PENDING;
		}
	}

	return Status;
}

#pragma CODE_LOCKED
NTSTATUS channel_midi_c::CopyBuffer_DX_In( ULONGLONG timestamp )
{
//#ifdef GL_LOGGER
//	pLogger->LogFast( "DI\n" );
//#endif

//Initialize.
	PUCHAR		IoUart_LSR = m_pCard->Get_IoUart() + UART_LSR;
	PUCHAR		IoUart_RBR = m_pCard->Get_IoUart() + UART_RBR;

//Is there anything new in input FIFO?
	if ( ( ReadByte( IoUart_LSR ) & UART_LSR_DR ) == 0 )
	{
	//There is nothing new in input FIFO -> skip current call.
		return STATUS_PENDING;
	}

//Get all the new data from UART.
//
//Notice:
//LSR DR will be high when ever there is data in the input FIFO.
//
//Notice:
//On todays computers we'll more or less get just 1 byte of data, as ISR and DPC are really fast.
//Well it depends on how much CPU resources are used.
//
//Notice:
//As data can come without any command bytes we'll have a secondary space available in the buffer in case we'll have to
//insert command bytes also.
	BYTE			data[20+20/2];
	unsigned long	data_size = 0;

	while ( ( ReadByte( IoUart_LSR ) & UART_LSR_DR ) != 0 )
	{
	//Is buffer full?
	//
	//Notice:
	//There will never be 20 bytes available, so this is here just to make sure that some error wont cause driver crash.
		if ( data_size >= 20 )
		{
		//No more space -> leave rest for the next time.
			break;
		}

	//There is data available -> read it to driver buffer.
		data[data_size] = ReadByte( IoUart_RBR );
		++data_size;
	}
//#ifdef GL_LOGGER
//	pLogger->LogFast( "DI: %d\n", data_size );
//#endif
//Update timestamp.
//
//As timestamp was acquired in ISR just after the first byte interrupt flag was signaled, all we have to do is to
//substract the 320us from it and we'll have a timestamp of precision of +-320us.
//
//Notice:
//Timestamp is in 100ns -> 320us /100ns = 320000 ns/100ns = 3200
	timestamp -= 3200;
	
	//The timestamp is already correct -> skip the first update.
	bool	timestamp_update = false;	

//Process retrieved data.
	for ( unsigned long i = 0; i < data_size; i++ )
	{
	//Update timestamp.
		if ( timestamp_update == true )
		{
			timestamp += 3200;
		}
		else
		{
		//Skip just one timestamp update event.
			timestamp_update = true;
		}

	//Get data byte.
		BYTE	data_byte = data[i];
	//#ifdef GL_LOGGER
	//	pLogger->LogFast( "I: 0x%X\n", data_byte );
	//#endif

	//Process new data byte.
		switch ( data_byte >> 4 )
		{
		case 0xC:	//Program Change
		case 0xD:	//Channel Pressure (After-touch)
		{
		//A new command -> length: 2 bytes
			m_Data_In.m_Data[0] = data_byte;
			++m_Data_In.m_Data_size;

			m_Data_In.m_Command = data_byte;
			m_Data_In.m_Command_bytesNeeded = 1;	//1 more byte needed

			m_Data_In.m_Command_old = 0;

			m_Data_In.m_TimeStamp = timestamp;
			break;
		}
		case 0x8:	//Note Off
		case 0x9:	//Note On
		case 0xA:	//Polyphonic Key Pressure (Aftertouch)
		case 0xB:	//Control Change
		case 0xE:	//Pitch Wheel Change
		{
		//A new command -> length: 3 bytes
			m_Data_In.m_Data[0] = data_byte;
			m_Data_In.m_Data_size = 1;

			m_Data_In.m_Command = data_byte;
			m_Data_In.m_Command_bytesNeeded = 2;	//2 more byte needed

			m_Data_In.m_Command_old = 0;

			m_Data_In.m_TimeStamp = timestamp;
			break;
		}
		case 0xF:	//System Common Messages or System Real-Time Messages
		{
			switch ( data_byte )
			{
			case MIDI_COMMAND_SYSTEM_COMMON_EXCLUSIVE_START:
			{
			//A new command -> length: n bytes or MIDI_COMMAND_SYSTEM_EXCLUSIVE_END
				m_Data_In.m_Data[0] = data_byte;
				++m_Data_In.m_Data_size;

				m_Data_In.m_Command = data_byte;
				m_Data_In.m_Command_bytesNeeded = 43;	//we still need 43 bytes to fill up current buffer

				m_Data_In.m_Command_old = 0;

				m_Data_In.m_TimeStamp = timestamp;
				break;
			}
			case MIDI_COMMAND_SYSTEM_COMMON_EXCLUSIVE_END:
			{
			//End of system exlusive command -> send the remaining data to system.
				//Append byte to the end of buffer.
				m_Data_In.m_Data[m_Data_In.m_Data_size] = data_byte;
				++m_Data_In.m_Data_size;

				//Send it to the system.
				CopyBuffer_MME_In_SendCommand( &m_Data_In );

			//Reset command.
				m_Data_In.m_Data_size = 0;
				m_Data_In.m_Command = 0;
				m_Data_In.m_Command_old = 0;
				break;
			}
			case MIDI_COMMAND_SYSTEM_COMMON_MIDI_TIME_CODE:
			case MIDI_COMMAND_SYSTEM_COMMON_SONG_SELECT:
			{
			//A new command -> length: 2 bytes
				m_Data_In.m_Data[0] = data_byte;
				m_Data_In.m_Data_size = 1;

				m_Data_In.m_Command = data_byte;
				m_Data_In.m_Command_bytesNeeded = 1;	//1 more byte needed

				m_Data_In.m_Command_old = 0;

				m_Data_In.m_TimeStamp = timestamp;
				break;
			}
			case MIDI_COMMAND_SYSTEM_COMMON_SONG_POSITION:
			{
			//A new command -> length: 3 bytes
				m_Data_In.m_Data[0] = data_byte;
				m_Data_In.m_Data_size = 1;

				m_Data_In.m_Command = data_byte;
				m_Data_In.m_Command_bytesNeeded = 2;	//2 more byte needed

				m_Data_In.m_Command_old = 0;

				m_Data_In.m_TimeStamp = timestamp;
				break;
			}
			case MIDI_COMMAND_SYSTEM_COMMON_UNDEFINED_1:
			case MIDI_COMMAND_SYSTEM_COMMON_UNDEFINED_2:
			case MIDI_COMMAND_SYSTEM_COMMON_TUNE_REQUEST:
			{
			//A new command -> length: 1 bytes
				m_Data_In.m_Data[0] = data_byte;
				m_Data_In.m_Data_size = 1;

				m_Data_In.m_Command = data_byte;
				m_Data_In.m_Command_bytesNeeded = 0;

				m_Data_In.m_Command_old = 0;

				m_Data_In.m_TimeStamp = timestamp;

			//As message is complete we can send it to the system.
				CopyBuffer_MME_In_SendCommand( &m_Data_In );

			//Reset command.
				m_Data_In.m_Data_size = 0;
				m_Data_In.m_Command = 0;
				m_Data_In.m_Command_old = 0;
				break;
			}
			case MIDI_COMMAND_SYSTEM_REALTIME_TIMING_CLOCK:
			case MIDI_COMMAND_SYSTEM_REALTIME_UNDEFINED_3:
			case MIDI_COMMAND_SYSTEM_REALTIME_START:
			case MIDI_COMMAND_SYSTEM_REALTIME_CONTINUE:
			case MIDI_COMMAND_SYSTEM_REALTIME_STOP:
			case MIDI_COMMAND_SYSTEM_REALTIME_UNDEFINED_4:
			case MIDI_COMMAND_SYSTEM_REALTIME_ACTIVE_SENSING:
			case MIDI_COMMAND_SYSTEM_REALTIME_RESET:
			{
			//A new realtime command -> length: 1 bytes
				midiCommand_s		data_In_realTime;

				data_In_realTime.m_Data[0] = data_byte;
				data_In_realTime.m_Data_size = 1;

				data_In_realTime.m_Command = data_byte;
				data_In_realTime.m_Command_bytesNeeded = 0;

				data_In_realTime.m_Command_old = 0;

				data_In_realTime.m_TimeStamp = timestamp;

			//As message is complete we can send it to the system.
				CopyBuffer_MME_In_SendCommand( &data_In_realTime );
				break;
			}
			default:
			{
			//Can't really happen, but is here just to be sure.
				break;
			}
			}
			break;
		}
		default:
		{
		//Data byte -> does it belong to a waiting command, an old command or is a leftover of a message before MIDI channel got opened?
			if ( m_Data_In.m_Command != 0 )
			{
			//It's part of the command -> append byte to the end of buffer.
				m_Data_In.m_Data[m_Data_In.m_Data_size] = data_byte;
				++m_Data_In.m_Data_size;

				--m_Data_In.m_Command_bytesNeeded;

			//Is message complete.
				if ( m_Data_In.m_Command_bytesNeeded <= 0 )
				{
				//Message complete -> send it to the system.
					CopyBuffer_MME_In_SendCommand( &m_Data_In );

				//Reset command.
				//
				//Notice:
				//System Exclusive Message can span on multiple buffers.
					if ( m_Data_In.m_Command != MIDI_COMMAND_SYSTEM_COMMON_EXCLUSIVE_START )
					{
					//A normal command -> full reset.
						m_Data_In.m_Command_old = m_Data_In.m_Command;

						m_Data_In.m_Data_size = 0;
						m_Data_In.m_Command = 0;
					}
					else
					{
					//It's an exclusive message -> partial reset.
						m_Data_In.m_Data_size = 0;
						m_Data_In.m_Command_bytesNeeded = 12;
						m_Data_In.m_TimeStamp = 3200 * 12;		//advance timestamp for 12 bytes
					}
				}
			}
			else if ( m_Data_In.m_Command_old != 0 )
			{
			//It's an old command -> "set up the new command".
			//
			//Notice:
			//We'll do a little for loop patching so that we wont have to double the code.
				//Shift all the remaining bytes 1 byte to the right.
				for ( unsigned long j = i; j < data_size; j++ )
				{
					data[j+1] = data[j];
				}
				++data_size;

				//Insert command byte.
				data[i] = m_Data_In.m_Command_old;
				m_Data_In.m_Command_old = 0;

				//Signal that next time the for loop starts the timestamp should not be updated.
				timestamp_update = false;

				//Move for loop 1 step back, so that on the next step we'll come to current byte again.
				--i;	
			}
			//else
			//{
			////It's a leftover -> ignore it.
			//}
			break;
		}
		}
	}

	return STATUS_SUCCESS;
}
#pragma CODE_LOCKED
void channel_midi_c::CopyBuffer_DX_In_SendCommand( midiCommand_s* pCommand )
{
//#ifdef GL_LOGGER
//	pLogger->LogFast( "New command: 0x%X; length: %d; timestamp: %.8I64d\n", pCommand->m_Data[0], pCommand->m_Data_size, pCommand->m_TimeStamp/10 );
//#endif

//Get streamPointer.
//
//Notice:
//There is (or was) a bug in MIDI_in thread of dmusic.dll.
//If we define DataUsed = DMUS_EVENT_SIZE(MidiMessageSize) and cbEvent = MidiMessageSize the bug wont be fired.
//
//So there are various ways to get around it:
//- one is to use the whole frame that streamPointer supplies us:
//  The frame size of streamPointer is 64 bytes and it contains 2 DX events.
//  As we want to send data as fast as we can we'll set the second event as
//  active sensing event.
//
//  1 event memory size is: 32bytes - sizeof( DMUS_EVENTHEADER ) = 12bytes for data
//
//  What's wrong with this one?
//  Well we are sending more data that is needed and more all midi loop test
//  applications will fail on the driver(because of the active sensing event).
//
//- second way (proposed by the Stephan Kappertz, well the first was too:) )
//  is to use stream pointer clones and it goes like this:
//  Get streamPointer->make a clone->advance the streamPointer and unlock it.
//  Now because of this advancement the leading streamPointer isn't the leading
//  anymore and because of this we can delete the clone streamPointer when
//  we'll finish with it(as it looks advancement doesn't unlock it).
//
//  Now the second part of the process is to fill the clone pointer with only
//  1 event and delete it.
//  The data from the clone will then be "pasted" to the main streamPointer and
//  returned to the DX driver. 
//
//  Well I've done some modification to this clone technique and came up with the
//  code that does all his clone thing on the streamPointer it self.
//  In the folowing code the maximum data size is 44bytes.
//  DMUS_EVENT_SIZE( 44 ) = 64bytes
//
	PKSSTREAM_POINTER		pStreamPointer = KsPinGetLeadingEdgeStreamPointer( m_pPin, KSSTREAM_POINTER_STATE_LOCKED );
	if ( pStreamPointer != NULL )
	{
	//Validate.
		if ( pStreamPointer->OffsetOut.Count < 64 )
		{
		//Advance streamPointer and unlock it (force eject).
			KsStreamPointerAdvanceOffsetsAndUnlock( pStreamPointer, 0, 0, TRUE );

		//Try luck with the next streamPointer.
			pStreamPointer = KsPinGetLeadingEdgeStreamPointer( m_pPin, KSSTREAM_POINTER_STATE_LOCKED );
			if ( pStreamPointer == NULL )
			{
			//Failed to get the next one.
				return;
			}

		//Check count again...
			if ( pStreamPointer->OffsetOut.Count < 64 )
			{
			//Nothing we can do -> we'll loose MIDI message.
				return;
			}
		}

	//DX driver uses DMUS_EVENTHEADER.
	//
	//typedef struct _DMUS_EVENTHEADER
	//{
	//    DWORD           cbEvent;                /* Unrounded bytes in event */
	//    DWORD           dwChannelGroup;         /* Channel group of event */
	//    REFERENCE_TIME  rtDelta;                /* Delta from start time of entire buffer */
	//    DWORD           dwFlags;                /* Flags DMUS_EVENT_xxx */
	//} DMUS_EVENTHEADER;
	//
	//Initialize.
		DMUS_EVENTHEADER*				pFormat = (DMUS_EVENTHEADER*)pStreamPointer->OffsetOut.Data;
		BYTE*							pData_midi = (BYTE*)( pFormat + 1 );

	//Set DMUS_EVENTHEADER.
	//
	//Notice:
	//All except SYSEX messages are structured.
		pStreamPointer->StreamHeader->DataUsed = DMUS_EVENT_SIZE( pCommand->m_Data_size );

		pFormat->cbEvent = pCommand->m_Data_size;
		pFormat->dwChannelGroup = 0;
		pFormat->rtDelta = 0;
		pFormat->dwFlags = ( pCommand->m_Command == MIDI_COMMAND_SYSTEM_COMMON_EXCLUSIVE_START ) ? 0 : DMUS_EVENT_STRUCTURED;

	//Set data.
	//
	//We'll just do a 4 DWORD copy as that is the maximum data we can receive.
		if ( pCommand->m_Data_size > 4 )
		{
			RtlCopyBytes( pData_midi, &pCommand->m_Data[0], pCommand->m_Data_size );
		}
		else
		{
			*( (DWORD*)pData_midi + 0 ) = *( (DWORD*)&pCommand->m_Data[0] );
		}

	//Set the streamPointer.
		pStreamPointer->StreamHeader->PresentationTime.Time = pCommand->m_TimeStamp;
		pStreamPointer->StreamHeader->PresentationTime.Numerator = 1;
		pStreamPointer->StreamHeader->PresentationTime.Denominator = 1;
		pStreamPointer->StreamHeader->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_TIMEVALID;

	//Advance streamPointer and unlock it (force eject).
		KsStreamPointerAdvanceOffsetsAndUnlock( pStreamPointer, 0, 0, TRUE );
	}
}
#pragma CODE_LOCKED
NTSTATUS channel_midi_c::CopyBuffer_DX_Out( ULONGLONG timestamp )
{
//#ifdef GL_LOGGER
//	pLogger->LogFast( "DO\n" );
//#endif

	NTSTATUS	Status = STATUS_SUCCESS;

//Initialize.
	PUCHAR		IoUart_MSR = m_pCard->Get_IoUart() + UART_MSR;
	PUCHAR		IoUart_THR = m_pCard->Get_IoUart() + UART_THR;

//Is there any space in output FIFO?
	if ( ( ReadByte( IoUart_MSR ) & UART_MSR_DSR ) == 0 )
	{
	//There is no space in output FIFO -> skip current call.
		Status = STATUS_PENDING;
	}

//Get streamPointer.
	if ( SUCCESS( Status ) )
	{
		PKSSTREAM_POINTER		pStreamPointer = KsPinGetLeadingEdgeStreamPointer( m_pPin, KSSTREAM_POINTER_STATE_LOCKED );
		if ( pStreamPointer != NULL )
		{
		//Initialize variables.
			PKSSTREAM_POINTER_OFFSET		pOffset = pStreamPointer->Offset;
			DMUS_EVENTHEADER*				pFormat = (DMUS_EVENTHEADER*)pOffset->Data;
			BYTE*							pData_midi = (BYTE*)( pFormat + 1 );
			unsigned long					data_size = pFormat->cbEvent;

		//#ifdef GL_LOGGER
		//	unsigned long	dataUsed = ( sizeof( KSMUSICFORMAT ) + pFormat->ByteCount + 3 ) & ( ~3 );
		//	pLogger->LogFast(
		//	  "O: %d(%d)\n",
		//	  pOffset->Count,
		//	  pOffset->Count / dataUsed
		//	);
		//#endif

		//Transfer data to UART.
		//
		//Notice:
		//m_Data_LastOffset_Out is offset from previous DPC call and if it's not 0 it means that not all data was transfered.
			while ( m_Data_Out_lastOffset < data_size )
			{
			//#ifdef GL_LOGGER
			//	pLogger->LogFast( "OUT: 0x%X\n", pData_midi[m_Data_Out_lastOffset] );
			//#endif

			//Transfer data.
				WriteByte( IoUart_THR, pData_midi[m_Data_Out_lastOffset] );

			//Update.
				++m_Data_Out_lastOffset;

			//Is there any more space left in output FIFO?
				if ( ( ReadByte( IoUart_MSR ) & UART_MSR_DSR ) == 0 )
				{
				//No... stop transfering of data.
					break;
				}
			}

		//Release stream pointer.
			if ( m_Data_Out_lastOffset >= data_size )
			{
			//All the data was transfered and streamPointer should advance.
				KsStreamPointerAdvanceOffsetsAndUnlock( pStreamPointer, pOffset->Count, 0, FALSE );

			//Reset offset.
				m_Data_Out_lastOffset = 0;

			//All was transfered in current call -> return SUCCESS to the caller.
				Status = STATUS_SUCCESS;
			}
			else
			{
			//Not all data was transfered. Just unlock the streamPointer without any advancement.
				KsStreamPointerAdvanceOffsetsAndUnlock( pStreamPointer, 0, 0, FALSE );

			//If we are here then the transfer was stopped because there was not enough space in UART.
			//So stop the function as nothing else can be done.
				Status = STATUS_PENDING;
			}
		}
		else
		{
		//Failed to get streamPointer -> move it into PENDING state and leave the timer DPC to handle it.
		//
		//This can also handle the first time the Process function is called... there are no streams available,
		//but system calls the procedure to inform it of channel start -> it requires STATUS_PENDING as return.
			Status = STATUS_PENDING;
		}
	}

	return Status;
}
