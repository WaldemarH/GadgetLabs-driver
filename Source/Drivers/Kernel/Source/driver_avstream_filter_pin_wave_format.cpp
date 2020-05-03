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
//		avstream.cpp
//
// PURPOSE:
//		File contains the avstream subdriver class.
//
// STATUS:
//		Unfinished/Untested.
//
//***************************************************************************
//***************************************************************************
#include "main.h"
#include "notify_event.h"


#pragma DATA_LOCKED

//***************************************************************************
// ProcedureName:
//		Pin_AudioIntersectHandler
//
// Explanation:
//		AVStream will call this procedure before opening the pin.
//		It will propose a DATARANGE that it wishes to use.
//
// NOTICE:
//		Filter mutex is held.
//
//	NOTICE!
//		We will only validate the datarange and set apropriate buffer size.
//		SampleRate, sampleBitSize and number of channels opened will be
//		verified in Pin_SetDataFormat( card dependant ).
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// filter: IN
//		Void pointer to the filter structure.
//
// IRP: IN
//		Contains a pointer to the data intersection property request.
//
// pin: IN
//		Contains a pointer to a structure indicating the pin in question.
//
// callerDataRange: IN
//		Contains a pointer to one of the data ranges supplied by the client
//		in the data intersection request. The format type, subtype and
//		specifier are compatible with the DescriptorDataRange.
//
// descriptorDataRange: IN
//		Contains a pointer to one of the data ranges from the pin descriptor
//		for the pin in question. The format type, subtype and specifier
//		are compatible with the CallerDataRange.
//
// dataBufferSize: IN
//		Contains the size in bytes of the buffer pointed to by the
//		Data argument. For size queries, this value will be zero.
//
// data: IN OPTIONAL
//		Optionally contains a pointer to the buffer to contain the data
//		format structure representing the best format in the intersection
//		of the two data ranges. For size queries, this pointer will be NULL.
//
// dataSize: OUT
//		Contains a pointer to the location at which to deposit the size of the
//		data format. This information is supplied by the function when the
//		format is actually delivered and in response to size queries.
//
// --------------------------------------------------------------------------
// Return:
//		STATUS_SUCCESS if there is an intersection and it fits in the supplied buffer
//		STATUS_BUFFER_OVERFLOW for successful size queries
//		STATUS_NO_MATCH if the intersection is empty
//		STATUS_BUFFER_TOO_SMALL if the supplied buffer is too small
//
#pragma CODE_LOCKED
NTSTATUS driver_c::Pin_AudioIntersectHandler
(
	IN PVOID					pContext,
	IN PIRP						pIrp,
	IN PKSP_PIN					pPin,
	IN PKSDATARANGE_AUDIO		pCallerDataRange,
	IN PKSDATARANGE_AUDIO		pDescriptorDataRange,
	IN ULONG					dataBufferSize,
	OUT PVOID					pData OPTIONAL,
	OUT PULONG					pDataSize
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
#endif

	NTSTATUS					Status = STATUS_NO_MATCH;

//Initialize variables.
	card_c*						pCard = NULL;
	card_c*						pCard_master = NULL;

	PKSFILTER		pFilter = KsGetFilterFromIrp( pIrp );
	if ( pFilter != NULL )
	{
		Status = pDriver->GetCard_FromFilter( pFilter, &pCard );
	}
	if ( SUCCESS( Status ) )
	{
		pCard_master = pCard->GetMasterCard();
	}

//Test descriptorDataRange.
//
//DX is using the WAVEFORMATEX structures too, so just to be 100% sure.
	if ( SUCCESS( Status ) )
	{
		if ( (BOOL)TRUE == IsEqualGUID( pDescriptorDataRange->DataRange.Specifier, KSDATAFORMAT_SPECIFIER_DSOUND ) )
		{
			Status = STATUS_NO_MATCH;
		}
	}

//Check data buffer size.
	if ( SUCCESS( Status ) )
	{
		if ( dataBufferSize == 0 )
		{
		//Caller is asking for data buffer size?
			*pDataSize = sizeof( KSDATAFORMAT_WAVEFORMATEXTENSIBLE );

		//Signal the client to change the buffer size.
			Status = STATUS_BUFFER_OVERFLOW;
		}
		else if ( dataBufferSize < sizeof( KSDATAFORMAT_WAVEFORMATEXTENSIBLE ) )
		{
		//Received buffer is not big enough.
			Status = STATUS_BUFFER_TOO_SMALL;
		}
	}

//Validate CallerDataRange.
	BOOL	wildcard;

	if ( SUCCESS( Status ) )
	{
		if ( (BOOL)TRUE == IsEqualGUID( pCallerDataRange->DataRange.Specifier, KSDATAFORMAT_SPECIFIER_WILDCARD ) )
		{
		//Wildcard -> return preferred dataRange.
			wildcard = TRUE;
		}
		else
		{
		//Not a wildcard -> check the dataRange.
			wildcard = FALSE;

		//Validate
			if (
			  ( pCallerDataRange->DataRange.FormatSize != sizeof( KSDATARANGE_AUDIO ) )

			  ||
			  ( pCallerDataRange->MaximumSampleFrequency < pDescriptorDataRange->MinimumSampleFrequency )
			  ||
			  ( pCallerDataRange->MinimumSampleFrequency > pDescriptorDataRange->MaximumSampleFrequency )

			  ||
			  ( pCallerDataRange->MaximumBitsPerSample < pDescriptorDataRange->MinimumBitsPerSample )
			  ||
			  ( pCallerDataRange->MinimumBitsPerSample > pDescriptorDataRange->MaximumBitsPerSample )

			  ||
			  ( pCallerDataRange->MaximumChannels < 2 )
			)
			{
				Status = STATUS_NO_MATCH;
			}
		}
	}

//Get sampleRate.
	unsigned long		nSamplesPerSec;

	if ( SUCCESS( Status ) )
	{
	//As procedure is going to access sampleRate variables we need to get a driver lock.
		Status = pDriver->LockDriver();

	//Check if sampleRate is already locked.
		if ( pCard_master->m_SampleRate_owned > 0 )
		{
		//It's locked -> use the locked sampleRate.
			nSamplesPerSec = (int)( pCard_master->m_SampleRate );
		}
		else
		{
			if ( wildcard == TRUE )
			{
			//Wildcard -> return preferred dataRange.
			//
			//The new wins standards request this to be 48000Hz, but as the cards defaults
			//is 44100Hz I choose this one.
				nSamplesPerSec = 44100;
			}
			else
			{
			//Not a wildcard -> check the dataRange.
				nSamplesPerSec = pCallerDataRange->MaximumSampleFrequency;
				switch ( nSamplesPerSec )
				{
				case 44100 :
				case 48000 :
				case 88200 :
				case 96000 :
					break;

				default:
					nSamplesPerSec = 44100;
					break;
				}
			}
		}

	//Release driver lock.
		pDriver->UnlockDriver();
	}

//Create the right data format.
//
//acceptedDataRange = callerDataRange "&" descriptorDataRange
	if ( SUCCESS( Status ) )
	{
		PKSDATAFORMAT_WAVEFORMATEXTENSIBLE		pDataFormat = (PKSDATAFORMAT_WAVEFORMATEXTENSIBLE)pData;
		*pDataSize = sizeof( KSDATAFORMAT_WAVEFORMATEXTENSIBLE );

	//All the guids are already in the descriptor's data format(range).
		RtlCopyMemory( &( pDataFormat->DataFormat ), pDescriptorDataRange, sizeof( pDataFormat->DataFormat ) );
		pDataFormat->DataFormat.FormatSize = sizeof( KSDATAFORMAT_WAVEFORMATEXTENSIBLE );

	//wFormatTag
		pDataFormat->WaveFormatEx.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;

	//nChannels
	//
	//Only stereo or all channels will be supported.
		pDataFormat->WaveFormatEx.Format.nChannels = (WORD)min( pCallerDataRange->MaximumChannels, pDescriptorDataRange->MaximumChannels );

	//nSamplesPerSec
		pDataFormat->WaveFormatEx.Format.nSamplesPerSec = nSamplesPerSec;

	//nBlockAlign
	//n bytes = n channels * (32bits/8)
	//i.e. 8 for 2 channel
		pDataFormat->WaveFormatEx.Format.nBlockAlign = pDataFormat->WaveFormatEx.Format.nChannels * 4;

	//nAvgBytesPerSec
		pDataFormat->WaveFormatEx.Format.nAvgBytesPerSec = pDataFormat->WaveFormatEx.Format.nSamplesPerSec * pDataFormat->WaveFormatEx.Format.nBlockAlign;

	//wBitsPerSample
		pDataFormat->WaveFormatEx.Format.wBitsPerSample = 32;

	//cbSize
		pDataFormat->WaveFormatEx.Format.cbSize = sizeof( WAVEFORMATEXTENSIBLE ) - sizeof( WAVEFORMATEX );

	//wValidBitesPerSample
		if ( wildcard == TRUE )
		{
		//Wildcard -> return preferred dataRange.
			pDataFormat->WaveFormatEx.Samples.wValidBitsPerSample = 24;
		}
		else
		{
		//Not a wildcard -> check the dataRange.
			pDataFormat->WaveFormatEx.Samples.wValidBitsPerSample = (WORD)min( pCallerDataRange->MaximumBitsPerSample, 24 );
		}

	//dwChannelMask
		if ( pDataFormat->WaveFormatEx.Format.nChannels == 2 )
		{
		//Stereo channels format...
			pDataFormat->WaveFormatEx.dwChannelMask = KSAUDIO_SPEAKER_STEREO;
		}
		else
		{
		//Surround channels format...
		//
		//WALDEMAR
		//Here we can have problems id nChannels is 3, 5, 7... probably we'll have to make the code that process
		//every speaker on it's own.
			if ( pCard->m_Type != card_c::CARD_TYPE_824 )
			{
			//4xx
				pDataFormat->WaveFormatEx.dwChannelMask = KSAUDIO_SPEAKER_QUAD;
			}
			else
			{
			//824
				pDataFormat->WaveFormatEx.dwChannelMask = KSAUDIO_SPEAKER_7POINT1_SURROUND;
			}
		}

	//SubFormat
		pDataFormat->WaveFormatEx.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
	}

#ifdef DBG_AND_LOGGER
	pLogger->Log(
	  logger_c::LOGGER_SEND_TO_LOG_FILE,
	  logger_c::LS_Driver_AVStream_Pin_Format_AudioIntersectHandler,
	  pCallerDataRange,
	  pDescriptorDataRange,
	  &dataBufferSize,
	  pData,
	  pDataSize
	);
	Procedure_Exit( Status );
#endif
	return Status;
}

