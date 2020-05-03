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
//      card.cpp
//
// PURPOSE:
//      File contains the card class functions.
//
// STATUS:
//      Unfinished/Untested.
//
//***************************************************************************
//***************************************************************************
#include "main.h"
#include "channel_stereo.h"
#include "driver_ioctl_asio.h"

#pragma DATA_LOCKED


#pragma CODE_LOCKED
void channel_stereo_c::Set_Buffers_ASIO( PULONG pBuffer_left_A, PULONG pBuffer_left_B, PULONG pBuffer_right_A, PULONG pBuffer_right_B )
{
	m_pBuffer_Left_A = pBuffer_left_A;
	m_pBuffer_Left_B = pBuffer_left_B;
	m_pBuffer_Right_A = pBuffer_right_A;
	m_pBuffer_Right_B = pBuffer_right_B;
}

//***************************************************************************
// ProcedureName:
//		Set_CopyBuffersProcedure_ASIO
//
// Explanation:
//		Procedure will select proper ASIO procedure from the required type.
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
NTSTATUS channel_stereo_c::Set_CopyBuffersProcedure_ASIO
(
	unsigned long		sampleType
)
{
	NTSTATUS	Status = STATUS_SUCCESS;

//Set procedure.
	if ( m_Type == CHANNEL_TYPE_IN )
	{
	//Input...
		switch ( sampleType )
		{
		case SAMPLETYPE_32BIT_LSB24: m_pCopyBuffersProcedure = (PCOPY_BUFFER_WAVE)&CopyBuffer_ASIO_Stereo_32LSB24_In; break;
		case SAMPLETYPE_32BIT_LSB: m_pCopyBuffersProcedure = (PCOPY_BUFFER_WAVE)&CopyBuffer_ASIO_Stereo_32LSB_In; break;
		case SAMPLETYPE_32BIT_MSB24: m_pCopyBuffersProcedure = (PCOPY_BUFFER_WAVE)&CopyBuffer_ASIO_Stereo_32MSB24_In; break;
		default:
			Status = STATUS_UNSUCCESSFUL;
		}
	}
	else
	{
	//Output...
		switch ( sampleType )
		{
		case SAMPLETYPE_32BIT_LSB24: m_pCopyBuffersProcedure = (PCOPY_BUFFER_WAVE)&CopyBuffer_ASIO_Stereo_32LSB24_Out; break;
		case SAMPLETYPE_32BIT_LSB: m_pCopyBuffersProcedure = (PCOPY_BUFFER_WAVE)&CopyBuffer_ASIO_Stereo_32LSB_Out; break;
		case SAMPLETYPE_32BIT_MSB24: m_pCopyBuffersProcedure = (PCOPY_BUFFER_WAVE)&CopyBuffer_ASIO_Stereo_32MSB24_Out; break;
		default:
			Status = STATUS_UNSUCCESSFUL;
		}
	}

//Set sample size.
	if ( SUCCESS( Status ) )
	{
		m_BitSize = CHANNEL_BIT_SIZE_32;
	}

	return Status;
}

#pragma CODE_LOCKED
void channel_stereo_c::CopyBuffer_ASIO_Stereo_32LSB_In( channel_stereo_c* pChannel )
{
	PULONG		pLeft = pChannel->m_pAddressOnCard_left;
	PULONG		pRight = pChannel->m_pAddressOnCard_right;

	PULONG		pSoftware_left;
	PULONG		pSoftware_right;

	if ( pChannel->m_BufferSelect == CHANNEL_STEREO_USE_BUFFER_A )
	{
		pSoftware_left = pChannel->m_pBuffer_Left_A;
		pSoftware_right = pChannel->m_pBuffer_Right_A;
	}
	else
	{
		pSoftware_left = pChannel->m_pBuffer_Left_B;
		pSoftware_right = pChannel->m_pBuffer_Right_B;
	}

//Transfer data...
	//Left
	unsigned long	samplesToCopy = pDriver->m_BufferSize;

	do
	{
		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);
		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);
		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);
		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);

		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);
		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);
		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);
		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);

	//Update.
		samplesToCopy -= 32;	//TRANSFER_SIZE
	}
	while ( samplesToCopy > 0 );

	//Right
	samplesToCopy = pDriver->m_BufferSize;

	do
	{
		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);
		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);
		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);
		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);

		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);
		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);
		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);
		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);

	//Update.
		samplesToCopy -= 32;	//TRANSFER_SIZE
	}
	while ( samplesToCopy > 0 );

