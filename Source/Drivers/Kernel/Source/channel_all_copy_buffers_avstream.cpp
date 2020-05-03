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
#include "channel_all.h"
#include "card_altera_firmware.h"

#pragma DATA_LOCKED
unsigned long	offsets[8] = { 0, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 };


#pragma CODE_LOCKED
void channel_all_c::Set_Buffer_AVStream( PULONG pBuffer )
{
	m_pBuffer_all = pBuffer;
}

#pragma CODE_LOCKED
void channel_all_c::Set_HardwareAddress( unsigned long offsetInSamples )
{
	if ( offsetInSamples < m_Channel_size )
	{
		m_pAddressOnCard[0] = m_pAddressOnCard_root[0] + offsetInSamples;
		m_pAddressOnCard[1] = m_pAddressOnCard_root[1] + offsetInSamples;
		m_pAddressOnCard[2] = m_pAddressOnCard_root[2] + offsetInSamples;
		m_pAddressOnCard[3] = m_pAddressOnCard_root[3] + offsetInSamples;
		m_pAddressOnCard[4] = m_pAddressOnCard_root[4] + offsetInSamples;
		m_pAddressOnCard[5] = m_pAddressOnCard_root[5] + offsetInSamples;
		m_pAddressOnCard[6] = m_pAddressOnCard_root[6] + offsetInSamples;
		m_pAddressOnCard[7] = m_pAddressOnCard_root[7] + offsetInSamples;
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
NTSTATUS channel_all_c::ZeroChannelMemory( void )
{
	NTSTATUS		Status = STATUS_SUCCESS;

//Validate.
	if ( m_Type != CHANNEL_TYPE_OUT )
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//Write zeros to all channels on card.
	if ( SUCCESS( Status ) )
	{
		PULONG				pDestination[8];
		pDestination[0] = m_pAddressOnCard[0];
		pDestination[1] = m_pAddressOnCard[1];
		pDestination[2] = m_pAddressOnCard[2];
		pDestination[3] = m_pAddressOnCard[3];
		pDestination[4] = m_pAddressOnCard[4];
		pDestination[5] = m_pAddressOnCard[5];
		pDestination[6] = m_pAddressOnCard[6];
		pDestination[7] = m_pAddressOnCard[7];

		unsigned long		samplesInOnePass = 32 * m_nChannels;
		unsigned long		samplesToCopy = m_BufferSize_copyInCurrentPass;

		PUSHORT				port = m_pCard->Get_IoWave();

		do
		{
		//How many samples do we have to copy?
			unsigned long	nSamples;

			if ( samplesToCopy >= samplesInOnePass )
			{
				nSamples = 32;					//per 1 mono channel
				samplesToCopy -= samplesInOnePass;
			}
			else
			{
			//Less than TRANSFER_SIZE samples left.
				nSamples = samplesToCopy/m_nChannels;
				samplesToCopy = 0;
			}

		//Copy buffers...
			for ( unsigned long i = 0; i < m_nChannels; i++ )
			{
			//Get destination channel address.
				PULONG		pDestination_temp = pDestination[i];
				if ( pDestination_temp == NULL )
				{
				//No more channels available.
					break;
				}

			//Transfer data to card.
				unsigned long		offset = 0;
				do
				{
					*(pDestination_temp++) = 0;

				//Update...
					++offset;

				} while ( offset < nSamples );

				//*(pLeft-1);			//Do 1 read cycle so that Altera code will get the access to local bus.
				ReadWord( port );

			//Save current destination pointer back to local variable.
				pDestination[i] = pDestination_temp;
			}
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
NTSTATUS channel_all_c::Set_CopyBuffersProcedure_AVStream
(
	unsigned long		nChannels,
	CHANNEL_BIT_SIZE	nBitsInSample
)
{
	NTSTATUS	Status = STATUS_SUCCESS;

//Set procedure.
	if ( m_Type == CHANNEL_TYPE_IN )
	{
	//Input...
		switch ( nBitsInSample )
		{
		case CHANNEL_BIT_SIZE_16: m_pCopyBuffersProcedure = (PCOPY_BUFFER_WAVE)&CopyBuffer_AVStream_All_16bit_In; break;
		case CHANNEL_BIT_SIZE_24: m_pCopyBuffersProcedure = (PCOPY_BUFFER_WAVE)&CopyBuffer_AVStream_All_24bit_In; break;
		case CHANNEL_BIT_SIZE_32: m_pCopyBuffersProcedure = (PCOPY_BUFFER_WAVE)&CopyBuffer_AVStream_All_32bit_In; break;
		default:
			Status = STATUS_UNSUCCESSFUL;
			break;
		}
	}
	else
	{
	//Output...
		switch ( nBitsInSample )
		{
		case CHANNEL_BIT_SIZE_16: m_pCopyBuffersProcedure = (PCOPY_BUFFER_WAVE)&CopyBuffer_AVStream_All_16bit_Out; break;
		case CHANNEL_BIT_SIZE_24: m_pCopyBuffersProcedure = (PCOPY_BUFFER_WAVE)&CopyBuffer_AVStream_All_24bit_Out; break;
		case CHANNEL_BIT_SIZE_32: m_pCopyBuffersProcedure = (PCOPY_BUFFER_WAVE)&CopyBuffer_AVStream_All_32bit_Out; break;
		default:
			Status = STATUS_UNSUCCESSFUL;
			break;
		}
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
void channel_all_c::CopyBuffer_AVStream_All_16bit_In( channel_all_c* pChannel )
{
//sample container: 16bit
//MSB16 - 0321  -> ..... -> 32
	PULONG				pSource[8];
	pSource[0] = pChannel->m_pAddressOnCard[0];
	pSource[1] = pChannel->m_pAddressOnCard[1];
	pSource[2] = pChannel->m_pAddressOnCard[2];
	pSource[3] = pChannel->m_pAddressOnCard[3];
	pSource[4] = pChannel->m_pAddressOnCard[4];
	pSource[5] = pChannel->m_pAddressOnCard[5];
	pSource[6] = pChannel->m_pAddressOnCard[6];
	pSource[7] = pChannel->m_pAddressOnCard[7];

	PUSHORT				pSoftware_muxed = (PUSHORT)pChannel->m_pBuffer_all;

	unsigned long		samplesInOnePass = 32 * pChannel->m_nChannels;
	unsigned long		samplesToCopy = pChannel->m_BufferSize_copyInCurrentPass;

	unsigned long		container[8*32];		//we'll first copy the samples and then pack them

	PUSHORT				port = pChannel->m_pCard->Get_IoWave();

	do
	{
	//How many samples do we have to copy?
		unsigned long	nSamples;

		if ( samplesToCopy >= samplesInOnePass )
		{
			nSamples = 32;					//per 1 mono channel
			samplesToCopy -= samplesInOnePass;
		}
		else
		{
		//Less than TRANSFER_SIZE samples left.
			nSamples = samplesToCopy/pChannel->m_nChannels;
			samplesToCopy = 0;
		}

	//Copy buffers...
		for ( unsigned long i = 0; i < pChannel->m_nChannels; i++ )
		{
		//Get source channel address.
			PULONG		pSource_temp = pSource[i];
			if ( pSource_temp == NULL )
			{
			//No more channels available.
				break;
			}

		//Transfer data to card.
			unsigned long*		pContainer = &container[ offsets[i] ];

			unsigned long		offset = 0;
			do
			{
				*(pContainer++) = *(pSource_temp++);

			//Update...
				++offset;

			} while ( offset < nSamples );

			//*(pLeft-1);			//Do 1 read cycle so that Altera code will get the access to local bus.
			ReadWord( port );

		//Save current destination pointer back to local variable.
			pSource[i] = pSource_temp;
		}

	//Pack samples.
	//
	//Why are we doing it like this?
	//We'll use the CPU and it's cache to do what's doing best and then we'll send the whole package as one to the application.
	//Hopefully this will speed up the transfer which is the slowest code in this procedure.
		unsigned long		offset = 0;
		do
		{
			for ( unsigned long i = 0; i < pChannel->m_nChannels; i++ )
			{
			//MSB16 - 0321  -> ..... -> 32
				*(pSoftware_muxed++) = (USHORT)( container[offsets[i] + offset] >> 8 );
			}

		//Update...
			++offset;
		} while ( offset < nSamples );
	}
	while ( samplesToCopy > 0 );
}
#pragma CODE_LOCKED
void channel_all_c::CopyBuffer_AVStream_All_16bit_Out( channel_all_c* pChannel )
{
//sample container: 16bit
//MSB16 - 21  -> ..... -> 0210
	PULONG				pDestination[8];
	pDestination[0] = pChannel->m_pAddressOnCard[0];
	pDestination[1] = pChannel->m_pAddressOnCard[1];
	pDestination[2] = pChannel->m_pAddressOnCard[2];
	pDestination[3] = pChannel->m_pAddressOnCard[3];
	pDestination[4] = pChannel->m_pAddressOnCard[4];
	pDestination[5] = pChannel->m_pAddressOnCard[5];
	pDestination[6] = pChannel->m_pAddressOnCard[6];
	pDestination[7] = pChannel->m_pAddressOnCard[7];

	PUSHORT				pSoftware_muxed = (PUSHORT)pChannel->m_pBuffer_all;

	unsigned long		samplesInOnePass = 32 * pChannel->m_nChannels;
	unsigned long		samplesToCopy = pChannel->m_BufferSize_copyInCurrentPass;

	unsigned long		container[8*32];		//we'll first unpack the samples and then send them to the card

	PUSHORT				port = pChannel->m_pCard->Get_IoWave();

	do
	{
	//How many samples do we have to copy?
		unsigned long	nSamples;

		if ( samplesToCopy >= samplesInOnePass )
		{
			nSamples = 32;					//per 1 mono channel
			samplesToCopy -= samplesInOnePass;
		}
		else
		{
		//Less than TRANSFER_SIZE samples left.
			nSamples = samplesToCopy/pChannel->m_nChannels;
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
			for ( unsigned long i = 0; i < pChannel->m_nChannels; i++ )
			{
			//MSB16 - 21  -> ..... -> 0210
				container[offsets[i] + offset] = ( (unsigned long)*(pSoftware_muxed++) ) << 8;
			}

		//Update...
			++offset;
		} while ( offset < nSamples );

	//Copy buffers...
		for ( unsigned long i = 0; i < pChannel->m_nChannels; i++ )
		{
		//Get destination channel address.
			PULONG		pDestination_temp = pDestination[i];
			if ( pDestination_temp == NULL )
			{
			//No more channels available.
				break;
			}

		//Transfer data to card.
			unsigned long*		pContainer = &container[ offsets[i] ];

			unsigned long		offset = 0;
			do
			{
				*(pDestination_temp++) = *(pContainer++);

			//Update...
				++offset;

			} while ( offset < nSamples );

			//*(pLeft-1);			//Do 1 read cycle so that Altera code will get the access to local bus.
			ReadWord( port );

		//Save current destination pointer back to local variable.
			pDestination[i] = pDestination_temp;
		}
	}
	while ( samplesToCopy > 0 );
}

#pragma CODE_LOCKED
void channel_all_c::CopyBuffer_AVStream_All_24bit_In( channel_all_c* pChannel )
{
//sample container: 24bit
//MSB24 - 0321  -> ..... -> 321
	PULONG					pSource[8];
	pSource[0] = pChannel->m_pAddressOnCard[0];
	pSource[1] = pChannel->m_pAddressOnCard[1];
	pSource[2] = pChannel->m_pAddressOnCard[2];
	pSource[3] = pChannel->m_pAddressOnCard[3];
	pSource[4] = pChannel->m_pAddressOnCard[4];
	pSource[5] = pChannel->m_pAddressOnCard[5];
	pSource[6] = pChannel->m_pAddressOnCard[6];
	pSource[7] = pChannel->m_pAddressOnCard[7];

	SAMPLES_STEREO_24BIT*	pSoftware_muxed = (SAMPLES_STEREO_24BIT*)pChannel->m_pBuffer_all;

	unsigned long			samplesInOnePass = 32 * pChannel->m_nChannels;
	unsigned long			samplesToCopy = pChannel->m_BufferSize_copyInCurrentPass;

	unsigned long			container[8*32];		//we'll first copy the samples and then pack them

	PUSHORT					port = pChannel->m_pCard->Get_IoWave();

	do
	{
	//How many samples do we have to copy?
		unsigned long	nSamples;

		if ( samplesToCopy >= samplesInOnePass )
		{
			nSamples = 32;					//per 1 mono channel
			samplesToCopy -= samplesInOnePass;
		}
		else
		{
		//Less than TRANSFER_SIZE samples left.
			nSamples = samplesToCopy/pChannel->m_nChannels;
			samplesToCopy = 0;
		}

	//Copy buffers...
		for ( unsigned long i = 0; i < pChannel->m_nChannels; i++ )
		{
		//Get source channel address.
			PULONG		pSource_temp = pSource[i];
			if ( pSource_temp == NULL )
			{
			//No more channels available.
				break;
			}

		//Transfer data to card.
			unsigned long*		pContainer = &container[ offsets[i] ];

			unsigned long		offset = 0;
			do
			{
				*(pContainer++) = *(pSource_temp++);

			//Update...
				++offset;

			} while ( offset < nSamples );

			//*(pLeft-1);			//Do 1 read cycle so that Altera code will get the access to local bus.
			ReadWord( port );

		//Save current destination pointer back to local variable.
			pSource[i] = pSource_temp;
		}

	//Pack samples.
	//
	//Why are we doing it like this?
	//We'll use the CPU and it's cache to do what's doing best and then we'll send the whole package as one to the application.
	//Hopefully this will speed up the transfer which is the slowest code in this procedure.
		unsigned long		offset = 0;
		do
		{
			for ( unsigned long i = 0; i < pChannel->m_nChannels; i++ )
			{
			//MSB24 - 0321  -> ..... -> 321
				SAMPLE		sample;
				
				sample.data.dwords[0] = container[offsets[i] + offset];

				pSoftware_muxed->sample[0] = sample.data.bytes[0];
				pSoftware_muxed->sample[1] = sample.data.bytes[1];
				pSoftware_muxed->sample[2] = sample.data.bytes[2];

				++pSoftware_muxed;
			}

		//Update...
			++offset;
		} while ( offset < nSamples );
	}
	while ( samplesToCopy > 0 );
}
#pragma CODE_LOCKED
void channel_all_c::CopyBuffer_AVStream_All_24bit_Out( channel_all_c* pChannel )
{
//sample container: 24bit
//MSB24 - 321  -> ..... -> 0321
	PULONG					pDestination[8];
	pDestination[0] = pChannel->m_pAddressOnCard[0];
	pDestination[1] = pChannel->m_pAddressOnCard[1];
	pDestination[2] = pChannel->m_pAddressOnCard[2];
	pDestination[3] = pChannel->m_pAddressOnCard[3];
	pDestination[4] = pChannel->m_pAddressOnCard[4];
	pDestination[5] = pChannel->m_pAddressOnCard[5];
	pDestination[6] = pChannel->m_pAddressOnCard[6];
	pDestination[7] = pChannel->m_pAddressOnCard[7];

	SAMPLES_STEREO_24BIT*	pSoftware_muxed = (SAMPLES_STEREO_24BIT*)pChannel->m_pBuffer_all;

	unsigned long			samplesInOnePass = 32 * pChannel->m_nChannels;
	unsigned long			samplesToCopy = pChannel->m_BufferSize_copyInCurrentPass;

	unsigned long			container[8*32];		//we'll first unpack the samples and then send them to the card

	PUSHORT					port = pChannel->m_pCard->Get_IoWave();

	do
	{
	//How many samples do we have to copy?
		unsigned long	nSamples;

		if ( samplesToCopy >= samplesInOnePass )
		{
			nSamples = 32;					//per 1 mono channel
			samplesToCopy -= samplesInOnePass;
		}
		else
		{
		//Less than TRANSFER_SIZE samples left.
			nSamples = samplesToCopy/pChannel->m_nChannels;
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
			for ( unsigned long i = 0; i < pChannel->m_nChannels; i++ )
			{
			//MSB24 - 321  -> ..... -> 0321
				SAMPLE		sample;

				sample.data.bytes[0] = pSoftware_muxed->sample[0];
				sample.data.bytes[1] = pSoftware_muxed->sample[1];
				sample.data.bytes[2] = pSoftware_muxed->sample[2];

				++pSoftware_muxed;

				container[offsets[i] + offset] = sample.data.dwords[0];		//upper 8 bits are useless... card doesn't see them
			}

		//Update...
			++offset;
		} while ( offset < nSamples );

	//Copy buffers...
		for ( unsigned long i = 0; i < pChannel->m_nChannels; i++ )
		{
		//Get destination channel address.
			PULONG		pDestination_temp = pDestination[i];
			if ( pDestination_temp == NULL )
			{
			//No more channels available.
				break;
			}

		//Transfer data to card.
			unsigned long*		pContainer = &container[ offsets[i] ];

			unsigned long		offset = 0;
			do
			{
				*(pDestination_temp++) = *(pContainer++);

			//Update...
				++offset;

			} while ( offset < nSamples );

			//*(pLeft-1);			//Do 1 read cycle so that Altera code will get the access to local bus.
			ReadWord( port );

		//Save current destination pointer back to local variable.
			pDestination[i] = pDestination_temp;
		}
	}
	while ( samplesToCopy > 0 );
}

#pragma CODE_LOCKED
void channel_all_c::CopyBuffer_AVStream_All_32bit_In( channel_all_c* pChannel )
{
//sample container: 32bit
//MSB24 - 0321 -> shl 8 -> 3210
	PULONG				pSource[8];
	pSource[0] = pChannel->m_pAddressOnCard[0];
	pSource[1] = pChannel->m_pAddressOnCard[1];
	pSource[2] = pChannel->m_pAddressOnCard[2];
	pSource[3] = pChannel->m_pAddressOnCard[3];
	pSource[4] = pChannel->m_pAddressOnCard[4];
	pSource[5] = pChannel->m_pAddressOnCard[5];
	pSource[6] = pChannel->m_pAddressOnCard[6];
	pSource[7] = pChannel->m_pAddressOnCard[7];

	PULONG				pSoftware_muxed = pChannel->m_pBuffer_all;

	unsigned long		samplesInOnePass = 32 * pChannel->m_nChannels;
	unsigned long		samplesToCopy = pChannel->m_BufferSize_copyInCurrentPass;

	unsigned long		container[8*32];		//we'll first copy the samples and then pack them

	PUSHORT				port = pChannel->m_pCard->Get_IoWave();

	do
	{
	//How many samples do we have to copy?
		unsigned long	nSamples;

		if ( samplesToCopy >= samplesInOnePass )
		{
			nSamples = 32;					//per 1 mono channel
			samplesToCopy -= samplesInOnePass;
		}
		else
		{
		//Less than TRANSFER_SIZE samples left.
			nSamples = samplesToCopy/pChannel->m_nChannels;
			samplesToCopy = 0;
		}

	//Copy buffers...
		for ( unsigned long i = 0; i < pChannel->m_nChannels; i++ )
		{
		//Get source channel address.
			PULONG		pSource_temp = pSource[i];
			if ( pSource_temp == NULL )
			{
			//No more channels available.
				break;
			}

		//Transfer data to card.
			unsigned long*		pContainer = &container[ offsets[i] ];

			unsigned long		offset = 0;
			do
			{
				*(pContainer++) = *(pSource_temp++);

			//Update...
				++offset;

			} while ( offset < nSamples );

			//*(pLeft-1);			//Do 1 read cycle so that Altera code will get the access to local bus.
			ReadWord( port );

		//Save current destination pointer back to local variable.
			pSource[i] = pSource_temp;
		}

	//Pack samples.
	//
	//Why are we doing it like this?
	//We'll use the CPU and it's cache to do what's doing best and then we'll send the whole package as one to the application.
	//Hopefully this will speed up the transfer which is the slowest code in this procedure.
		unsigned long		offset = 0;
		do
		{
			for ( unsigned long i = 0; i < pChannel->m_nChannels; i++ )
			{
			//MSB24 - 0321 -> shl 8 -> 3210
				*(pSoftware_muxed++) = container[offsets[i] + offset] << 8;
			}

		//Update...
			++offset;
		} while ( offset < nSamples );
	}
	while ( samplesToCopy > 0 );
}
#pragma CODE_LOCKED
void channel_all_c::CopyBuffer_AVStream_All_32bit_Out( channel_all_c* pChannel )
{
//sample container: 32bit
//MSB16 - 2100 -> shr 8 -> 0210
//MSB24 - 3210 -> shr 8 -> 0321
	PULONG				pDestination[8];
	pDestination[0] = pChannel->m_pAddressOnCard[0];
	pDestination[1] = pChannel->m_pAddressOnCard[1];
	pDestination[2] = pChannel->m_pAddressOnCard[2];
	pDestination[3] = pChannel->m_pAddressOnCard[3];
	pDestination[4] = pChannel->m_pAddressOnCard[4];
	pDestination[5] = pChannel->m_pAddressOnCard[5];
	pDestination[6] = pChannel->m_pAddressOnCard[6];
	pDestination[7] = pChannel->m_pAddressOnCard[7];

	PULONG				pSoftware_muxed = (PULONG)pChannel->m_pBuffer_all;

	unsigned long		samplesInOnePass = 32 * pChannel->m_nChannels;
	unsigned long		samplesToCopy = pChannel->m_BufferSize_copyInCurrentPass;

	unsigned long		container[8*32];		//we'll first unpack the samples and then send them to the card

	PUSHORT				port = pChannel->m_pCard->Get_IoWave();

	do
	{
	//How many samples do we have to copy?
		unsigned long	nSamples;

		if ( samplesToCopy >= samplesInOnePass )
		{
			nSamples = 32;					//per 1 mono channel
			samplesToCopy -= samplesInOnePass;
		}
		else
		{
		//Less than TRANSFER_SIZE samples left.
			nSamples = samplesToCopy/pChannel->m_nChannels;
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
			for ( unsigned long i = 0; i < pChannel->m_nChannels; i++ )
			{
				container[offsets[i] + offset] = ( (unsigned long)*(pSoftware_muxed++) ) >> 8;
			}

		//Update...
			++offset;
		} while ( offset < nSamples );

	//Copy buffers...
		for ( unsigned long i = 0; i < pChannel->m_nChannels; i++ )
		{
		//Get destination channel address.
			PULONG		pDestination_temp = pDestination[i];
			if ( pDestination_temp == NULL )
			{
			//No more channels available.
				break;
			}

		//Transfer data to card.
			unsigned long*		pContainer = &container[ offsets[i] ];

			unsigned long		offset = 0;
			do
			{
				*(pDestination_temp++) = *(pContainer++);

			//Update...
				++offset;

			} while ( offset < nSamples );

			//*(pLeft-1);			//Do 1 read cycle so that Altera code will get the access to local bus.
			ReadWord( port );

		//Save current destination pointer back to local variable.
			pDestination[i] = pDestination_temp;
		}
	}
	while ( samplesToCopy > 0 );
}
