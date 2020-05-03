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
#include "driver_avstream_filter_register.h"

#pragma DATA_LOCKED

//--------------------------------------------------------------------------
//  Filter definitions
//--------------------------------------------------------------------------
const KSPIN_DISPATCH	PinDispatch_Midi = 
{
	&driver_c::Pin_Create_Midi,				//Create
	&driver_c::Pin_Close_Midi,				//Close
	&driver_c::Pin_Process_Midi,			//Process
	NULL,									//Reset
	NULL,									//SetDataFormat
	&driver_c::Pin_SetDeviceState_Midi,		//SetDeviceState
	NULL,									//Connect
	NULL,									//Disconnect
	NULL,									//Clock
	NULL									//Allocator
};


//÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷
//Midi Pin
//As you can not get the card object from node in Property procedures we need to define
//specific automation table for every card separately.
const KSPROPERTY_ITEM PropertyItem_Synth_Node[4] =
{
	DEFINE_KSPROPERTY_ITEM
	(
	  KSPROPERTY_SYNTH_CAPS,												//PropertyId
	  (PFNKSHANDLER)&driver_c::PropertyItem_Midi_Synth_Capabilities_Get,		//GetPropertyHandler
	  sizeof(KSNODEPROPERTY),												//MinProperty
	  sizeof(SYNTHCAPS),													//MinData
	  NULL,																	//SetPropertyHandler
	  NULL,																	//Values
	  0,																	//RelationsCount
	  NULL,																	//Relations
	  NULL,																	//SupportHandler
	  0
	),
	DEFINE_KSPROPERTY_ITEM
	(
	  KSPROPERTY_SYNTH_CHANNELGROUPS,										//PropertyId
	  (PFNKSHANDLER)driver_c::PropertyItem_Midi_Synth,						//GetPropertyHandler
	  sizeof(KSNODEPROPERTY),												//MinProperty
	  sizeof(ULONG),														//MinData
	  (PFNKSHANDLER)driver_c::PropertyItem_Midi_Synth,						//SetPropertyHandler
	  NULL,																	//Values
	  0,																	//RelationsCount
	  NULL,																	//Relations
	  NULL,																	//SupportHandler
	  0
	),
	DEFINE_KSPROPERTY_ITEM
	(
	  KSPROPERTY_SYNTH_PORTPARAMETERS,										//PropertyId
	  (PFNKSHANDLER)driver_c::PropertyItem_Midi_Synth,						//GetPropertyHandler
	  sizeof(KSNODEPROPERTY) + sizeof(SYNTH_PORTPARAMS),					//MinProperty
	  sizeof(SYNTH_PORTPARAMS),												//MinData
	  NULL,																	//SetPropertyHandler
	  NULL,																	//Values
	  0,																	//RelationsCount
	  NULL,																	//Relations
	  NULL,																	//SupportHandler
	  0
	),
	DEFINE_KSPROPERTY_ITEM
	(
	  KSPROPERTY_SYNTH_LATENCYCLOCK,										//PropertyId
	  (PFNKSHANDLER)driver_c::PropertyItem_Midi_Synth_Clock,				//GetPropertyHandler
	  sizeof(KSNODEPROPERTY),												//MinProperty
	  sizeof(ULONGLONG),													//MinData
	  NULL,																	//SetPropertyHandler
	  NULL,																	//Values
	  0,																	//RelationsCount
	  NULL,																	//Relations
	  NULL,																	//SupportHandler
	  0
	)
};

const KSPROPERTY_SET PropertyTable_Synth_Node[1] =
{
	DEFINE_KSPROPERTY_SET
	(
	  &KSPROPSETID_Synth,
	  SIZEOF_ARRAY( PropertyItem_Synth_Node ),
	  PropertyItem_Synth_Node,
	  0,
	  NULL
	)
};


DEFINE_KSAUTOMATION_TABLE ( AutomationTable_Synth_Node )
{
	DEFINE_KSAUTOMATION_PROPERTIES ( PropertyTable_Synth_Node ),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS_NULL
};

