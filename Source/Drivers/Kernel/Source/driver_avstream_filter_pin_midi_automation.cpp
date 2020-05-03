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
#include "card_altera_firmware.h"
#include "driver_avstream_filter_register.h"

#pragma DATA_LOCKED

//---------------------------------------------------------------------------------------
//-- MIDI Property pages ----------------------------------------------------------------
//---------------------------------------------------------------------------------------
//NOTICE:
//We assume that all property procedures are called at PASSIVE_LEVEL.
#pragma CODE_LOCKED
NTSTATUS driver_c::PropertyItem_Midi_Synth
(
	IN PIRP					pIrp,
	IN PKSIDENTIFIER		pRequest,
	IN OUT PVOID			pData
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
#endif

	NTSTATUS			Status = STATUS_SUCCESS;

//Process.
	switch ( pRequest->Id )
	{
	case KSPROPERTY_SYNTH_PORTPARAMETERS :
	{
	#ifdef GL_LOGGER
		pLogger->Log( "KSPROPERTY_SYNTH_PORTPARAMETERS:\n" );
	#endif

	//Initialize data structure.
		SYNTH_PORTPARAMS*	pSynth = (SYNTH_PORTPARAMS*)pData;
		RtlCopyMemory( pSynth, ( (char*)pRequest + sizeof( KSNODEPROPERTY ) ), sizeof( SYNTH_PORTPARAMS ) );

	#ifdef DBG_AND_LOGGER
		pLogger->Log( "KSPROPERTY_SYNTH_PORTPARAMETERS(before):\n" );
		pLogger->Log( "  ValidParams: 0x%X\n", pSynth->ValidParams );
		pLogger->Log( "  Voices: %d\n", pSynth->Voices );
		pLogger->Log( "  ChannelGroups: %d\n", pSynth->ChannelGroups );
		pLogger->Log( "  AudioChannels: %d\n", pSynth->AudioChannels );
		pLogger->Log( "  SampleRate: %d\n", pSynth->SampleRate );
		pLogger->Log( "  EffectsFlags: 0x%X\n", pSynth->EffectsFlags );
		pLogger->Log( "  Share: %d\n", pSynth->Share );
	#endif

	//Set structure.
	//
	//The code below is almost an exact copy of dmusuart example from DDK.
		if ( ( pSynth->ValidParams & ~SYNTH_PORTPARAMS_CHANNELGROUPS ) != 0 )
		{
			pSynth->ValidParams &= SYNTH_PORTPARAMS_CHANNELGROUPS;
		}
		if ( ( pSynth->ValidParams & SYNTH_PORTPARAMS_CHANNELGROUPS ) != 0 )
		{
			if ( pSynth->ChannelGroups != 1 )
			{
				pSynth->ChannelGroups = 1;
				Status = STATUS_NOT_ALL_ASSIGNED;
			}
		}
		else
		{
			pSynth->ChannelGroups = 1;
		}
		#ifdef DBG_AND_LOGGER
			pLogger->Log( "KSPROPERTY_SYNTH_PORTPARAMETERS(after):\n" );
			pLogger->Log( "  ValidParams: 0x%X\n", pSynth->ValidParams );
			pLogger->Log( "  Voices: %d\n", pSynth->Voices );
			pLogger->Log( "  ChannelGroups: %d\n", pSynth->ChannelGroups );
			pLogger->Log( "  AudioChannels: %d\n", pSynth->AudioChannels );
			pLogger->Log( "  SampleRate: %d\n", pSynth->SampleRate );
			pLogger->Log( "  EffectsFlags: 0x%X\n", pSynth->EffectsFlags );
			pLogger->Log( "  Share: %d\n", pSynth->Share );
		#endif
		break;
	}
	case KSPROPERTY_SYNTH_CHANNELGROUPS :
	{
	#ifdef GL_LOGGER
		pLogger->Log( "KSPROPERTY_SYNTH_CHANNELGROUPS\n" );
	#endif

	//Set data variable.
		if ( ( pRequest->Flags & KSPROPERTY_TYPE_GET ) != 0 )
		{
			*( (unsigned long*)pData ) = 0;

		#ifdef DBG_AND_LOGGER
			pLogger->Log( "KSPROPERTY_SYNTH_CHANNELGROUPS: KSPROPERTY_TYPE_GET: data = 0\n" );
		#endif
		}
	#ifdef DBG_AND_LOGGER
		else if ( ( pRequest->Flags & KSPROPERTY_TYPE_SET ) != 0 )
		{
			pLogger->Log( "KSPROPERTY_SYNTH_CHANNELGROUPS: KSPROPERTY_TYPE_SET: data = %d\n", *( (unsigned long*)pData ) );
		}
	#endif
		break;
	}
	default:
	{
		Status = STATUS_UNSUCCESSFUL;

		#ifdef DBG_AND_LOGGER
			pLogger->Log( "ERROR: Unknown case: %d\n", pRequest->Id );
		#endif
		break;
	}
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

#pragma CODE_LOCKED
NTSTATUS driver_c::PropertyItem_Midi_Synth_Capabilities_Get
(
  IN PIRP				pIrp,
  IN PKSIDENTIFIER		pRequest,
  IN OUT SYNTHCAPS*		pData
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
#endif

	NTSTATUS		Status = STATUS_SUCCESS;

//Initialize variables.
	//Filter...
	PKSFILTER	pFilter = KsGetFilterFromIrp( pIrp );
	if ( pFilter == NULL )
	{
		Status = STATUS_UNSUCCESSFUL;
	}

	//Cards...
	card_c*			pCard = NULL;

	if ( SUCCESS( Status ) )
	{
		Status = pDriver->GetCard_FromFilter( pFilter, &pCard );
	}

	//Node...
	unsigned long	nNode = KsGetNodeIdFromIrp( pIrp );
	switch ( nNode )
	{
	case SYNTH_NODE_0:
	case SYNTH_NODE_1:
	{
	//IRP was send to SYNTH node -> DX MIDI.
		//Initialize data structure.
		RtlZeroMemory( pData, sizeof( SYNTHCAPS ) );
		break;
	}
	case KSFILTER_NODE:
	default:
	{
	//IRP was send directly to filter -> MME MIDI -> this procedure is intended only for DX Synth node -> ignore request.
		Status = STATUS_UNSUCCESSFUL;
		break;
	}
	}

//Set return data structure.
	if ( SUCCESS( Status ) )
	{
	//Define card type.
		unsigned long			cardType;

		switch ( pCard->m_Type )
		{
		case card_c::CARD_TYPE_424:
			cardType = 424;
			break;
		case card_c::CARD_TYPE_496:
			cardType = 496;
			break;
		case card_c::CARD_TYPE_824:
			cardType = 824;
			break;
		};

	//Guid.
		if ( nNode == SYNTH_NODE_0 )
		{
		//In.
			pData->Guid = CLSID_MiniportDriverDMusUARTCapture;
		}
		else
		{
		//Out.
			pData->Guid = CLSID_MiniportDriverDMusUART;
		}

	//Description.
		swprintf( (WCHAR*)&pData->Description, L"WavePro%d(%d) DX Midi", cardType, pCard->m_SerialNumber+1 );

	//Flags.
		pData->Flags = SYNTH_PC_EXTERNAL;

	//MemorySize.
		pData->MemorySize = 0;

	//MaxChannelGroups
		pData->MaxChannelGroups = 1;

	//MaxVoices
		pData->MaxVoices = (DWORD)-1;

	//MaxAudioChannels
		pData->MaxAudioChannels = (DWORD)-1;

	//EffectFlags
		pData->EffectFlags = SYNTH_EFFECT_NONE;
	}

#ifdef DBG_AND_LOGGER
	pLogger->Log( "SYNTHCAPS:\n" );
	pLogger->Log( "  Guid:\n" );
	pLogger->Log( "  Flags: 0x%X\n", pData->Flags );
	pLogger->Log( "  MemorySize: %d\n", pData->MemorySize );
	pLogger->Log( "  MaxChannelGroups: %d\n", pData->MaxChannelGroups );
	pLogger->Log( "  MaxVoices: %d\n", pData->MaxVoices );
	pLogger->Log( "  MaxAudioChannels: %d\n", pData->MaxAudioChannels );
	pLogger->Log( "  EffectFlags: 0x%X\n", pData->EffectFlags );
	pLogger->Log( "  Description: %S\n", pData->Description );

	Procedure_Exit( Status );
#endif
	return Status;
}

#pragma CODE_LOCKED
NTSTATUS driver_c::PropertyItem_Midi_Synth_Clock
(
  IN PIRP				pIrp,
  IN PKSIDENTIFIER		pRequest,
  IN OUT ULONGLONG*		pData
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
#endif

	NTSTATUS		Status = STATUS_SUCCESS;

////Initialize variables.
//	PKSPIN			pPin = KsGetPinFromIrp( pIrp );
//	if ( pPin == NULL )
//	{
//		Status = STATUS_UNSUCCESSFUL;
//	}
//
//	//Cards...
//	card_c*			pCard = NULL;
//
//	if ( SUCCESS( Status ) )
//	{
//		Status = pDriver->GetCard_FromPin( pPin, &pCard );
//	}
//
//	//Channel...
//	channel_midi_c*		pChannel_midi = NULL;
//
//	if ( SUCCESS( Status ) )
//	{
//		Status = pCard->GetChannel_FromPin_Midi( pPin, &pChannel_midi );
//	}
//
//	//Spinlock...
//	PKSPIN_LOCK			pSpinLock = NULL;
//
//	if ( SUCCESS( Status ) )
//	{
//		if ( pChannel_midi->m_Type == channel_midi_c::CHANNEL_TYPE_IN )
//		{
//		//Input...
//			pSpinLock = &pCard->m_SpinLock_channel_midi_in;
//		}
//		else
//		{
//		//Output...
//			pSpinLock = &pCard->m_SpinLock_channel_midi_out;
//		}
//	}
//
////Set return timestamp.
//	if ( SUCCESS( Status ) )
//	{
//		ULONGLONG			timestamp_current;
//		ULONGLONG			timestamp_streamPause;
//		ULONGLONG			timestamp_streamStart;
//		unsigned long		timestamp_streamStart_isSet;
//
//	//Acquire channel spinLock.
//		KIRQL			spinLock_oldLevel;
//
//		KeAcquireSpinLock( pSpinLock, &spinLock_oldLevel );
//
//		//Get start timestamp.
//			timestamp_streamPause= pChannel_midi->m_Timestamp_streamPause;
//			timestamp_streamStart = pChannel_midi->m_Timestamp_streamStart;
//			timestamp_streamStart_isSet = pChannel_midi->m_Timestamp_streamStart_isSet;
//
//		//Get current timestamp.
//			timestamp_current = KeQueryPerformanceCounter( NULL ).QuadPart;
//
//		KeReleaseSpinLock( pSpinLock, spinLock_oldLevel );
//
//	//Is stream already producing a sound... is it at or past of it's first sample?
//		if ( pChannel_midi->m_Type == channel_midi_c::CHANNEL_TYPE_IN )
//		{
//		//Input...
//			*pData = timestamp_current - timestamp_streamStart;
//		}
//		else
//		{
//		//Output...
//			if (
//			  ( timestamp_streamStart_isSet == YES )
//			  &&
//			  ( timestamp_current >= timestamp_streamStart )
//			)
//			{
//
//			}
//			else
//			{
//			//No clock was set or stream is not producing sound yet.
//				*pData = 0;
//			}
//	}
	//Convert from 100ns to MIDI bytes.

	////Convert 4MHz ticks to 100ns.
	////
	////1 (4MHz) tick = 1/4MHz = 250ns = 2.5 * 100ns = 5/2 * 100ns
	//	if ( clock_current.clock != CLOCK_ZERO_TIMESTAMP )
	//	{
	//	//Update clock.
	//		*pData = ( clock_current.clock * 5 ) / 2;
	//	}
	//	else
	//	{
	//	//Clock was not set yet.
	//		*pData = 0;
	//	}
		*pData = KeQueryPerformanceCounter( NULL ).QuadPart;
	//}

	return Status;
}