//Change sample type...
//32LSB   - 0321 ->        -> 3210
	if ( pChannel->m_BufferSelect == CHANNEL_STEREO_USE_BUFFER_A )
	{
		pSoftware_left = pChannel->m_pBuffer_Left_A;
		pSoftware_right = pChannel->m_pBuffer_Right_A;
	}
	else
	{
		pSoftware_left = pChannel->m_pBuffer_Left_B;
		pSoftware_right = pChannel->m_pBuffer_Right_B;
	}

	//Left
	samplesToCopy = pDriver->m_BufferSize;

	do
	{
		unsigned long		sample = *pSoftware_left;
		sample <<= 8;
		*(pSoftware_left++) = sample;

	//Update.
		--samplesToCopy;
	}
	while ( samplesToCopy > 0 );

	//Right
	samplesToCopy = pDriver->m_BufferSize;

	do
	{
		unsigned long		sample = *pSoftware_right;
		sample <<= 8;
		*(pSoftware_right++) = sample;

	//Update.
		--samplesToCopy;
	}
	while ( samplesToCopy > 0 );
}
#pragma CODE_LOCKED
void channel_stereo_c::CopyBuffer_ASIO_Stereo_32LSB_Out( channel_stereo_c* pChannel )
{
	PULONG		pLeft = pChannel->m_pAddressOnCard_left;
	PULONG		pRight = pChannel->m_pAddressOnCard_right;

	PULONG		pSoftware_left;
	PULONG		pSoftware_right;

	PUSHORT		port = pChannel->m_pCard->Get_IoWave();

	if ( pChannel->m_BufferSelect == CHANNEL_STEREO_USE_BUFFER_A )
	{
		pSoftware_left = pChannel->m_pBuffer_Left_A;
		pSoftware_right = pChannel->m_pBuffer_Right_A;
	}
	else
	{
		pSoftware_left = pChannel->m_pBuffer_Left_B;
		pSoftware_right = pChannel->m_pBuffer_Right_B;
	}

//Change sample type...
//32LSB   - 3210 ->        -> 0321
	//Left
	unsigned long	samplesToCopy = pDriver->m_BufferSize;

	do
	{
		unsigned long		sample = *pSoftware_left;
		sample >>= 8;
		*(pSoftware_left++) = sample;

	//Update.
		--samplesToCopy;
	}
	while ( samplesToCopy > 0 );

	//Right
	samplesToCopy = pDriver->m_BufferSize;

	do
	{
		unsigned long		sample = *pSoftware_right;
		sample >>= 8;
		*(pSoftware_right++) = sample;

	//Update.
		--samplesToCopy;
	}
	while ( samplesToCopy > 0 );

//Transfer data...
	if ( pChannel->m_BufferSelect == CHANNEL_STEREO_USE_BUFFER_A )
	{
		pSoftware_left = pChannel->m_pBuffer_Left_A;
		pSoftware_right = pChannel->m_pBuffer_Right_A;
	}
	else
	{
		pSoftware_left = pChannel->m_pBuffer_Left_B;
		pSoftware_right = pChannel->m_pBuffer_Right_B;
	}

	//Left
	samplesToCopy = pDriver->m_BufferSize;

	do
	{
		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);
		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);
		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);
		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);

		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);
		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);
		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);
		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);

		//*(pLeft-1);			//Do 1 read cycle so that Altera code will get the access to local bus.
		ReadWord( port );

	//Update.
		samplesToCopy -= 32;	//TRANSFER_SIZE
	}
	while ( samplesToCopy > 0 );

	//Right
	samplesToCopy = pDriver->m_BufferSize;

	do
	{
		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);
		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);
		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);
		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);

		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);
		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);
		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);
		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);

		//*(pRight-1);		//Do 1 read cycle so that Altera code will get the access to local bus.
		ReadWord( port );

	//Update.
		samplesToCopy -= 32;	//TRANSFER_SIZE
	}
	while ( samplesToCopy > 0 );
}

