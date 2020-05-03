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
// MODULE:
//		card.cpp
//
// PURPOSE:
//		File contains the card class functions.
//
// STATUS:
//		Unfinished/Untested.
//
//***************************************************************************
//***************************************************************************
#include "main.h"
#include "asio_handle.h"
#include "driver_ioctl_asio.h"


#pragma DATA_LOCKED


#pragma CODE_LOCKED
NTSTATUS asioHandle_c::Memory_ValidateSize
(
	unsigned long		size_memory
)
{
	NTSTATUS				Status = STATUS_SUCCESS;

//Get number of channels opened.
	unsigned long			nChannels_opened = 0;

	//INS...
	channel_stereo_c*		pChannel;
	channel_stereo_c**		ppChannels = &m_pChannels_Stereo_INs[0];

	while ( ( pChannel = *ppChannels ) != NULL )
	{
		++nChannels_opened;

	//Next...
		++ppChannels;
	}

	//OUTs...
	ppChannels = &m_pChannels_Stereo_OUTs[0];

	while ( ( pChannel = *ppChannels ) != NULL )
	{
		++nChannels_opened;

	//Next...
		++ppChannels;
	}

//Calculate required memory size.
//
//size buffers: (A1 + A2 + B1 +B2) * nChannels * bufferSize * sizeof( SAMPLE )
	unsigned long		size_required = sizeof( ioctl_asio_switch ) + ( 4*nChannels_opened * pDriver->m_BufferSize*sizeof( unsigned long ) );

//Validate.
	if ( size_memory != size_required )
	{
		Status = STATUS_UNSUCCESSFUL;
	}

	return Status;
}

