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
#include "card.h"


#pragma DATA_LOCKED


#pragma CODE_LOCKED
void channel_stereo_c::Set_Buffer_AVStream( PULONG pBuffer )
{
	m_pBuffer_Left_A = pBuffer;
}

#pragma CODE_LOCKED
void channel_stereo_c::Set_HardwareAddress( unsigned long offsetInSamples )
{
	if ( offsetInSamples < m_Channel_size )
	{
		m_pAddressOnCard_left = m_pAddressOnCard_root_left + offsetInSamples;
		m_pAddressOnCard_right = m_pAddressOnCard_root_right + offsetInSamples;
	}
}

//***************************************************************************
// ProcedureName:
//		ZeroChannelMemory
//
// Explanation:
//		Procedure will zero out the output channels data.
//		And with that make the channel silent.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// Synhronization:
//		None
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return Codes:
//		Non.
//
#pragma CODE_LOCKED
NTSTATUS channel_stereo_c::ZeroChannelMemory( void )
{
	NTSTATUS		Status = STATUS_SUCCESS;

//Validate.
	if ( m_Type != CHANNEL_TYPE_OUT )
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//Write zeros to left and right channel on card.
	if ( SUCCESS( Status ) )
	{
		PULONG				pLeft = m_pAddressOnCard_left;
		PULONG				pRight = m_pAddressOnCard_right;

		unsigned long		samplesToCopy = m_BufferSize_copyInCurrentPass;

		PUSHORT				port = m_pCard->Get_IoWave();

		do
		{
		//How many samples do we have to copy?
			unsigned long	nSamples;

			if ( samplesToCopy >= 64 )			//64 = 32 (TRANSFER_SIZE) * 2 (stereo)
			{
				nSamples = 32;					//per 1 mono channel
				samplesToCopy -= 64;
			}
			else
			{
			//Less or equal to TRANSFER_SIZE samples left.
				nSamples = samplesToCopy/2;		//we will always get even numbers of samples to process
				samplesToCopy = 0;
			}

		//Copy samples.
			unsigned long		offset;

			//Left...
			offset = 0;
			do
			{
				*(pLeft++) = 0;

			//Update...
				++offset;

			} while ( offset < nSamples );

			//*(pLeft-1);			//Do 1 read cycle so that Altera code will get the access to local bus.
			ReadWord( port );

			//Right
			offset = 0;
			do
			{
				*(pRight++) = 0;

			//Update...
				++offset;

			} while ( offset < nSamples );

			//*(pRight-1);			//Do 1 read cycle so that Altera code will get the access to local bus.
			ReadWord( port );
		}
		while ( samplesToCopy > 0 );
	}

	return Status;
}

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
NTSTATUS channel_stereo_c::Set_CopyBuffersProcedure_AVStream
(
	unsigned long		nChannels,
	CHANNEL_BIT_SIZE	nBitsInSample
)
{
	NTSTATUS	Status = STATUS_SUCCESS;

//Set procedure.
	if ( nChannels == 2 )
	{
		if ( m_Type == CHANNEL_TYPE_IN )
		{
		//Input...
			switch ( nBitsInSample )
			{
			case CHANNEL_BIT_SIZE_16: m_pCopyBuffersProcedure = (PCOPY_BUFFER_WAVE)&CopyBuffer_AVStream_Stereo_16bit_In; break;
			case CHANNEL_BIT_SIZE_24: m_pCopyBuffersProcedure = (PCOPY_BUFFER_WAVE)&CopyBuffer_AVStream_Stereo_24bit_In; break;
			case CHANNEL_BIT_SIZE_32: m_pCopyBuffersProcedure = (PCOPY_BUFFER_WAVE)&CopyBuffer_AVStream_Stereo_32bit_In; break;
			default:
				Status = STATUS_UNSUCCESSFUL;
			}
		}
		else
		{
		//Output...
			switch ( nBitsInSample )
			{
			case CHANNEL_BIT_SIZE_16: m_pCopyBuffersProcedure = (PCOPY_BUFFER_WAVE)&CopyBuffer_AVStream_Stereo_16bit_Out; break;
			case CHANNEL_BIT_SIZE_24: m_pCopyBuffersProcedure = (PCOPY_BUFFER_WAVE)&CopyBuffer_AVStream_Stereo_24bit_Out; break;
			case CHANNEL_BIT_SIZE_32: m_pCopyBuffersProcedure = (PCOPY_BUFFER_WAVE)&CopyBuffer_AVStream_Stereo_32bit_Out; break;
			default:
				Status = STATUS_UNSUCCESSFUL;
			}
		}
	}
	else
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//Set other stuff...
	if ( SUCCESS( Status ) )
	{
		m_BitSize = nBitsInSample;
		m_BufferSize_nChannels_inSamples = nChannels * pDriver->m_BufferSize;
		m_nChannels = nChannels;
	}

	return Status;
}