#pragma CODE_LOCKED
void channel_stereo_c::CopyBuffer_ASIO_Stereo_32LSB24_In( channel_stereo_c* pChannel )
{
	PULONG		pLeft = pChannel->m_pAddressOnCard_left;
	PULONG		pRight = pChannel->m_pAddressOnCard_right;

	PULONG		pSoftware_left;
	PULONG		pSoftware_right;

	if ( pChannel->m_BufferSelect == CHANNEL_STEREO_USE_BUFFER_A )
	{
		pSoftware_left = pChannel->m_pBuffer_Left_A;
		pSoftware_right = pChannel->m_pBuffer_Right_A;
	}
	else
	{
		pSoftware_left = pChannel->m_pBuffer_Left_B;
		pSoftware_right = pChannel->m_pBuffer_Right_B;
	}

//Transfer data...
	//Left...
	unsigned long	samplesToCopy = pDriver->m_BufferSize;

	do
	{
		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);
		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);
		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);
		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);

		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);
		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);
		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);
		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);

	//Update.
		samplesToCopy -= 32;	//TRANSFER_SIZE
	}
	while ( samplesToCopy > 0 );

	//Right...
	samplesToCopy = pDriver->m_BufferSize;

	do
	{
		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);
		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);
		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);
		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);

		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);
		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);
		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);
		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);

	//Update.
		samplesToCopy -= 32;	//TRANSFER_SIZE
	}
	while ( samplesToCopy > 0 );
}
#pragma CODE_LOCKED
void channel_stereo_c::CopyBuffer_ASIO_Stereo_32LSB24_Out( channel_stereo_c* pChannel )
{
	PULONG		pLeft = pChannel->m_pAddressOnCard_left;
	PULONG		pRight = pChannel->m_pAddressOnCard_right;

	PULONG		pSoftware_left;
	PULONG		pSoftware_right;

	PUSHORT		port = pChannel->m_pCard->Get_IoWave();

	if ( pChannel->m_BufferSelect == CHANNEL_STEREO_USE_BUFFER_A )
	{
		pSoftware_left = pChannel->m_pBuffer_Left_A;
		pSoftware_right = pChannel->m_pBuffer_Right_A;
	}
	else
	{
		pSoftware_left = pChannel->m_pBuffer_Left_B;
		pSoftware_right = pChannel->m_pBuffer_Right_B;
	}

//Transfer data...
	//Left...
	unsigned long	samplesToCopy = pDriver->m_BufferSize;

	do
	{
		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);
		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);
		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);
		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);

		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);
		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);
		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);
		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);

		//*(pLeft-1);			//Do 1 read cycle so that Altera code will get the access to local bus.
		ReadWord( port );

	//Update.
		samplesToCopy -= 32;	//TRANSFER_SIZE
	}
	while ( samplesToCopy > 0 );

	//Right...
	samplesToCopy = pDriver->m_BufferSize;

	do
	{
		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);
		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);
		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);
		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);


		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);
		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);
		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);
		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);

		//*(pRight-1);		//Do 1 read cycle so that Altera code will get the access to local bus.
		ReadWord( port );

	//Update.
		samplesToCopy -= 32;	//TRANSFER_SIZE
	}
	while ( samplesToCopy > 0 );
}

#pragma CODE_LOCKED
void channel_stereo_c::CopyBuffer_ASIO_Stereo_32MSB24_In( channel_stereo_c* pChannel )
{
	PULONG		pLeft = pChannel->m_pAddressOnCard_left;
	PULONG		pRight = pChannel->m_pAddressOnCard_right;

	PULONG		pSoftware_left;
	PULONG		pSoftware_right;

	if ( pChannel->m_BufferSelect == CHANNEL_STEREO_USE_BUFFER_A )
	{
		pSoftware_left = pChannel->m_pBuffer_Left_A;
		pSoftware_right = pChannel->m_pBuffer_Right_A;
	}
	else
	{
		pSoftware_left = pChannel->m_pBuffer_Left_B;
		pSoftware_right = pChannel->m_pBuffer_Right_B;
	}

//Transfer data...
	//Left
	unsigned long	samplesToCopy = pDriver->m_BufferSize;

	do
	{
		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);
		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);
		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);
		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);

		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);
		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);
		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);
		*(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++); *(pSoftware_left++) = *(pLeft++);

	//Update.
		samplesToCopy -= 32;	//TRANSFER_SIZE
	}
	while ( samplesToCopy > 0 );

	//Right
	samplesToCopy = pDriver->m_BufferSize;

	do
	{
		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);
		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);
		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);
		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);

		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);
		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);
		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);
		*(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++); *(pSoftware_right++) = *(pRight++);

	//Update.
		samplesToCopy -= 32;	//TRANSFER_SIZE
	}
	while ( samplesToCopy > 0 );