const KSPIN_INTERFACE PinInterfaces_Midi[1]  = 
{
	{
		STATICGUIDOF( KSINTERFACESETID_Standard ),
		KSINTERFACE_STANDARD_STREAMING,
		0
	}
};

const KSPIN_MEDIUM PinMediums_Midi[1]  = 
{
	{
		STATICGUIDOF( KSMEDIUMSETID_Standard ),
		KSMEDIUM_TYPE_ANYINSTANCE,
		0
	}
};

const KSDATARANGE_MUSIC PinWaveIoRange_Midi_MME =
{
	{
		sizeof(KSDATARANGE_MUSIC),
		0,
		0,
		0,
		STATICGUIDOF( KSDATAFORMAT_TYPE_MUSIC ),
		STATICGUIDOF( KSDATAFORMAT_SUBTYPE_MIDI ),
		STATICGUIDOF( KSDATAFORMAT_SPECIFIER_NONE )
	},
	STATICGUIDOF( KSMUSIC_TECHNOLOGY_PORT ),
	0,
	0,
	0xFFFF
};
//Define another to allow DX drivers too.
const KSDATARANGE_MUSIC PinWaveIoRange_Midi_DX =
{
	{
		sizeof( KSDATARANGE_MUSIC ),
		0,
		0,
		0,
		STATICGUIDOF( KSDATAFORMAT_TYPE_MUSIC ),
		STATICGUIDOF( KSDATAFORMAT_SUBTYPE_DIRECTMUSIC ),
		STATICGUIDOF( KSDATAFORMAT_SPECIFIER_NONE )
	},
	STATICGUIDOF( KSMUSIC_TECHNOLOGY_PORT ),
	0,
	0,
	0xFFFF
};
const PKSDATARANGE PinAudioFormatRanges_Midi_MME[1]  = 
{
	(PKSDATARANGE) &PinWaveIoRange_Midi_MME
};
const PKSDATARANGE PinAudioFormatRanges_Midi_DX[1]  = 
{
	(PKSDATARANGE) &PinWaveIoRange_Midi_DX
};
//const PKSDATARANGE PinAudioFormatRanges_Midi_Both[2]  = 
//{
//	(PKSDATARANGE) &PinWaveIoRange_Midi_MME,
//	(PKSDATARANGE) &PinWaveIoRange_Midi_DX
//};

GUID pinCategory_Midi = { STATIC_KSCATEGORY_AUDIO };
GUID pinName_Midi = { STATIC_KSAUDFNAME_MIDI };
GUID pinName_Midi_DX_In = { STATIC_KSAUDFNAME_DMUSIC_MPU_IN };
GUID pinName_Midi_DX_Out = { STATIC_KSAUDFNAME_DMUSIC_MPU_OUT };

//÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷
//Bridge Pin
static KSDATARANGE PinWaveIoRange_MidiBridge[] =
{
	{
		sizeof( KSDATARANGE ),
		0,
		0,
		0,
		STATICGUIDOF( KSDATAFORMAT_TYPE_MUSIC ),
		STATICGUIDOF( KSDATAFORMAT_SUBTYPE_MIDI_BUS ),
		STATICGUIDOF( KSDATAFORMAT_SPECIFIER_NONE )
	}
};

const PKSDATARANGE PinAudioFormatRanges_MidiBridge[1] = 
{
	(PKSDATARANGE) &PinWaveIoRange_MidiBridge
};

GUID pinCategory_bridge_Midi = { STATIC_KSCATEGORY_AUDIO };
//GUID pinCategory_bridge_Midi = { STATIC_KSNODETYPE_MIDI_JACK };
GUID pinName_bridge_Midi = { STATIC_KSAUDFNAME_MIDI };

//÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷
//Pins descriptors:
//
//Every stereo IN/OUT is one filter.
//If not done this way the system can't see all the channels
//(only the first stereo IN/OUT).
//
//Card structure:
//e.g. 4xx
//             -------------------------------
// WaveIn1,2 ->|Pin       |       |Pin       |-> WaveOut1,2
//             |Factory 0 |       |Factory 0 |
//             |----------|   C   |----------|
// WaveIn3,4 ->|Pin       |   A   |Pin       |-> WaveOut3,4
//             |Factory 1 |   R   |Factory 1 |
//             |----------|   D   |----------|
// MidiIn    ->|Pin       |       |Pin       |-> MidiOut
//             |Factory 2 |       |Factory 2 |
//             -------------------------------
//
//
//Detailed filter description (all card types):
//
//                   --------------------------
//                   | "Filter"               |
//                   |  (1)--------------(0)  |
//                   | ->o-|SYNTH Node 0|-o>--|--o 1  Sink Pin
//                   | |   --------------     |      (MidiIn DX)
//  Bridge Pin  2 o--|-|                      |
//                   | |                      |
//                   | -----------------------|--o 0  Sink Pin
//                   |                        |      (MidiIn MME)
//                   |                        |
//                   |  (1)--------------(0)  |
//    Sink Pin  4 o--|-->o-|SYNTH Node 2|-o>- |
//  (MidiOut DX)     |     --------------   | |
//                   |                      --|--o 5  Bridge Pin
//                   |                      | |
//    Sink Pin  3 o--|----------------------- |
//  (MidiOut MME)    |                        |
//                   --------------------------
//
//Notice:
//DX Pin must be the first pin present in the filter. As only the first
//pin can expose the clock.
//
KSPIN_DESCRIPTOR_EX KsPinDescriptors_Midi[]  = 
{
//MidiIn
	//Sink Pin MME
	{
		&PinDispatch_Midi,
		NULL, //&AutomationTable_Midi,
		{
			SIZEOF_ARRAY( PinInterfaces_Midi ),
			(KSPIN_INTERFACE*) &PinInterfaces_Midi,
			SIZEOF_ARRAY( PinMediums_Midi ),
			(KSPIN_MEDIUM*) &PinMediums_Midi,
			SIZEOF_ARRAY( PinAudioFormatRanges_Midi_MME ),
			PinAudioFormatRanges_Midi_MME,
			KSPIN_DATAFLOW_OUT,
			KSPIN_COMMUNICATION_SINK,
			&pinCategory_Midi,
			&pinName_Midi,
			0
		},
		KSPIN_FLAG_DISPATCH_LEVEL_PROCESSING |
		  //Directly poll the queue at DPC time( transport data to hardware ).
		  KSPIN_FLAG_DO_NOT_INITIATE_PROCESSING |
		  //Pin is responsible to check for available frames.
		  KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING |
		  KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY |
		  //Change status one step at a time.
		  KSPIN_FLAG_USE_STANDARD_TRANSPORT,
		1,	//InstancesPossible
		0,	//InstancesNecessary
		NULL,
		NULL	//&AudioIntersectHandler
	},
	////Sink Pin DX
	//{
	//	&PinDispatch_Midi,
	//	NULL, //&AutomationTable_Midi,
	//	{
	//		SIZEOF_ARRAY( PinInterfaces_Midi ),
	//		(KSPIN_INTERFACE*) &PinInterfaces_Midi,
	//		SIZEOF_ARRAY( PinMediums_Midi ),
	//		(KSPIN_MEDIUM*) &PinMediums_Midi,
	//		SIZEOF_ARRAY( PinAudioFormatRanges_Midi_DX ),
	//		PinAudioFormatRanges_Midi_DX,
	//		KSPIN_DATAFLOW_OUT,
	//		KSPIN_COMMUNICATION_SINK,
	//		&pinCategory_Midi,
	//		&pinName_Midi_DX_In,
	//		0
	//	},
	//	KSPIN_FLAG_DISPATCH_LEVEL_PROCESSING |
	//	  //Directly poll the queue at DPC time( transport data to hardware ).
	//	  KSPIN_FLAG_DO_NOT_INITIATE_PROCESSING |
	//	  //Pin is responsible to check for available frames.
	//	  KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING |
	//	  KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY |
	//	  //Change status one step at a time.
	//	  KSPIN_FLAG_USE_STANDARD_TRANSPORT,// |
	//	  //This pin exposes the clock.
	//	  //KSPIN_FLAG_IMPLEMENT_CLOCK,
	//	1,	//InstancesPossible
	//	0,	//InstancesNecessary
	//	NULL,
	//	NULL	//&AudioIntersectHandler
	//},
	//Bridge PIN
	{
		NULL,
		NULL,
		{
			SIZEOF_ARRAY( PinInterfaces_Midi ),
			(KSPIN_INTERFACE*) &PinInterfaces_Midi,
			SIZEOF_ARRAY( PinMediums_Midi ),
			(KSPIN_MEDIUM*) &PinMediums_Midi,
			SIZEOF_ARRAY( PinAudioFormatRanges_MidiBridge ),
			PinAudioFormatRanges_MidiBridge,
			KSPIN_DATAFLOW_IN,
			KSPIN_COMMUNICATION_NONE,
			&pinCategory_bridge_Midi,
			&pinName_bridge_Midi,
			0
		},
		0,
		0,
		0,
		NULL,
		NULL
	},
//MidiOut
	//Sink PIN MME
	{
		&PinDispatch_Midi,
		NULL, //&AutomationTable_Midi,
		{
			SIZEOF_ARRAY( PinInterfaces_Midi ),
			(KSPIN_INTERFACE*) &PinInterfaces_Midi,
			SIZEOF_ARRAY( PinMediums_Midi ),
			(KSPIN_MEDIUM*) &PinMediums_Midi,
			SIZEOF_ARRAY( PinAudioFormatRanges_Midi_MME ),
			PinAudioFormatRanges_Midi_MME,
			KSPIN_DATAFLOW_IN,
			KSPIN_COMMUNICATION_SINK,
			&pinCategory_Midi,
			&pinName_Midi,
			0
		},
		//Flags...
		  //Pin is responsible to check for available frames.
		  KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING |
		  KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY |
		  //Change status one step at a time.
		  KSPIN_FLAG_USE_STANDARD_TRANSPORT |
		  KSPIN_FLAG_RENDERER,
		1,	//InstancesPossible
		0,	//InstancesNecessary
		NULL,
		NULL	//&AudioIntersectHandler
	},
	////Sink PIN DX
	//{
	//	&PinDispatch_Midi,
	//	NULL, //&AutomationTable_Midi,
	//	{
	//		SIZEOF_ARRAY( PinInterfaces_Midi ),
	//		(KSPIN_INTERFACE*) &PinInterfaces_Midi,
	//		SIZEOF_ARRAY( PinMediums_Midi ),
	//		(KSPIN_MEDIUM*) &PinMediums_Midi,
	//		SIZEOF_ARRAY( PinAudioFormatRanges_Midi_DX ),
	//		PinAudioFormatRanges_Midi_DX,
	//		KSPIN_DATAFLOW_IN,
	//		KSPIN_COMMUNICATION_SINK,
	//		&pinCategory_Midi,
	//		&pinName_Midi_DX_In,
	//		0
	//	},
	//	//Flags...
	//	  //Pin is responsible to check for available frames.
	//	  KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING |
	//	  KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY |
	//	  //Change status one step at a time.
	//	  KSPIN_FLAG_USE_STANDARD_TRANSPORT |
	//	  KSPIN_FLAG_RENDERER,
	//	1,	//InstancesPossible
	//	0,	//InstancesNecessary
	//	NULL,
	//	NULL	//&AudioIntersectHandler
	//},
	//Bridge PIN
	{
		NULL,
		NULL,
		{
			SIZEOF_ARRAY( PinInterfaces_Midi ),
			(KSPIN_INTERFACE*) &PinInterfaces_Midi,
			SIZEOF_ARRAY( PinMediums_Midi ),
			(KSPIN_MEDIUM*) &PinMediums_Midi,
			SIZEOF_ARRAY( PinAudioFormatRanges_MidiBridge ),
			PinAudioFormatRanges_MidiBridge,
			KSPIN_DATAFLOW_OUT,
			KSPIN_COMMUNICATION_NONE,
			&pinCategory_bridge_Midi,
			&pinName_bridge_Midi,
			0
		},
		0,
		0,
		0,
		NULL,
		NULL
	}
};

