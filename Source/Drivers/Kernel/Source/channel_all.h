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
#include "channel_base.h"


//Notice:
//Class is used only for WDM multichannel support -> ASIO will not support this.
class channel_all_c : public channel_base_c
{
	//friend class logger_c;

//--------------------------------------------------------------------------
	public: channel_all_c( IN card_c* pCard, IN bool isInput, IN unsigned long channelSize_inSamples, PULONG pAddressOnCard[8] );
	//private: ~channel_all_c(void);

//--------------------------------------------------------------------------
//Addresses.
	protected: virtual void Set_Buffer_AVStream( PULONG pBuffer );
	protected: virtual void Set_HardwareAddress( unsigned long offsetInSamples );

	private: PULONG						m_pAddressOnCard_root[8];		//hardware address; if NULL there are no more channels available
	private: PULONG						m_pAddressOnCard[8];			//hardware address; if NULL there are no more channels available

	private: PULONG						m_pBuffer_all;					//software buffer (notice: used for WDM driver)

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

	private: static void CopyBuffer_AVStream_All_16bit_In( channel_all_c* pChannel );
	private: static void CopyBuffer_AVStream_All_16bit_Out( channel_all_c* pChannel );
	private: static void CopyBuffer_AVStream_All_24bit_In( channel_all_c* pChannel );
	private: static void CopyBuffer_AVStream_All_24bit_Out( channel_all_c* pChannel );
	private: static void CopyBuffer_AVStream_All_32bit_In( channel_all_c* pChannel );
	private: static void CopyBuffer_AVStream_All_32bit_Out( channel_all_c* pChannel );
};