#pragma CODE_LOCKED
void channel_stereo_c::CopyBuffer_AVStream_Stereo_16bit_In( channel_stereo_c* pChannel )
{
//sample container: 16bit
//MSB16 - 0321 -> ..... -> 32
	PULONG				pLeft = pChannel->m_pAddressOnCard_left;
	PULONG				pRight = pChannel->m_pAddressOnCard_right;

	PUSHORT				pSoftware_muxed = (PUSHORT)pChannel->m_pBuffer_Left_A;

	unsigned long		samplesToCopy = pChannel->m_BufferSize_copyInCurrentPass;

	unsigned long		container_left[32];		//we'll first copy the samples and then pack them
	unsigned long		container_right[32];	//we'll first copy the samples and then pack them

	do
	{
	//How many samples do we have to copy?
		unsigned long	nSamples;

		if ( samplesToCopy >= 64 )			//64 = 32 (TRANSFER_SIZE) * 2 (stereo)
		{
			nSamples = 32;					//per 1 mono channel
			samplesToCopy -= 64;
		}
		else
		{
		//Less or equal to TRANSFER_SIZE samples left.
			nSamples = samplesToCopy/2;		//we will always get even numbers of samples to process
			samplesToCopy = 0;
		}

	//Copy buffers...
	//
	//Why are we doing it like this?
	//As copy from card to cache is the slowest operation we'll let the CPU do it's job as good as it can and then we'll do ours.
		unsigned long		offset;
		unsigned long*		pContainer;

		//Left...
		pContainer = container_left;

		offset = 0;
		do
		{
			*(pContainer++) = *(pLeft++);

		//Update...
			++offset;

		} while ( offset < nSamples );

		//Right...
		pContainer = container_right;

		offset = 0;
		do
		{
			*(pContainer++) = *(pRight++);

		//Update...
			++offset;

		} while ( offset < nSamples );

	//Pack samples.
		offset = 0;

		do
		{
			//Left...
			*(pSoftware_muxed++) = (USHORT)( container_left[offset] >> 8 );

			//Right...
			*(pSoftware_muxed++) = (USHORT)( container_right[offset] >> 8 );

		//Update...
			++offset;
		} while ( offset < nSamples );
	}
	while ( samplesToCopy > 0 );
}
#pragma CODE_LOCKED
void channel_stereo_c::CopyBuffer_AVStream_Stereo_16bit_Out( channel_stereo_c* pChannel )
{
//sample container: 16bit
//MSB16 - 21  -> ..... -> 0210
	PULONG				pLeft = pChannel->m_pAddressOnCard_left;
	PULONG				pRight = pChannel->m_pAddressOnCard_right;

	PUSHORT				pSoftware_muxed = (PUSHORT)pChannel->m_pBuffer_Left_A;

	unsigned long		samplesToCopy = pChannel->m_BufferSize_copyInCurrentPass;

	unsigned long		container_left[32];		//we'll first unpack the samples and then send them to the card
	unsigned long		container_right[32];	//we'll first unpack the samples and then send them to the card

	PUSHORT				port = pChannel->m_pCard->Get_IoWave();

	do
	{
	//How many samples do we have to copy?
		unsigned long	nSamples;

		if ( samplesToCopy >= 64 )			//64 = 32 (TRANSFER_SIZE) * 2 (stereo)
		{
			nSamples = 32;					//per 1 mono channel
			samplesToCopy -= 64;
		}
		else
		{
		//Less than TRANSFER_SIZE samples left.
			nSamples = samplesToCopy/2;		//we will always get even numbers of samples to process
			samplesToCopy = 0;
		}

	//Unpack samples.
	//
	//Why are we doing it like this?
	//We'll first use the CPU and it's cache to do what's doing best and then we'll send the whole package as one to the card.
	//Hopefully this will speed up the transfer which is the slowest code in this procedure.
		unsigned long		offset = 0;

		do
		{
			//Left...
			container_left[offset] = ( (unsigned long)*(pSoftware_muxed++) ) << 8;

			//Right...
			container_right[offset] = ( (unsigned long)*(pSoftware_muxed++) ) << 8;

		//Update...
			++offset;
		} while ( offset < nSamples );

	//Copy buffers...
		unsigned long*		pContainer;

		//Left...
		pContainer = container_left;

		offset = 0;
		do
		{
			*(pLeft++) = *(pContainer++);

		//Update...
			++offset;

		} while ( offset < nSamples );

		//*(pLeft-1);			//Do 1 read cycle so that Altera code will get the access to local bus.
		ReadWord( port );

		//Right...
		pContainer = container_right;

		offset = 0;
		do
		{
			*(pRight++) = *(pContainer++);

		//Update...
			++offset;

		} while ( offset < nSamples );

		//*(pRight-1);			//Do 1 read cycle so that Altera code will get the access to local bus.
		ReadWord( port );
	}
	while ( samplesToCopy > 0 );
}