//Change sample type...
//32MSB24 - 0321 ->        -> 1230
	if ( pChannel->m_BufferSelect == CHANNEL_STEREO_USE_BUFFER_A )
	{
		pSoftware_left = pChannel->m_pBuffer_Left_A;
		pSoftware_right = pChannel->m_pBuffer_Right_A;
	}
	else
	{
		pSoftware_left = pChannel->m_pBuffer_Left_B;
		pSoftware_right = pChannel->m_pBuffer_Right_B;
	}

	//Left
	samplesToCopy = pDriver->m_BufferSize;

	do
	{
		unsigned long	sample = *pSoftware_left;
		sample = _byteswap_ulong( sample );
		*pSoftware_left = sample;

		++pSoftware_left;

	//Update.
		--samplesToCopy;
	}
	while ( samplesToCopy > 0 );

	//Right
	samplesToCopy = pDriver->m_BufferSize;

	do
	{
		unsigned long	sample = *pSoftware_right;
		sample = _byteswap_ulong( sample );
		*pSoftware_right = sample;

		++pSoftware_right;

	//Update.
		--samplesToCopy;
	}
	while ( samplesToCopy > 0 );
}
#pragma CODE_LOCKED
void channel_stereo_c::CopyBuffer_ASIO_Stereo_32MSB24_Out( channel_stereo_c* pChannel )
{
	PULONG		pLeft = pChannel->m_pAddressOnCard_left;
	PULONG		pRight = pChannel->m_pAddressOnCard_right;

	PULONG		pSoftware_left;
	PULONG		pSoftware_right;

	PUSHORT		port = pChannel->m_pCard->Get_IoWave();

	if ( pChannel->m_BufferSelect == CHANNEL_STEREO_USE_BUFFER_A )
	{
		pSoftware_left = pChannel->m_pBuffer_Left_A;
		pSoftware_right = pChannel->m_pBuffer_Right_A;
	}
	else
	{
		pSoftware_left = pChannel->m_pBuffer_Left_B;
		pSoftware_right = pChannel->m_pBuffer_Right_B;
	}

//Change sample type...
//32MSB24 - 1230 ->        -> 0321
	//Left
	unsigned long	samplesToCopy = pDriver->m_BufferSize;

	do
	{
		unsigned long	sample = *pSoftware_left;
		sample = _byteswap_ulong( sample );
		*pSoftware_left = sample;

		++pSoftware_left;

	//Update.
		--samplesToCopy;
	}
	while ( samplesToCopy > 0 );

	//Right
	samplesToCopy = pDriver->m_BufferSize;

	do
	{
		unsigned long	sample = *pSoftware_right;
		sample = _byteswap_ulong( sample );
		*pSoftware_right = sample;

		++pSoftware_right;

	//Update.
		--samplesToCopy;
	}
	while ( samplesToCopy > 0 );

//Transfer data...
	if ( pChannel->m_BufferSelect == CHANNEL_STEREO_USE_BUFFER_A )
	{
		pSoftware_left = pChannel->m_pBuffer_Left_A;
		pSoftware_right = pChannel->m_pBuffer_Right_A;
	}
	else
	{
		pSoftware_left = pChannel->m_pBuffer_Left_B;
		pSoftware_right = pChannel->m_pBuffer_Right_B;
	}

	//Left
	samplesToCopy = pDriver->m_BufferSize;

	do
	{
		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);
		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);
		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);
		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);


		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);
		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);
		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);
		*(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++); *(pLeft++) = *(pSoftware_left++);

		//*(pLeft-1);			//Do 1 read cycle so that Altera code will get the access to local bus.
		ReadWord( port );

	//Update.
		samplesToCopy -= 32;	//TRANSFER_SIZE
	}
	while ( samplesToCopy > 0 );

	//Right
	samplesToCopy = pDriver->m_BufferSize;

	do
	{
		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);
		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);
		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);
		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);


		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);
		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);
		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);
		*(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++); *(pRight++) = *(pSoftware_right++);

		//*(pRight-1);		//Do 1 read cycle so that Altera code will get the access to local bus.
		ReadWord( port );

	//Update.
		samplesToCopy -= 32;	//TRANSFER_SIZE
	}
	while ( samplesToCopy > 0 );
}
