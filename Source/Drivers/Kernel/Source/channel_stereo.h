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
#include "channel_base.h"



class channel_stereo_c : public channel_base_c
{
	friend class asioHandle_c;
	friend class logger_c;

//--------------------------------------------------------------------------
	public: channel_stereo_c( IN card_c* pCard, IN bool isInput, IN unsigned long channelSize_inSamples, IN unsigned long channelIndex, IN PULONG pAddressOnCard_left, IN PULONG pAddressOnCard_right );
	//private: ~channel_stereo_c(void);

//--------------------------------------------------------------------------
//Index.
	private: unsigned long				m_Index;

//--------------------------------------------------------------------------
//Addresses.
	protected: virtual void Set_Buffer_AVStream( PULONG pBuffer );
	protected: virtual void Set_HardwareAddress( unsigned long offsetInSamples );

	public: void Set_Buffers_ASIO( PULONG pBuffer_left_A, PULONG pBuffer_left_B, PULONG pBuffer_right_A, PULONG pBuffer_right_B );

	private: PULONG						m_pAddressOnCard_root_left;		//root hardware address
	private: PULONG						m_pAddressOnCard_root_right;	//root hardware address

	private: PULONG						m_pAddressOnCard_left;			//current hardware address
	private: PULONG						m_pAddressOnCard_right;			//current hardware address

	private: PULONG						m_pBuffer_Left_A;				//software buffer (notice: also used for WDM driver)
	private: PULONG						m_pBuffer_Left_B;				//software buffer

	private: PULONG						m_pBuffer_Right_A;				//software buffer
	private: PULONG						m_pBuffer_Right_B;				//software buffer

	private: unsigned long				m_BufferSelect;					//use selected buffer in copy procedure... only for ASIO...
#define CHANNEL_STEREO_USE_BUFFER_A		0
#define CHANNEL_STEREO_USE_BUFFER_B		1

//--------------------------------------------------------------------------
//ASIO variables
	public: unsigned long				m_hAsio;

//--------------------------------------------------------------------------
//Process buffers AVStream...
//
//WDM, DirectX and MME formats
//
//sample container: 16bit
//MSB16 - 21  -> ..... -> 0210
//
//sample container: 24bit
//MSB24 - 321  -> ..... -> 0321
//
//sample container: 32bit
//MSB16 - 2100 -> shr 8 -> 0210
//MSB24 - 3210 -> shr 8 -> 0321
//
	public: virtual NTSTATUS Set_CopyBuffersProcedure_AVStream( unsigned long nChannels, CHANNEL_BIT_SIZE nBitsInSample );
	public: virtual NTSTATUS ZeroChannelMemory( void );

	private: static void CopyBuffer_AVStream_Stereo_16bit_In( channel_stereo_c* pChannel );
	private: static void CopyBuffer_AVStream_Stereo_16bit_Out( channel_stereo_c* pChannel );
	private: static void CopyBuffer_AVStream_Stereo_24bit_In( channel_stereo_c* pChannel );
	private: static void CopyBuffer_AVStream_Stereo_24bit_Out( channel_stereo_c* pChannel );
	private: static void CopyBuffer_AVStream_Stereo_32bit_In( channel_stereo_c* pChannel );
	private: static void CopyBuffer_AVStream_Stereo_32bit_Out( channel_stereo_c* pChannel );

//--------------------------------------------------------------------------
//Process buffers ASIO...
//
//sample container: 32bit
//32LSB24 - 0321 ->        -> 0321 (default)
//32LSB   - 0321 ->        -> 3210
//32MSB24 - 1230 ->        -> 0321
//
	public: NTSTATUS Set_CopyBuffersProcedure_ASIO( unsigned long sampleType );

	private: static void CopyBuffer_ASIO_Stereo_32LSB_In( channel_stereo_c* pChannel );
	private: static void CopyBuffer_ASIO_Stereo_32LSB_Out( channel_stereo_c* pChannel );
	private: static void CopyBuffer_ASIO_Stereo_32LSB24_In( channel_stereo_c* pChannel );
	private: static void CopyBuffer_ASIO_Stereo_32LSB24_Out( channel_stereo_c* pChannel );
	private: static void CopyBuffer_ASIO_Stereo_32MSB24_In( channel_stereo_c* pChannel );
	private: static void CopyBuffer_ASIO_Stereo_32MSB24_Out( channel_stereo_c* pChannel );
};