#pragma CODE_LOCKED
void channel_stereo_c::CopyBuffer_AVStream_Stereo_24bit_In( channel_stereo_c* pChannel )
{
//sample container: 24bit
//MSB24 - 321  -> ..... -> 0321
	PULONG					pLeft = pChannel->m_pAddressOnCard_left;
	PULONG					pRight = pChannel->m_pAddressOnCard_right;

	SAMPLES_STEREO_24BIT*	pSoftware_muxed = (SAMPLES_STEREO_24BIT*)pChannel->m_pBuffer_Left_A;

	unsigned long			samplesToCopy = pChannel->m_BufferSize_copyInCurrentPass;

	unsigned long			container_left[32];		//we'll first copy the samples and then pack them
	unsigned long			container_right[32];	//we'll first copy the samples and then pack them

	do
	{
	//How many samples do we have to copy?
		unsigned long	nSamples;

		if ( samplesToCopy >= 64 )			//64 = 32 (TRANSFER_SIZE) * 2 (stereo)
		{
			nSamples = 32;					//per 1 mono channel
			samplesToCopy -= 64;
		}
		else
		{
		//Less then TRANSFER_SIZE samples left.
			nSamples = samplesToCopy/2;		//we will always get even numbers of samples to process
			samplesToCopy = 0;
		}

	//Copy buffers...
	//
	//Why are we doing it like this?
	//As copy from card to cache is the slowest operation we'll let the CPU do it's job as good as it can and then we'll do ours.
		unsigned long		offset;
		unsigned long*		pContainer;

		//Left...
		pContainer = container_left;

		offset = 0;
		do
		{
			*(pContainer++) = *(pLeft++);

		//Update...
			++offset;

		} while ( offset < nSamples );

		//Right...
		pContainer = container_right;

		offset = 0;
		do
		{
			*(pContainer++) = *(pRight++);

		//Update...
			++offset;

		} while ( offset < nSamples );

	//Pack samples.
		offset = 0;

		do
		{
			SAMPLE		sample;

			//Left...
			sample.data.dwords[0] = container_left[offset];

			pSoftware_muxed->sample[0] = sample.data.bytes[0];
			pSoftware_muxed->sample[1] = sample.data.bytes[1];
			pSoftware_muxed->sample[2] = sample.data.bytes[2];

			++pSoftware_muxed;

			//Right...
			sample.data.dwords[0] = container_right[offset];

			pSoftware_muxed->sample[0] = sample.data.bytes[0];
			pSoftware_muxed->sample[1] = sample.data.bytes[1];
			pSoftware_muxed->sample[2] = sample.data.bytes[2];

			++pSoftware_muxed;

		//Update...
			++offset;
		} while ( offset < nSamples );
	}
	while ( samplesToCopy > 0 );
}
#pragma CODE_LOCKED
void channel_stereo_c::CopyBuffer_AVStream_Stereo_24bit_Out( channel_stereo_c* pChannel )
{
//sample container: 24bit
//MSB24 - 321  -> ..... -> 0321
	PULONG					pLeft = pChannel->m_pAddressOnCard_left;
	PULONG					pRight = pChannel->m_pAddressOnCard_right;

	SAMPLES_STEREO_24BIT*	pSoftware_muxed = (SAMPLES_STEREO_24BIT*)pChannel->m_pBuffer_Left_A;

	unsigned long			samplesToCopy = pChannel->m_BufferSize_copyInCurrentPass;

	unsigned long			container_left[32];		//we'll first unpack the samples and then send them to the card
	unsigned long			container_right[32];	//we'll first unpack the samples and then send them to the card

	PUSHORT					port = pChannel->m_pCard->Get_IoWave();

	do
	{
	//How many samples do we have to copy?
		unsigned long	nSamples;

		if ( samplesToCopy >= 64 )			//64 = 32 (TRANSFER_SIZE) * 2 (stereo)
		{
			nSamples = 32;					//per 1 mono channel
			samplesToCopy -= 64;
		}
		else
		{
		//Less or equal to TRANSFER_SIZE samples left.
			nSamples = samplesToCopy/2;		//we will always get even numbers of samples to process
			samplesToCopy = 0;
		}

	//Unpack samples.
	//
	//Why are we doing it like this?
	//We'll first use the CPU and it's cache to do what's doing best and then we'll send the whole package as one to the card.
	//Hopefully this will speed up the transfer which is the slowest code in this procedure.
		unsigned long		offset = 0;

		do
		{
			SAMPLE		sample;

			//Left...
			sample.data.bytes[0] = pSoftware_muxed->sample[0];
			sample.data.bytes[1] = pSoftware_muxed->sample[1];
			sample.data.bytes[2] = pSoftware_muxed->sample[2];

			container_left[offset] = sample.data.dwords[0];		//upper 8 bits are useless... card doesn't see them

			++pSoftware_muxed;

			//Right...
			sample.data.bytes[0] = pSoftware_muxed->sample[0];
			sample.data.bytes[1] = pSoftware_muxed->sample[1];
			sample.data.bytes[2] = pSoftware_muxed->sample[2];

			container_right[offset] = sample.data.dwords[0];	//upper 8 bits are useless... card doesn't see them

			++pSoftware_muxed;

		//Update...
			++offset;
		} while ( offset < nSamples );

	//Copy buffers...
		unsigned long*		pContainer;

		//Left...
		pContainer = container_left;

		offset = 0;
		do
		{
			*(pLeft++) = *(pContainer++);

		//Update...
			++offset;

		} while ( offset < nSamples );

		//*(pLeft-1);			//Do 1 read cycle so that Altera code will get the access to local bus.
		ReadWord( port );

		//Right...
		pContainer = container_right;

		offset = 0;
		do
		{
			*(pRight++) = *(pContainer++);

		//Update...
			++offset;

		} while ( offset < nSamples );

		//*(pRight-1);			//Do 1 read cycle so that Altera code will get the access to local bus.
		ReadWord( port );
	}
	while ( samplesToCopy > 0 );
}