//****************************************************************************
// ProcedureName:
//		Pin_SetDataFormat_Wave
//
// Explanation:
//		An AVStream minidriver's AVStrMiniPinSetDataFormat routine is called
//		at pin creation time to verify that the previously agreed upon data
//		format is acceptable for this KSPIN structure and a match for this
//		KSDATARANGE structure.
//		This routine is also called due to certain types of dynamic format
//		changes, for example the acceptance of a
//		KSPROPERTY_CONNECTION_PROPOSEDATAFORMAT property request.
//
//
//		It is called in two circumstances:
//		1: before pPin's creation dispatch has been made to verify that
//		pPin -> ConnectionFormat is an acceptable format for the range
//		DataRange. In this case OldFormat is NULL.
//
//		2: after pPin's creation dispatch has been made and an initial format
//		selected in order to change the format for the pin. In this case,
//		OldFormat will not be NULL.
//
//		Validate that the format is acceptible and perform the actions necessary
//		to change format if appropriate.
//
//		STATUS_SUCCESS - The format is acceptable / the format has been changed
//
//		STATUS_NO_MATCH - The format is not-acceptable / the format has not
//		been changed
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// pPin: IN
//		Pointer to the KSPIN structure for which the data format is changing.
//
// OldFormat: IN
//		Optional.
//		Pointer to a KSDATAFORMAT structure. Minidrivers can use this field
//		to determine the data format that the pin was using before this call.
//		If NULL, indicates that no data format has been set for the pin and
//		that pPin’s create dispatch has not yet been made.
//		A NULL value here indicates that this routine was called at
//		initialization time for format verification.
//
//	OldAttributeList: IN
//		Optional.
//		Pointer to a KSMULTIPLE_ITEM structure that stores attributes for the
//		previous format. 
//
//	DataRange: IN
//		Pointer to a KSDATARANGE structure. The data range for the new format.
//
//	AttributeRange: IN
//		Optional.
//		The attribute range for the new format. 
//
// --------------------------------------------------------------------------
// Return:
//		Should return STATUS_SUCCESS or an error message.
//
#pragma CODE_LOCKED
NTSTATUS driver_c::Pin_SetDataFormat_Wave
(
	IN PKSPIN						pPin,
	IN PKSDATAFORMAT				pOldFormat OPTIONAL,
	IN PKSMULTIPLE_ITEM				pOldAttributeList OPTIONAL,
	IN const PKSDATARANGE_AUDIO		pAudioDataRange,
	IN const KSATTRIBUTE_LIST*		pAttributeRange OPTIONAL
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
#endif

	NTSTATUS								Status;
	NTSTATUS								Status_locked;

//Initialize variables.
	//Cards...
	card_c*				pCard = NULL;
	card_c*				pCard_master = NULL;

	Status = pDriver->GetCard_FromPin( pPin, &pCard );
	if ( SUCCESS( Status ) )
	{
		pCard_master = pCard->GetMasterCard();
	}

	//Channel...
	channel_base_c*		pChannel_base = NULL;

	if ( SUCCESS( Status ) )
	{
		Status = pCard->GetChannel_FromPin_Wave( pPin, &pChannel_base );
	}
#ifdef DBG_AND_LOGGER
	if ( SUCCESS( Status ) )
	{
		pLogger->Log( "  Pin Type: %s\n", ( pChannel_base->m_Format == channel_base_c::CHANNEL_FORMAT_STEREO ) ? "Stereo" : "Surround" );
	}
#endif

	//Connection format...
	PKSDATAFORMAT_WAVEFORMATEXTENSIBLE		pConnectionFormat = (PKSDATAFORMAT_WAVEFORMATEXTENSIBLE)pPin->ConnectionFormat;

//Lock the driver (protect against deviceIoControl).
	if ( SUCCESS( Status ) )
	{
		Status = Status_locked = pDriver->LockDriver();
	}

//Is this an ASIO channel (NON and WDM must be allowed)?
//
//We don't have to acquire channelInfo spinLock as AVStream can't call
//any other procedure while we are in here and LockDriver protects us agains
//ASIO state changes.
	if ( SUCCESS( Status ) )
	{
		if ( pChannel_base->m_Type_client == channel_base_c::CHANNEL_TYPE_CLIENT_ASIO )
		{
			Status = STATUS_NO_MATCH;
		}
	}

//Check if channel is in RUNing state.
	if ( SUCCESS( Status ) )
	{
		if ( pChannel_base->m_State == KSSTATE_RUN )
		{
			Status = STATUS_NO_MATCH;
		}
	}

//Test descriptorDataRange.
//
//DX is using the WAVEFORMATEX structures too, so just to be 100% sure.
	if ( SUCCESS( Status ) )
	{
		if ( (BOOL)TRUE == IsEqualGUID( pAudioDataRange->DataRange.Specifier, KSDATAFORMAT_SPECIFIER_DSOUND ) )
		{
		#ifdef DBG_AND_LOGGER
			pLogger->Log( "  ERROR: descriptorDataRange is set as KSDATAFORMAT_SPECIFIER_DSOUND\n" );
		#endif
			Status = STATUS_NO_MATCH;
		}
	}

//Validate variables.
	unsigned long		formatSize;

	if ( SUCCESS( Status ) )
	{
	//Validate format.
		if (
		  ( pAudioDataRange->DataRange.FormatSize != sizeof( KSDATARANGE_AUDIO ) )
		  ||
		  ( pAudioDataRange->MaximumSampleFrequency < pAudioDataRange->MinimumSampleFrequency )
		  ||
		  ( pAudioDataRange->MaximumBitsPerSample < pAudioDataRange->MinimumBitsPerSample )
		  ||
		  (
		    (
		      ( pChannel_base->m_Format == channel_base_c::CHANNEL_FORMAT_STEREO )
		      &&
		      ( pAudioDataRange->MaximumChannels != 2 )
		    )
		    ||
		    (
		      ( pChannel_base->m_Format == channel_base_c::CHANNEL_FORMAT_SURROUND )
		      &&
		      ( pAudioDataRange->MaximumChannels > (ULONG)( ( pCard->m_Type != card_c::CARD_TYPE_824 ) ? 4 : 8 ) )
		    )
		  )
		)
		{
		#ifdef DBG_AND_LOGGER
			pLogger->Log( "  ERROR: AudioDataRange validation failed (MaxCh: %d)\n", pAudioDataRange->MaximumChannels );
		#endif
			Status = STATUS_NO_MATCH;
		}

	//Format size and format specifier.
		if ( SUCCESS( Status ) )
		{
			formatSize = pPin->ConnectionFormat->FormatSize;

		//Format size.
			//MME
			if ( formatSize < sizeof( KSDATAFORMAT_WAVEFORMATEXTENSIBLE ) )
			{
			//WDM
				if ( formatSize < sizeof( KSDATAFORMAT_WAVEFORMATEX ) )
				{
					Status = STATUS_BUFFER_TOO_SMALL;
				}
			}

		//Format Specifier.
			if ( SUCCESS( Status ) )
			{
				if ( (BOOL)TRUE != IsEqualGUID( pPin->ConnectionFormat->Specifier, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX ) )
				{
				#ifdef DBG_AND_LOGGER
					pLogger->Log( "  ERROR: wrong connection format specifier\n" );
				#endif

					Status = STATUS_NO_MATCH;
				}
			}
		}
	}

//Prepare variables.
	unsigned long		nSamplesPerSec;

	if ( SUCCESS( Status ) )
	{
	//Sample rate.
		nSamplesPerSec = pConnectionFormat->WaveFormatEx.Format.nSamplesPerSec;
		//Convert sampleRate to driver sampleRate value.
		switch ( nSamplesPerSec )
		{
		case 44100 :
		case 48000 :
			break;

		case 88200 :
		case 96000 :
		//Check if master and slave cards can handle this sample rate.
			Status = pCard_master->CanMasterAndSlaveCardsHandleRisedSampleRate();
			break;

		default:
			Status = STATUS_NO_MATCH;
			break;
		}
	}

//Do we have an acceptable format?
	if ( SUCCESS( Status ) )
	{
	//Sample rate.
	//
	//Protect sampleRate manipulation process ->this is already done by LockDriver.
		if ( pOldFormat == NULL )
		{
		//This channel is not opened yet -> it doesn't own a sampleRate yet.
		//
		//Can we set the sampleRate?
			if ( pCard_master->m_SampleRate_owned > 0 )
			{
			//It's owned -> we can't change the samplerate ->
			//check if sampleRate is the same?
				if ( nSamplesPerSec != (unsigned long)pCard_master->m_SampleRate )
				{
					Status = STATUS_NO_MATCH;
				}
			}
		}
		else
		{
		//This channel is already opened.
		//
		//Do we have to (and can we) change sample rate?
			if ( nSamplesPerSec != (unsigned long)pCard_master->m_SampleRate )
			{
			//Is this channel the sample rate owner?
				if ( pCard_master->m_SampleRate_owned == 1 )
				{
				//It's owned by this channel -> unlock sampleRate.
					--pCard_master->m_SampleRate_owned;

				//Set sampleRate.
					Status = pCard_master->SetSampleRate( nSamplesPerSec );

				//Lock sampleRate.
					++pCard_master->m_SampleRate_owned;

				//Send new sampleRate notification to ControlPanel.
					if ( SUCCESS( Status ) )
					{
					//Send notification to GUI.
					#ifdef DBG_AND_LOGGER
						pLogger->Log( "  pDriver->m_GUI_notifyEvent: 0x%p\n", pDriver->m_GUI_notifyEvent );
					#endif
						KIRQL		spinLock_oldLevel;

						KeAcquireSpinLock( &pDriver->m_GUI_notifyEvent_spinLock, &spinLock_oldLevel );

						//Is GUI notification set?
							if ( pDriver->m_GUI_notifyEvent != NULL )
							{
							//Yes -> set new event.
								pDriver->m_GUI_notifyEvent_master->SetEvent_SampleRate( pCard_master->m_SerialNumber, nSamplesPerSec );

							//Fire notification.
								KeSetEvent( pDriver->m_GUI_notifyEvent, EVENT_INCREMENT, FALSE );
							}

						KeReleaseSpinLock( &pDriver->m_GUI_notifyEvent_spinLock, spinLock_oldLevel );
					}
				}
				else
				{
				//The samplerate is locked to more than 1 channel.
					Status = STATUS_NO_MATCH;
				}
			}
		}
	}

//Other format variables.
//
//Driver supports: DX, MME and WDM
//
	if ( SUCCESS( Status ) )
	{
		WORD	cbSize = pConnectionFormat->WaveFormatEx.Format.cbSize;
		WORD	nChannels = pConnectionFormat->WaveFormatEx.Format.nChannels;

	//wFormatTag
		WORD	wFormatTag = pConnectionFormat->WaveFormatEx.Format.wFormatTag;

		if ( wFormatTag == WAVE_FORMAT_EXTENSIBLE )
		{
			if ( formatSize < sizeof( KSDATAFORMAT_WAVEFORMATEXTENSIBLE ) )
			{
				Status = STATUS_BUFFER_TOO_SMALL;
			}
		}
		else if ( wFormatTag == WAVE_FORMAT_PCM )
		{
			if ( formatSize < sizeof( KSDATAFORMAT_WAVEFORMATEX ) )
			{
				Status = STATUS_BUFFER_TOO_SMALL;
			}
		}
		else
		{
		#ifdef DBG_AND_LOGGER
			pLogger->Log( "  ERROR: wrong connection format\n" );
		#endif
			Status = STATUS_NO_MATCH;
		}

	//nChannels
		if ( pCard->m_Type != card_c::CARD_TYPE_824 )
		{
		//4xx
			if (
			  ( nChannels < 2 )
			  ||
			  ( nChannels > 4 )
			)
			{
				Status = STATUS_NO_MATCH;
			}
		}
		else
		{
		//824
			if (
			  ( nChannels < 2 )
			  ||
			  ( nChannels > 8 )
			)
			{
				Status = STATUS_NO_MATCH;
			}
		}

	//nAvgBytesPerSec.
		if ( SUCCESS( Status ) )
		{
			if (
			  pConnectionFormat->WaveFormatEx.Format.nAvgBytesPerSec
			  != 
			  (
			    pConnectionFormat->WaveFormatEx.Format.nSamplesPerSec
			    *
			    pConnectionFormat->WaveFormatEx.Format.nBlockAlign
			  )
			)
			{
				Status = STATUS_NO_MATCH;
			}
		}

	//sample containers
		if ( SUCCESS( Status ) )
		{
			WORD	nBlockAlign = pConnectionFormat->WaveFormatEx.Format.nBlockAlign;

			switch ( nBlockAlign/nChannels )
			{
			case 2:
			{
			//2 byte sample container => sampleSize = 16 bit
			//
			//WAVEFORMATEX -> DX_MME_16bit
			//WAVEFORMATEXTENSIBLE -> DX_MME_16bit
			//
			//wBitsPerSample
				if ( pConnectionFormat->WaveFormatEx.Format.wBitsPerSample != 16 )
				{
					Status = STATUS_NO_MATCH;
				}

				if ( cbSize >= ( sizeof( WAVEFORMATEXTENSIBLE ) - sizeof( WAVEFORMATEX ) ) )
				{
				//WAVEFORMATEXTENSIBLE -> DX_MME_16bit
				//
				//wValidBitesPerSample
					if ( pConnectionFormat->WaveFormatEx.Samples.wValidBitsPerSample != 16 )
					{
						Status = STATUS_NO_MATCH;
					}
				}
				else if ( cbSize != 0 )
				{
					Status = STATUS_NO_MATCH;
				}

			//Update channelInfo
				if ( SUCCESS( Status ) )
				{
				//Acquire channelInfo spinLock.
					KIRQL		spinLock_oldLevel;

					KeAcquireSpinLock( &pDriver->m_SpinLock_channel_wave, &spinLock_oldLevel );

					//(If copyBuffersProc is != NULL the channel is opened -> update the variables.)
						if ( pChannel_base->m_pCopyBuffersProcedure != NULL )
						{
							Status = pChannel_base->Set_CopyBuffersProcedure_AVStream( nChannels, channel_base_c::CHANNEL_BIT_SIZE_16 );
						}

					KeReleaseSpinLock( &pDriver->m_SpinLock_channel_wave, spinLock_oldLevel );
				}
				break;
			}
			case 3 :
			{
			//3 byte sample container => sampleSize = 24 bit
			//
			//WAVEFORMATEX -> DX_MME_24bit
			//WAVEFORMATEXTENSIBLE -> DX_MME_24bit
			//
			//wBitsPerSample
				if ( pConnectionFormat->WaveFormatEx.Format.wBitsPerSample != 24 )
				{
					Status = STATUS_NO_MATCH;
				}
			
				if ( cbSize >= ( sizeof( WAVEFORMATEXTENSIBLE ) - sizeof( WAVEFORMATEX ) ) )
				{
				//WAVEFORMATEXTENSIBLE -> DX_MME_24bit
				//
				//wValidBitesPerSample
					if ( pConnectionFormat->WaveFormatEx.Samples.wValidBitsPerSample != 24 )
					{
						Status = STATUS_NO_MATCH;
					}
				}
				else if ( cbSize != 0 )
				{
					Status = STATUS_NO_MATCH;
				}
			
			//Update channelInfo
				if ( SUCCESS( Status ) )
				{
				//Acquire channelInfo spinLock.
					KIRQL		spinLock_oldLevel;

					KeAcquireSpinLock( &pDriver->m_SpinLock_channel_wave, &spinLock_oldLevel );

					//(If copyBuffersProc is != NULL the channel is opened -> update the variables.)
						if ( pChannel_base->m_pCopyBuffersProcedure != NULL )
						{
							Status = pChannel_base->Set_CopyBuffersProcedure_AVStream( nChannels, channel_base_c::CHANNEL_BIT_SIZE_24 );
						}

					KeReleaseSpinLock( &pDriver->m_SpinLock_channel_wave, spinLock_oldLevel );
				}
				break;
			}
			case 4 :
			{
			//4 byte sample container => sampleSize = 8,16,24 bit
			//
			//WAVEFORMATEX -> DX_MME_32bit
			//WAVEFORMATEXTENSIBLE -> DX_MME_32bit
			//
			//wBitsPerSample
				if ( pConnectionFormat->WaveFormatEx.Format.wBitsPerSample != 32 )
				{
					Status = STATUS_NO_MATCH;
				}

				if ( cbSize >= ( sizeof( WAVEFORMATEXTENSIBLE ) - sizeof( WAVEFORMATEX ) ) )
				{
				//WAVEFORMATEXTENSIBLE -> DX_MME_32bit
				//
				//wValidBitesPerSample
					if ( pConnectionFormat->WaveFormatEx.Samples.wValidBitsPerSample > 24 )
					{
						Status = STATUS_NO_MATCH;
					}
				}
				else if ( cbSize != 0 )
				{
					Status = STATUS_NO_MATCH;
				}

			//Update channelInfo
				if ( SUCCESS( Status ) )
				{
				//Acquire channelInfo spinLock.
					KIRQL		spinLock_oldLevel;

					KeAcquireSpinLock( &pDriver->m_SpinLock_channel_wave, &spinLock_oldLevel );

					//(If copyBuffersProc is != NULL the channel is opened -> update the variables.)
						if ( pChannel_base->m_pCopyBuffersProcedure != NULL )
						{
							Status = pChannel_base->Set_CopyBuffersProcedure_AVStream( nChannels, channel_base_c::CHANNEL_BIT_SIZE_32 );
						}

					KeReleaseSpinLock( &pDriver->m_SpinLock_channel_wave, spinLock_oldLevel );
				}
				break;
			}
			default:
				Status = STATUS_NO_MATCH;
				break;
			}
		}
	}

//Unlock the driver.
	if ( SUCCESS( Status_locked ) )
	{
		pDriver->UnlockDriver();
	}

#ifdef DBG_AND_LOGGER
	pLogger->Log(
	  logger_c::LOGGER_SEND_TO_LOG_FILE,
	  logger_c::LS_Driver_AVStream_Pin_Format_SetDataFormat,
	  pOldFormat,
	  pAudioDataRange,
	  pConnectionFormat
	);
	Procedure_Exit( Status );
#endif
	return Status;
}