//Categories that the driver belongs.
//AUDIO is main category,
//CAPTURE and RENDER are sub categories.
const GUID KsCategories_Midi[] =
{
	STATICGUIDOF( KSCATEGORY_AUDIO ),
	STATICGUIDOF( KSCATEGORY_CAPTURE ),
	STATICGUIDOF( KSCATEGORY_RENDER )
};

const GUID SynthNodeType = {STATICGUIDOF( KSNODETYPE_SYNTHESIZER )};

const KSNODE_DESCRIPTOR KsNodeDescriptors_Midi[] = 
{
	DEFINE_NODE_DESCRIPTOR( &AutomationTable_Synth_Node, &SynthNodeType, NULL ),
	DEFINE_NODE_DESCRIPTOR( &AutomationTable_Synth_Node, &SynthNodeType, NULL )
};

//Filter description (all card types):
//
//                   --------------------------
//                   | "Filter"               |
//                   |  (1)--------------(0)  |
//                   | ->o-|SYNTH Node 0|-o>--|--o 1  Sink Pin
//                   | |   --------------     |      (MidiIn DX)
//  Bridge Pin  2 o--|-|                      |
//                   | |                      |
//                   | -----------------------|--o 0  Sink Pin
//                   |                        |      (MidiIn MME)
//                   |                        |
//                   |  (1)--------------(0)  |
//    Sink Pin  4 o--|-->o-|SYNTH Node 2|-o>- |
//  (MidiOut DX)     |     --------------   | |
//                   |                      --|--o 5  Bridge Pin
//                   |                      | |
//    Sink Pin  3 o--|----------------------- |
//  (MidiOut MME)    |                        |
//                   --------------------------
//
//Notice:
//DX Pin must be the first pin present in the filter. As only the first
//pin can expose the clock.
//
const KSTOPOLOGY_CONNECTION KsConnections_Midi[]  = 
{//        FromNode,         FromPin,       ToNode,          ToPin
//MidiIn
	//{ KSFILTER_NODE,  MIDI_IN_BRIDGE,  SYNTH_NODE_0,  NODE_PIN_INPUT },
	//{  SYNTH_NODE_0, NODE_PIN_OUTPUT, KSFILTER_NODE,      MIDI_IN_DX },

	{ KSFILTER_NODE,  MIDI_IN_BRIDGE, KSFILTER_NODE,     MIDI_IN_MME },

//MidiOut
	//{ KSFILTER_NODE,     MIDI_OUT_DX,  SYNTH_NODE_1,  NODE_PIN_INPUT },
	//{  SYNTH_NODE_1, NODE_PIN_OUTPUT, KSFILTER_NODE, MIDI_OUT_BRIDGE },

	{ KSFILTER_NODE,    MIDI_OUT_MME, KSFILTER_NODE, MIDI_OUT_BRIDGE }
};

//÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷
//Filter factory:
const KSFILTER_DESCRIPTOR FilterDescriptor_Midi = 
{
	NULL,							//Filter Dispatch Table
	NULL,							//Automation Table
	KSFILTER_DESCRIPTOR_VERSION,	//Version
	0,								//Flags
	&KSNAME_Filter,					//Reference GUID
	DEFINE_KSFILTER_PIN_DESCRIPTORS ( KsPinDescriptors_Midi ),
	DEFINE_KSFILTER_CATEGORIES ( KsCategories_Midi ),
	DEFINE_KSFILTER_NODE_DESCRIPTORS( KsNodeDescriptors_Midi ),
	DEFINE_KSFILTER_CONNECTIONS( KsConnections_Midi ),
	NULL
};

//***************************************************************************
// ProcedureName: RegisterChannelsToSystem_Midi/
//                UnregisterChannelsFromSystem_Midi
//
// Explanation: Procedure will register/unregister channel-filter interafaces
//              and set registry values.
//
// IRQ Level: PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return:
//		STATUS_SUCCESS or the error code that was returned
//		from the attempt to perform the operation.
//
#pragma CODE_PAGED
NTSTATUS driver_c::Channels_RegisterToSystem_Midi
(
	IN card_c*		pCard
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	NTSTATUS				Status = STATUS_UNSUCCESSFUL;

	WCHAR					buffer_friendlyName[32];
	WCHAR					buffer_interfaceName[32];
	unsigned long			cardType;
	KSFILTER_DESCRIPTOR*	pFilterDescriptor_Midi;
	const GUID*				pGUID;
	KSPIN_DESCRIPTOR_EX*	pPinDescriptors_Midi;

//Define card type.
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
	pFilterDescriptor_Midi = (KSFILTER_DESCRIPTOR*) &FilterDescriptor_Midi;
	pPinDescriptors_Midi = (KSPIN_DESCRIPTOR_EX*) &KsPinDescriptors_Midi;

//Define filters... register Midi channels.
	//Create strings.
	swprintf( buffer_friendlyName, L"WavePro%d(%d) Midi", cardType, pCard->m_SerialNumber+1 );

	swprintf( buffer_interfaceName, L"GLI%d(%d) Midi", cardType, pCard->m_SerialNumber+1 );

	//Create copy of structures.
	Status = Channels_CopyDescriptors_Midi( &pFilterDescriptor_Midi, &pPinDescriptors_Midi );
	if ( SUCCESS( Status ) )
	{
	//Backup for 'free resources'.
		m_pFilterDescriptors_Midi[pCard->m_SerialNumber] = pFilterDescriptor_Midi;
		m_pPinDescriptors_Midi[pCard->m_SerialNumber] = pPinDescriptors_Midi;
	}

	//Set copied pin as part of filter.
	if ( SUCCESS( Status ) )
	{
		pFilterDescriptor_Midi->PinDescriptors = pPinDescriptors_Midi;
	}

	//Set pins names.
	if ( SUCCESS( Status ) )
	{
		pGUID = NULL;
		Status = Interface_SetPinNames( buffer_friendlyName, &pGUID );
		if ( SUCCESS( Status ) )
		{
		//Replace dummy GUID with drivers one.
			pPinDescriptors_Midi[MIDI_IN_BRIDGE].PinDescriptor.Name = pGUID;
			pPinDescriptors_Midi[MIDI_OUT_BRIDGE].PinDescriptor.Name = pGUID;
		}
	}

	//Register filter interface.
	if ( SUCCESS( Status ) )
	{
		Status = Interface_Register_Filter( pCard, buffer_interfaceName, buffer_friendlyName );
	}

	//Register filter to system.
	if ( SUCCESS( Status ) )
	{
		Status = KsCreateFilterFactory(
			pCard->m_DeviceObject->FunctionalDeviceObject,
			pFilterDescriptor_Midi,
			buffer_interfaceName,
			NULL,
			0,
			NULL,
			NULL,
			&pCard->m_pFilterFactories_Midi
		);
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

#pragma CODE_PAGED
void driver_c::Channels_UnregisterFromSystem_Midi
(
	IN card_c*		pCard
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	WCHAR					buffer_interfaceName[32];
	unsigned long			cardType;

//Define card type.
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

//Undefine filters... unregister all channel.
	//Create strings.
	swprintf( buffer_interfaceName, L"GLI%d(%d) Midi", cardType, pCard->m_SerialNumber + 1 );

	//Disable filter factory.
	KsFilterFactorySetDeviceClassesState( pCard->m_pFilterFactories_Midi, FALSE );
	Interface_Disable_Filter( pCard, buffer_interfaceName );

	//Release filter factories
	KsDeleteFilterFactory( pCard->m_pFilterFactories_Midi );
	pCard->m_pFilterFactories_Midi = NULL;

//Free descriptors.
	KSFILTER_DESCRIPTOR*	pFilterDescriptor = m_pFilterDescriptors_Midi[pCard->m_SerialNumber];
	if ( pFilterDescriptor != NULL )
	{
		ExFreePoolWithTag( pFilterDescriptor, MEMORY_TAG_DRIVER_FILTER_DESCRIPTOR );
		m_pFilterDescriptors_Midi[pCard->m_SerialNumber] = NULL;
	}

	KSPIN_DESCRIPTOR_EX*	pPinDescriptor = m_pPinDescriptors_Midi[pCard->m_SerialNumber];
	if ( pPinDescriptor != NULL )
	{
		ExFreePoolWithTag( pPinDescriptor, MEMORY_TAG_DRIVER_PIN_DESCRIPTOR );
		m_pPinDescriptors_Midi[pCard->m_SerialNumber] = NULL;
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( STATUS_SUCCESS );
#endif
	return;
}

#pragma CODE_PAGED
NTSTATUS driver_c::Channels_CopyDescriptors_Midi
(
	IN OUT KSFILTER_DESCRIPTOR**		ppFilterDescriptor,
	IN OUT KSPIN_DESCRIPTOR_EX**		ppPinDescriptors
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	NTSTATUS	Status = STATUS_SUCCESS;

//Validate...
	if (
	  ( ppFilterDescriptor == NULL )
	  ||
	  ( *ppFilterDescriptor == NULL )
	)
	{
		Status = STATUS_UNSUCCESSFUL;
	}

	if (
	  ( ppPinDescriptors == NULL )
	  ||
	  ( *ppPinDescriptors == NULL )
	)
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//Create a copy of descriptors.
	NTSTATUS	Status_filter = STATUS_SUCCESS;
	NTSTATUS	Status_pin = STATUS_SUCCESS;

	//Filter descriptor...
	if ( SUCCESS( Status ) )
	{
		KSFILTER_DESCRIPTOR*	pFilterDescriptor = (KSFILTER_DESCRIPTOR*)ExAllocatePoolWithTag(
		  NonPagedPool,
		  sizeof(FilterDescriptor_Midi),	//all FilterDescriptors have the same size
		  MEMORY_TAG_DRIVER_FILTER_DESCRIPTOR
		);
		if ( pFilterDescriptor != NULL )
		{
		//Copy data.
			RtlCopyMemory( pFilterDescriptor, *ppFilterDescriptor, sizeof(FilterDescriptor_Midi) );

		//Return pointer of a copy.
			*ppFilterDescriptor = pFilterDescriptor;
		}
		else
		{
			Status = Status_filter = STATUS_INSUFFICIENT_RESOURCES;
		}
	}

	//Pin descriptor...
	if ( SUCCESS( Status ) )
	{
		KSPIN_DESCRIPTOR_EX*	pPinDescriptor = (KSPIN_DESCRIPTOR_EX*)ExAllocatePoolWithTag(
		  NonPagedPool,
		  sizeof(KsPinDescriptors_Midi),	//all PinDescriptors have the same size
		  MEMORY_TAG_DRIVER_PIN_DESCRIPTOR
		);
		if ( pPinDescriptor != NULL )
		{
		//Copy data.
			RtlCopyMemory( pPinDescriptor, *ppPinDescriptors, sizeof(KsPinDescriptors_Midi) );

		//Return pointer of a copy.
			*ppPinDescriptors = pPinDescriptor;
		}
		else
		{
			Status = Status_pin = STATUS_INSUFFICIENT_RESOURCES;
		}
	}

//All ok?
	if ( FAILED( Status ) )
	{
		if ( SUCCESS( Status_filter ) )
		{
		//Free filter descriptor.
			ExFreePoolWithTag( *ppFilterDescriptor, MEMORY_TAG_DRIVER_FILTER_DESCRIPTOR );
		}
		//if ( SUCCESS( Status_pin ) )
		//{
		////Free pin descriptor.
		//	ExFreePoolWithTag( *ppPinDescriptors, MEMORY_TAG_DRIVER_PIN_DESCRIPTOR );
		//}

		*ppFilterDescriptor = NULL;
		*ppPinDescriptors = NULL;
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}
