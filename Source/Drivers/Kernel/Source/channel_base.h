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


class card_c;

class channel_base_c : public port_c
{
	friend class logger_c;

//--------------------------------------------------------------------------
	public: channel_base_c( IN card_c* pCard, IN bool isInput, IN bool isStereo, IN unsigned long channelSize_inSamples );
	//public: ~channel_base_c(void);

//--------------------------------------------------------------------------
//Class memory
	#define MEMORY_TAG_CLASS_CHANNEL_BASE		'ABHC'	//tag: "CHBA"

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

	public: enum CHANNEL_FORMAT
	{
		CHANNEL_FORMAT_STEREO,
		CHANNEL_FORMAT_SURROUND
	};
	public: CHANNEL_FORMAT				m_Format;

	public: enum CHANNEL_TYPE_CLIENT
	{
		CHANNEL_TYPE_CLIENT_NON,	//default value
		CHANNEL_TYPE_CLIENT_AVSTREAM,
		CHANNEL_TYPE_CLIENT_ASIO
	};
	public: CHANNEL_TYPE_CLIENT			m_Type_client;

	public: enum CHANNEL_BIT_SIZE
	{
		CHANNEL_BIT_SIZE_16,
		CHANNEL_BIT_SIZE_24,		//default value
		CHANNEL_BIT_SIZE_32
	};
	public: CHANNEL_BIT_SIZE			m_BitSize;

//--------------------------------------------------------------------------
//Byte <-> Sample conversions.
	#define bytesToSamples32( variable ) ( variable/=4 )
	#define samplesToBytes32( variable ) ( variable*=4 )
	
	#define bytesToSamples24( variable ) ( variable/=3 )
	#define samplesToBytes24( variable ) ( variable*=3 )
	
	#define bytesToSamples16( variable ) ( variable/=2 )
	#define samplesToBytes16( variable ) ( variable*=2 )

//--------------------------------------------------------------------------
//Channel state (KSSTATE_STOP, KSSTATE_ACQUIRE, KSSTATE_PAUSE, KSSTATE_RUN)
	public: KSSTATE						m_State;

//--------------------------------------------------------------------------
//Addresses
	protected: virtual void Set_Buffer_AVStream( PULONG pBuffer );
	protected: virtual void Set_HardwareAddress( unsigned long offsetInSamples );

	protected: unsigned long			m_Channel_size;		//in samples

//--------------------------------------------------------------------------
//Samples clock definitions.
//
//Notice:
//All clocks/timestamps are dereived from master class clock.
	public: sampleClock_s				m_Clock_playOffset;						//KSAUDIO_POSITION->PlayOffset for inputs

	public: sampleClock_s				m_Clock_writeOffset;					//look at KSAUDIO_POSITION->WriteOffset... it looks from system buffer point of view
	public: unsigned short				m_Clock_writeOffset_newStreamPointer;	//YES/NO

	public: sampleClock_s				m_Clock_timestamp_streamPause;			//it will be used for KSAUDIO_POSITION->PlayOffset for outputs; 0 = ignore; 1 = set PAUSE timestamp; > 1 = timestamp
	public: sampleClock_s				m_Clock_timestamp_streamStart;			//it will be used for KSAUDIO_POSITION->PlayOffset for outputs
	public: unsigned short				m_Clock_timestamp_streamStart_isSet;	//YES/NO

	public: unsigned long				m_Clock_samplesLost;

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

	public: unsigned long				m_BufferSize_copyInCurrentPass;			//in samples... number of samples in AVStream buffer -> dataPerChannel*nChannels
	public: unsigned long				m_BufferSize_nChannels_inSamples;		//nChannels * bufferSize

//--------------------------------------------------------------------------
//Process buffers...
	public: void TransferData_AVStream( unsigned long offset_inSamples, ULONGLONG clock_masterCard, unsigned long clock_samplesLost, unsigned long sampleRate );
	public: void TransferData_ASIO( unsigned long offset_inSamples );

	public: virtual NTSTATUS Set_CopyBuffersProcedure_AVStream( unsigned long nChannels, CHANNEL_BIT_SIZE nBitsInSample );

	public: virtual NTSTATUS ZeroChannelMemory( void );
	public: NTSTATUS ZeroChannelMemory_Full( void );

	typedef void (*PCOPY_BUFFER_WAVE)( channel_base_c* pChannel );
	public: PCOPY_BUFFER_WAVE				m_pCopyBuffersProcedure;

	public: unsigned long				m_nChannels;
};


