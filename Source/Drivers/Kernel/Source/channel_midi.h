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
// PURPOSE:
//      File defines channel class. It's intention is to handle all channel
//      related stuff.
//
// STATUS:
//      Unfinished.
//
//***************************************************************************
//***************************************************************************

#pragma once

//Includes...
#include "main_defines.h"
#include "port.h"

struct midiCommand_s
{
//MIDI data
//
//MME: can handle 12 bytes.
//DX:  can handle 44 bytes.
//
	BYTE			m_Data[44];
	BYTE 			m_Data_size;

//MIDI Command... current command that is processing.
	BYTE			m_Command;
	BYTE			m_Command_bytesNeeded;

	BYTE			m_Command_old;				//in case that MIDI device sends just next data without the command byte, we need to save the

//Timestamp of first byte in m_Data.
	ULONGLONG		m_TimeStamp;
};

class card_c;

class channel_midi_c : public port_c
{
	friend class logger_c;

//--------------------------------------------------------------------------
	public: channel_midi_c( IN card_c* pCard, IN bool isInput );
	//public: ~channel_midi_c(void);

//--------------------------------------------------------------------------
//Class memory
	#define MEMORY_TAG_CLASS_CHANNEL_MIDI		'IMHC'	//tag: "CHMI"

	public: static void* __cdecl operator new( size_t size );
	public: static void __cdecl operator delete( void* pMemory );

//--------------------------------------------------------------------------
//Owner
	public: card_c*						m_pCard;

//--------------------------------------------------------------------------
//Types
	public: enum CHANNEL_TYPE
	{
		CHANNEL_TYPE_IN,
		CHANNEL_TYPE_OUT
	};
	public: CHANNEL_TYPE				m_Type;

	public: enum CHANNEL_TYPE_CLIENT
	{
		CHANNEL_TYPE_CLIENT_NON,		//default value
		CHANNEL_TYPE_CLIENT_MME,
		CHANNEL_TYPE_CLIENT_DX
	};
	public: CHANNEL_TYPE_CLIENT			m_Type_client;

//--------------------------------------------------------------------------
//Channel state (KSSTATE_STOP, KSSTATE_ACQUIRE, KSSTATE_PAUSE, KSSTATE_RUN)
	public: KSSTATE						m_State;

//--------------------------------------------------------------------------
//Samples clock definitions.
//
//Notice:
//All clocks/timestamps are dereived from master class clock.
//	public: sampleClock_s				m_Clock_playOffset;						//KSAUDIO_POSITION->PlayOffset for inputs
//
//	public: sampleClock_s				m_Clock_writeOffset;					//look at KSAUDIO_POSITION->WriteOffset... it looks from system buffer point of view
//	public: unsigned short				m_Clock_writeOffset_newStreamPointer;	//YES/NO

//	public: sampleClock_s				m_Clock_timestamp_streamPause;			//it will be used for KSAUDIO_POSITION->PlayOffset for outputs; 0 = ignore; 1 = set PAUSE timestamp; > 1 = timestamp
//	public: sampleClock_s				m_Clock_timestamp_streamStart;			//it will be used for KSAUDIO_POSITION->PlayOffset for outputs
//	public: unsigned short				m_Clock_timestamp_streamStart_isSet;	//YES/NO
//
//	public: unsigned long				m_Clock_samplesLost;

//--------------------------------------------------------------------------
//AVStream variables
	public: __forceinline NTSTATUS Set_AVStream_Pin( PKSPIN pPin )
	{
		NTSTATUS		Status = STATUS_SUCCESS;

	//Are we allowed to do this?
		if ( m_State == KSSTATE_STOP )
		{
			m_pPin = pPin;
		}
		else
		{
			Status = STATUS_UNSUCCESSFUL;
		}

		return Status;
	}

	protected: PKSPIN					m_pPin;

//--------------------------------------------------------------------------
//Process buffers...
	public: NTSTATUS Set_CopyBuffersProcedure();

	typedef NTSTATUS ( channel_midi_c::*PCOPY_BUFFER_MIDI )( ULONGLONG ) const;

	public: PCOPY_BUFFER_MIDI			m_pCopyBuffersProcedure;

	private: NTSTATUS CopyBuffer_MME_In( ULONGLONG timestamp );
	private: NTSTATUS CopyBuffer_MME_Out( ULONGLONG timestamp );
	private: NTSTATUS CopyBuffer_DX_In( ULONGLONG timestamp );
	private: NTSTATUS CopyBuffer_DX_Out( ULONGLONG timestamp );

	private: void CopyBuffer_MME_In_SendCommand( midiCommand_s* pCommand );
	private: void CopyBuffer_DX_In_SendCommand( midiCommand_s* pCommand );

	public: midiCommand_s				m_Data_In;		//this is used only if command was not fully received
	public: unsigned long				m_Data_Out_lastOffset;

	//public: ULONGLONG					m_Timestamp_streamPause;			//it will be used for KSAUDIO_POSITION->PlayOffset for outputs
	//public: ULONGLONG					m_Timestamp_streamStart;			//it will be used for KSAUDIO_POSITION->PlayOffset for outputs
	//public: unsigned long				m_Timestamp_streamStart_isSet;		//YES/NO
};