#pragma CODE_LOCKED
void channel_stereo_c::CopyBuffer_AVStream_Stereo_32bit_In( channel_stereo_c* pChannel )
{
//sample container: 32bit
//MSB16 - 2100 -> shr 8 -> 0210
//MSB24 - 3210 -> shr 8 -> 0321
	PULONG				pLeft = pChannel->m_pAddressOnCard_left;
	PULONG				pRight = pChannel->m_pAddressOnCard_right;

	PULONG				pSoftware_muxed = pChannel->m_pBuffer_Left_A;

	unsigned long		samplesToCopy = pChannel->m_BufferSize_copyInCurrentPass;

	unsigned long		container_left[32];		//we'll first copy the samples and then pack them
	unsigned long		container_right[32];	//we'll first copy the samples and then pack them

	do
	{
	//How many samples do we have to copy?
		unsigned long	nSamples;

		if ( samplesToCopy >= 64 )			//64 = 32 (TRANSFER_SIZE) * 2 (stereo)
		{
			nSamples = 32;					//per 1 mono channel
			samplesToCopy -= 64;
		}
		else
		{
		//Less or equal to TRANSFER_SIZE samples left.
			nSamples = samplesToCopy/2;		//we will always get even numbers of samples to process
			samplesToCopy = 0;
		}

	//Copy buffers...
	//
	//Why are we doing it like this?
	//As copy from card to cache is the slowest operation we'll let the CPU do it's job as good as it can and then we'll do ours.
		unsigned long		offset;
		unsigned long*		pContainer;

		//Left...
		pContainer = container_left;

		offset = 0;
		do
		{
			*(pContainer++) = *(pLeft++);

		//Update...
			++offset;

		} while ( offset < nSamples );

		//Right...
		pContainer = container_right;

		offset = 0;
		do
		{
			*(pContainer++) = *(pRight++);

		//Update...
			++offset;

		} while ( offset < nSamples );

	//Pack samples.
		offset = 0;

		do
		{
			//Left...
			*(pSoftware_muxed++) = container_left[offset] << 8;

			//Right...
			*(pSoftware_muxed++) = container_right[offset] << 8;

		//Update...
			++offset;
		} while ( offset < nSamples );
	}
	while ( samplesToCopy > 0 );
}
#pragma CODE_LOCKED
void channel_stereo_c::CopyBuffer_AVStream_Stereo_32bit_Out( channel_stereo_c* pChannel )
{
//sample container: 32bit
//MSB16 - 2100 -> shr 8 -> 0210
//MSB24 - 3210 -> shr 8 -> 0321
	PULONG				pLeft = pChannel->m_pAddressOnCard_left;
	PULONG				pRight = pChannel->m_pAddressOnCard_right;

	PULONG				pSoftware_muxed = pChannel->m_pBuffer_Left_A;

	unsigned long		samplesToCopy = pChannel->m_BufferSize_copyInCurrentPass;

	unsigned long		container_left[32];		//we'll first unpack the samples and then send them to the card
	unsigned long		container_right[32];	//we'll first unpack the samples and then send them to the card

	PUSHORT				port = pChannel->m_pCard->Get_IoWave();

	do
	{
	//How many samples do we have to copy?
		unsigned long	nSamples;

		if ( samplesToCopy >= 64 )			//64 = 32 (TRANSFER_SIZE) * 2 (stereo)
		{
			nSamples = 32;					//per 1 mono channel
			samplesToCopy -= 64;
		}
		else
		{
		//Less or equal to TRANSFER_SIZE samples left.
			nSamples = samplesToCopy/2;		//we will always get even numbers of samples to process
			samplesToCopy = 0;
		}

	//Unpack samples.
	//
	//Why are we doing it like this?
	//We'll first use the CPU and it's cache to do what's doing best and then we'll send the whole package as one to the card.
	//Hopefully this will speed up the transfer which is the slowest code in this procedure.
		unsigned long		offset = 0;

		do
		{
			//Left...
			container_left[offset] = ( (unsigned long)*(pSoftware_muxed++) ) >> 8;

			//Right...
			container_right[offset] = ( (unsigned long)*(pSoftware_muxed++) ) >> 8;

		//Update...
			++offset;
		} while ( offset < nSamples );

	//Copy buffers...
		unsigned long*		pContainer;

		//Left...
		pContainer = container_left;

		offset = 0;
		do
		{
			*(pLeft++) = *(pContainer++);

		//Update...
			++offset;

		} while ( offset < nSamples );

		//*(pLeft-1);			//Do 1 read cycle so that Altera code will get the access to local bus.
		ReadWord( port );

		//Right...
		pContainer = container_right;

		offset = 0;
		do
		{
			*(pRight++) = *(pContainer++);

		//Update...
			++offset;

		} while ( offset < nSamples );

		//*(pRight-1);			//Do 1 read cycle so that Altera code will get the access to local bus.
		ReadWord( port );
	}
	while ( samplesToCopy > 0 );
}
