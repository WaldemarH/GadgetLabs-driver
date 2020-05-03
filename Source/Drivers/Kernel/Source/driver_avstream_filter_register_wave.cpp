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
const KSPIN_DISPATCH PinDispatch_Wave =
{
	&driver_c::Pin_Create_Wave,										//Create
	&driver_c::Pin_Close_Wave,										//Close
	&driver_c::Pin_Process_Wave,									//Process
	NULL,															//Reset
	(PFNKSPINSETDATAFORMAT)&driver_c::Pin_SetDataFormat_Wave,		//SetDataFormat
	&driver_c::Pin_SetDeviceState_Wave,								//SetDeviceState
	NULL,															//Connect
	NULL,															//Disconnect
	NULL,															//Clock
	NULL															//Allocator
};

//÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷
//Wave Pin
const KSPROPERTY_ITEM PropertyItem_Wave_Audio [1] =
{
	DEFINE_KSPROPERTY_ITEM
	(
	  KSPROPERTY_AUDIO_POSITION,									//PropertyId
	  driver_c::PropertyItem_Wave_Audio_Position_Handler_Get,		//GetPropertyHandler
	  sizeof(KSPROPERTY),											//MinProperty
	  sizeof(KSAUDIO_POSITION),										//MinData
	  driver_c::PropertyItem_Wave_Audio_Position_Handler_Set,		//SetPropertyHandler
	  NULL,		//Values
	  0,		//RelationsCount
	  NULL,		//Relations
	  NULL,		//SupportHandler
	  0
	)
};
const KSPROPERTY_SET PropertyTable_Wave[1] =
{
	DEFINE_KSPROPERTY_SET
	(
	  &KSPROPSETID_Audio,
	  SIZEOF_ARRAY( PropertyItem_Wave_Audio ),
	  PropertyItem_Wave_Audio,
	  0,
	  NULL
	)
};
DEFINE_KSAUTOMATION_TABLE ( AutomationTable_Wave )
{
	DEFINE_KSAUTOMATION_PROPERTIES ( PropertyTable_Wave ),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS_NULL
};

const KSPIN_INTERFACE PinInterfaces_Wave[1] =
{
	{
		STATICGUIDOF( KSINTERFACESETID_Standard ),
		KSINTERFACE_STANDARD_STREAMING,
		0
	}
};

const KSPIN_MEDIUM PinMediums_Wave[1] =
{
	{
		STATICGUIDOF( KSMEDIUMSETID_Standard ),
		KSMEDIUM_TYPE_ANYINSTANCE,
		0
	}
};

const KSDATARANGE_AUDIO PinWaveIoRange_4xx_Wave_MME_Stereo =
{
	{
		sizeof( KSDATARANGE_AUDIO ),
		0,
		0,
		0,
		STATICGUIDOF( KSDATAFORMAT_TYPE_AUDIO ),
		STATICGUIDOF( KSDATAFORMAT_SUBTYPE_PCM ),
		STATICGUIDOF( KSDATAFORMAT_SPECIFIER_WAVEFORMATEX )
	},
	2,			// max number of channels
	16,			// min bits per sample (our sample container is 32 bit in size)
	32,			// max bits per sample (our sample container is 32 bit in size)
	44100,		// min frequency->44100
	96000		// max frequency->96000
};
const KSDATARANGE_AUDIO PinWaveIoRange_4xx_Wave_DX_Stereo =		//Define another to allow DX drivers too.
{
	{
		sizeof( KSDATARANGE_AUDIO ),
		0,
		0,
		0,
		STATICGUIDOF( KSDATAFORMAT_TYPE_AUDIO ),
		STATICGUIDOF( KSDATAFORMAT_SUBTYPE_PCM ),
		STATICGUIDOF( KSDATAFORMAT_SPECIFIER_DSOUND )
	},
	2,
	16,
	32,
	44100,
	96000
};
const PKSDATARANGE PinAudioFormatRanges_4xx_WaveSink_Stereo[2] =
{
	(PKSDATARANGE) &PinWaveIoRange_4xx_Wave_MME_Stereo,
	(PKSDATARANGE) &PinWaveIoRange_4xx_Wave_DX_Stereo
};

const KSDATARANGE_AUDIO PinWaveIoRange_4xx_Wave_MME_All =
{
	{
		sizeof( KSDATARANGE_AUDIO ),
		0,
		0,
		0,
		STATICGUIDOF( KSDATAFORMAT_TYPE_AUDIO ),
		STATICGUIDOF( KSDATAFORMAT_SUBTYPE_PCM ),
		STATICGUIDOF( KSDATAFORMAT_SPECIFIER_WAVEFORMATEX )
	},
	4,			// max number of channels
	16,			// min bits per sample (our sample container is 32 bit in size)
	32,			// max bits per sample (our sample container is 32 bit in size)
	44100,		// min frequency->44100
	96000		// max frequency->96000
};
const KSDATARANGE_AUDIO PinWaveIoRange_4xx_Wave_DX_All =		//Define another to allow DX drivers too.
{
	{
		sizeof( KSDATARANGE_AUDIO ),
		0,
		0,
		0,
		STATICGUIDOF( KSDATAFORMAT_TYPE_AUDIO ),
		STATICGUIDOF( KSDATAFORMAT_SUBTYPE_PCM ),
		STATICGUIDOF( KSDATAFORMAT_SPECIFIER_DSOUND )
	},
	4,
	16,
	32,
	44100,
	96000
};
const PKSDATARANGE PinAudioFormatRanges_4xx_WaveSink_All[2] =
{
	(PKSDATARANGE) &PinWaveIoRange_4xx_Wave_MME_All,
	(PKSDATARANGE) &PinWaveIoRange_4xx_Wave_DX_All
};

const KSDATARANGE_AUDIO PinWaveIoRange_824_Wave_MME_Stereo =
{
	{
		sizeof( KSDATARANGE_AUDIO ),						//FormatSize
		0,													//Flags
		0,													//SampleSize
		0,													//Reserved
		STATICGUIDOF( KSDATAFORMAT_TYPE_AUDIO ),			//MajorFormat
		STATICGUIDOF( KSDATAFORMAT_SUBTYPE_PCM ),			//SubFormat
		STATICGUIDOF( KSDATAFORMAT_SPECIFIER_WAVEFORMATEX )	//Specifier
	},
	2,			// max number of channels
	16,			// min bits per sample (our sample container is 32 bit in size)
	32,			// max bits per sample (our sample container is 32 bit in size)
	44100,		// min frequency->44100
	48000		// max frequency->48000
};
const KSDATARANGE_AUDIO PinWaveIoRange_824_Wave_DX_Stereo =			//Define another to allow DX drivers too.
{
	{
		sizeof( KSDATARANGE_AUDIO ),
		0,
		0,
		0,
		STATICGUIDOF( KSDATAFORMAT_TYPE_AUDIO ),
		STATICGUIDOF( KSDATAFORMAT_SUBTYPE_PCM ),
		STATICGUIDOF( KSDATAFORMAT_SPECIFIER_DSOUND )
	},
	2,
	16,
	32,
	44100,
	48000
};
const PKSDATARANGE PinAudioFormatRanges_824_WaveSink_Stereo[2] =
{
	(PKSDATARANGE) &PinWaveIoRange_824_Wave_MME_Stereo,
	(PKSDATARANGE) &PinWaveIoRange_824_Wave_DX_Stereo
};

const KSDATARANGE_AUDIO PinWaveIoRange_824_Wave_MME_All =
{
	{
		sizeof( KSDATARANGE_AUDIO ),						//FormatSize
		0,													//Flags
		0,													//SampleSize
		0,													//Reserved
		STATICGUIDOF( KSDATAFORMAT_TYPE_AUDIO ),			//MajorFormat
		STATICGUIDOF( KSDATAFORMAT_SUBTYPE_PCM ),			//SubFormat
		STATICGUIDOF( KSDATAFORMAT_SPECIFIER_WAVEFORMATEX )	//Specifier
	},
	8,			// max number of channels
	16,			// min bits per sample (our sample container is 32 bit in size)
	32,			// max bits per sample (our sample container is 32 bit in size)
	44100,		// min frequency->44100
	48000		// max frequency->48000
};
const KSDATARANGE_AUDIO PinWaveIoRange_824_Wave_DX_All =	//Define another to allow DX drivers too.
{
	{
		sizeof( KSDATARANGE_AUDIO ),
		0,
		0,
		0,
		STATICGUIDOF( KSDATAFORMAT_TYPE_AUDIO ),
		STATICGUIDOF( KSDATAFORMAT_SUBTYPE_PCM ),
		STATICGUIDOF( KSDATAFORMAT_SPECIFIER_DSOUND )
	},
	8,
	16,
	32,
	44100,
	48000
};
const PKSDATARANGE PinAudioFormatRanges_824_WaveSink_All[2] =
{
	(PKSDATARANGE) &PinWaveIoRange_824_Wave_MME_All,
	(PKSDATARANGE) &PinWaveIoRange_824_Wave_DX_All
};

GUID pinCategory_Wave = { STATIC_KSCATEGORY_AUDIO };
GUID pinName_Wave_line_in = { STATIC_KSNODETYPE_MICROPHONE };
GUID pinName_Wave_line_out = { STATIC_KSNODETYPE_SPEAKER };

//Here we can define the frame size(bufferSize).
//
//As the documents are really bad I went my way... it's working but I don't know if it's correct.
//
KSALLOCATOR_FRAMING_EX	driver_c::m_AllocatorFraming_4xx_Wave_All =
{
	1,
	0,
	{
		1,
		1,
		0
	},
	0,
	{
		{
			STATICGUIDOF( KSMEMORY_TYPE_KERNEL_NONPAGED ),	//MemoryType
			STATIC_KS_TYPE_DONT_CARE,						//BusType
			KSALLOCATOR_REQUIREMENTF_MUST_ALLOCATE,			//MemoryFlags
			0,												//BusFlags
			KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY,			//Flags
			1,												//Frames
			FILE_QUAD_ALIGNMENT,							//FileAlignment
			0,												//MemoryTypeWeight
			{												//PhysicalRange
				0,			//min: 4*glClass.bufferSize
				0,			//max: 4*glClass.bufferSize
				0			//step: 0
			},
			{												//FramingRange
				{
				//Defines optimal range of frame sizes.
				//Notice:
				//You will have to reset this every time the driver
				//buffer will change.
					0,		//MinFrameSize: 4*glClass.bufferSize
					0,		//MaxFrameSize: 4*glClass.bufferSize
					0		//Stepping: 0
				},
				0,
				0
			}
		}
	}
};
KSALLOCATOR_FRAMING_EX	driver_c::m_AllocatorFraming_824_Wave_All =
{
	1,
	0,
	{
		1,
		1,
		0
	},
	0,
	{
		{
			STATICGUIDOF( KSMEMORY_TYPE_KERNEL_NONPAGED ),	//MemoryType
			STATIC_KS_TYPE_DONT_CARE,						//BusType
			KSALLOCATOR_REQUIREMENTF_MUST_ALLOCATE,			//MemoryFlags
			0,												//BusFlags
			KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY,			//Flags
			1,												//Frames
			FILE_QUAD_ALIGNMENT,							//FileAlignment
			0,												//MemoryTypeWeight
			{												//PhysicalRange
				0,			//min: 8*glClass.bufferSize
				0,			//max: 8*glClass.bufferSize
				0			//step: 0
			},
			{												//FramingRange
				{
				//Defines optimal range of frame sizes.
				//Notice:
				//You will have to reset this every time the driver
				//buffer will change.
					0,		//MinFrameSize: 8*glClass.bufferSize
					0,		//MaxFrameSize: 8*glClass.bufferSize
					0		//Stepping: 0
				},
				0,
				0
			}
		}
	}
};
KSALLOCATOR_FRAMING_EX	driver_c::m_AllocatorFraming_Wave_Stereo = 
{
	1,
	0,
	{
		1,
		1,
		0
	},
	0,
	{
		{
			STATICGUIDOF( KSMEMORY_TYPE_KERNEL_NONPAGED ),	//MemoryType
			STATIC_KS_TYPE_DONT_CARE,						//BusType
			KSALLOCATOR_REQUIREMENTF_MUST_ALLOCATE,			//MemoryFlags
			0,												//BusFlags
			KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY,			//Flags
			1,												//Frames
			FILE_QUAD_ALIGNMENT,							//FileAlignment
			0,												//MemoryTypeWeight
			{												//PhysicalRange
				0,			//min: 2*glClass.bufferSize
				0,			//max: 2*glClass.bufferSize
				0			//step: 0
			},
			{												//FramingRange
				{
				//Defines optimal range of frame sizes.
				//Notice:
				//You will have to reset this every time the driver
				//buffer will change.
					0,		//MinFrameSize: 2*glClass.bufferSize
					0,		//MaxFrameSize: 2*glClass.bufferSize
					0		//Stepping: 0
				},
				0,
				0
			}
		}
	}
};

//÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷
//Bridge Pin
static KSDATARANGE PinWaveIoRange_WaveBridge[] = 
{
	{
		sizeof( KSDATARANGE ),
		0,
		0,
		0,
		STATICGUIDOF( KSDATAFORMAT_TYPE_AUDIO ),
		STATICGUIDOF( KSDATAFORMAT_SUBTYPE_ANALOG ),
		STATICGUIDOF( KSDATAFORMAT_SPECIFIER_NONE )
	}
};

const PKSDATARANGE PinAudioFormatRanges_WaveBridge[1] = 
{
	(PKSDATARANGE) &PinWaveIoRange_WaveBridge
};

//After Vista bridge category must be one of the:
//KSNODETYPE_SPEAKER
//KSNODETYPE_HEADPHONES
//KSNODETYPE_LINE_CONNECTOR
//KSNODETYPE_SPDIF_INTERFACE
//KSNODETYPE_MICROPHONE
//KSNODETYPE_HANDSET
//KSNODETYPE_HEADSET_SPEAKERS
//KSNODETYPE_HEADSET_MICROPHONE
//GUID pinCategory_bridge_wave = { STATIC_KSCATEGORY_AUDIO };
GUID pinCategory_bridge_wave = { STATIC_KSNODETYPE_LINE_CONNECTOR };
GUID pinName_bridge_line_in = { STATIC_KSNODETYPE_LINE_CONNECTOR };		//{ STATIC_KSNODETYPE_MICROPHONE };
GUID pinName_bridge_line_out = { STATIC_KSNODETYPE_LINE_CONNECTOR };	//{ STATIC_KSNODETYPE_SPEAKER };

//÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷
//Pins descriptors:
//
//Every stereo IN/OUT is one filter.
//If not done this way the system can't see all the channels
//(only the first stereo IN/OUT).
//
//Notice:
//If Multichannel Pin Factory is opened then all stereo channels will be disabled -> driver wont allow them to open.
//
//Card structure:
//e.g. 4xx
//                 -------------------------------
// WaveIn1,2 ----->|Pin       |       |Pin       |-> WaveOut1,2
//                 |Factory 0 |       |Factory 0 |
//                 |----------|       |----------|
// WaveIn3,4 ----->|Pin       |       |Pin       |-> WaveOut3,4
//                 |Factory 1 |   C   |Factory 1 |
//                 |----------|   A   |----------|
// WaveIn1,2,3,4 ->|Pin       |   R   |Pin       |-> WaveOut1,2,3,4
//                 |Factory 2 |   D   |Factory 2 |
//                 |----------|       |----------|
// MidiIn -------->|Pin       |       |Pin       |-> MidiOut
//                 |Factory 3 |       |Factory 3 |
//                 -------------------------------
//
//
//Detailed filter description (all card types):
//
//                   --------------------------
//                   | "Stereo Filter"        |
//                   |  (1) ------------ (0)  |
//  Bridge Pin  1 o--|-->o--|ADC Node 0|--o>--|--o 0  Sink Pin
//                   |      ------------      |      (WaveIn 1,2)
//                   |                        |
//                   |  (1) ------------ (0)  |
//    Sink Pin  2 o--|-->o--|DAC Node 1|--o>--|--o 3  Bridge Pin
//   (WaveOut 1,2)   |      ------------      |
//                   |                        |
//                   --------------------------
//
//                   --------------------------
//                   | "All Filter"           |
//                   |  (1) ------------ (0)  |
//  Bridge Pin  1 o--|-->o--|ADC Node 0|--o>--|--o 0  Sink Pin
//                   |      ------------      |      (WaveIn All)
//                   |                        |
//                   |  (1) ------------ (0)  |
//    Sink Pin  2 o--|-->o--|DAC Node 1|--o>--|--o 3  Bridge Pin
//   (WaveOut all)   |      ------------      |
//                   |                        |
//                   --------------------------
//
//As you see they are both the same, but structures they use are different.
//
//Notice1:
//Every definition of IN/OUT looks from card point of view and not the driver( filter ).
//e.g. WaveIn1 - input channel-1 into card
//
KSPIN_DESCRIPTOR_EX KsPinDescriptors_4xx_Wave_All[] =
{
//WaveIn 1,2
	//Sink Pin
	{
		&PinDispatch_Wave,
		NULL,	//&AutomationTable_Wave,
		{
			SIZEOF_ARRAY( PinInterfaces_Wave ),
			(KSPIN_INTERFACE*) &PinInterfaces_Wave,
			SIZEOF_ARRAY( PinMediums_Wave ),
			(KSPIN_MEDIUM*) &PinMediums_Wave,
			SIZEOF_ARRAY( PinAudioFormatRanges_4xx_WaveSink_All ),
			PinAudioFormatRanges_4xx_WaveSink_All,
			KSPIN_DATAFLOW_OUT,
			KSPIN_COMMUNICATION_SINK,
			&pinCategory_Wave,
			&pinName_Wave_line_in,
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
		&driver_c::m_AllocatorFraming_4xx_Wave_All,
		(PFNKSINTERSECTHANDLEREX)&driver_c::Pin_AudioIntersectHandler
	},
	//Bridge PIN
	{
		NULL,
		NULL,
		{
			SIZEOF_ARRAY( PinInterfaces_Wave ),
			(KSPIN_INTERFACE*) &PinInterfaces_Wave,
			SIZEOF_ARRAY( PinMediums_Wave ),
			(KSPIN_MEDIUM*) &PinMediums_Wave,
			SIZEOF_ARRAY( PinAudioFormatRanges_WaveBridge ),
			PinAudioFormatRanges_WaveBridge,
			KSPIN_DATAFLOW_IN,
			KSPIN_COMMUNICATION_NONE,
			&pinCategory_bridge_wave,
			&pinName_bridge_line_in,
			0
		},
		0,
		0,
		0,
		NULL,
		NULL
	},
//WaveOut 1,2
	//Sink PIN
	{
		&PinDispatch_Wave,
		&AutomationTable_Wave,
		{
			SIZEOF_ARRAY( PinInterfaces_Wave ),
			(KSPIN_INTERFACE*) &PinInterfaces_Wave,
			SIZEOF_ARRAY( PinMediums_Wave ),
			(KSPIN_MEDIUM*) &PinMediums_Wave,
			SIZEOF_ARRAY( PinAudioFormatRanges_4xx_WaveSink_All ),
			PinAudioFormatRanges_4xx_WaveSink_All,
			KSPIN_DATAFLOW_IN,
			KSPIN_COMMUNICATION_SINK,
			&pinCategory_Wave,
			&pinName_Wave_line_out,
			0
		},
		//We need this if we want to call KsPinAttemptProcessing
		//synhronically( do KsPinProcessing come back to DPC )
		KSPIN_FLAG_DISPATCH_LEVEL_PROCESSING |
		  //Directly poll the queue at DPC time( transport data to hardware ).
		  KSPIN_FLAG_DO_NOT_INITIATE_PROCESSING |
		  //Pin is responsible to check for available frames.
		  KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING |
		  KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY |
		  //Change status one step at a time.
		  KSPIN_FLAG_USE_STANDARD_TRANSPORT |
		  KSPIN_FLAG_RENDERER,
		1,	//InstancesPossible
		0,	//InstancesNecessary
		&driver_c::m_AllocatorFraming_4xx_Wave_All,
		(PFNKSINTERSECTHANDLEREX)&driver_c::Pin_AudioIntersectHandler
	},
	//Bridge PIN
	{
		NULL,
		NULL,
		{
			SIZEOF_ARRAY( PinInterfaces_Wave ),
			(KSPIN_INTERFACE*) &PinInterfaces_Wave,
			SIZEOF_ARRAY( PinMediums_Wave ),
			(KSPIN_MEDIUM*) &PinMediums_Wave,
			SIZEOF_ARRAY( PinAudioFormatRanges_WaveBridge ),
			PinAudioFormatRanges_WaveBridge,
			KSPIN_DATAFLOW_OUT,
			KSPIN_COMMUNICATION_NONE,
			&pinCategory_bridge_wave,
			&pinName_bridge_line_out,
			0
		},
		0,
		0,
		0,
		NULL,
		NULL
	}
};
KSPIN_DESCRIPTOR_EX KsPinDescriptors_4xx_Wave_Stereo[] =
{
//WaveIn 1,2
	//Sink Pin
	{
		&PinDispatch_Wave,
		NULL,	//&AutomationTable_Wave,
		{
			SIZEOF_ARRAY( PinInterfaces_Wave ),
			(KSPIN_INTERFACE*) &PinInterfaces_Wave,
			SIZEOF_ARRAY( PinMediums_Wave ),
			(KSPIN_MEDIUM*) &PinMediums_Wave,
			SIZEOF_ARRAY( PinAudioFormatRanges_4xx_WaveSink_Stereo ),
			PinAudioFormatRanges_4xx_WaveSink_Stereo,
			KSPIN_DATAFLOW_OUT,
			KSPIN_COMMUNICATION_SINK,
			&pinCategory_Wave,
			&pinName_Wave_line_in,
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
		&driver_c::m_AllocatorFraming_Wave_Stereo,
		(PFNKSINTERSECTHANDLEREX)&driver_c::Pin_AudioIntersectHandler
	},
	//Bridge PIN
	{
		NULL,
		NULL,
		{
			SIZEOF_ARRAY( PinInterfaces_Wave ),
			(KSPIN_INTERFACE*) &PinInterfaces_Wave,
			SIZEOF_ARRAY( PinMediums_Wave ),
			(KSPIN_MEDIUM*) &PinMediums_Wave,
			SIZEOF_ARRAY( PinAudioFormatRanges_WaveBridge ),
			PinAudioFormatRanges_WaveBridge,
			KSPIN_DATAFLOW_IN,
			KSPIN_COMMUNICATION_NONE,
			&pinCategory_bridge_wave,
			&pinName_bridge_line_in,
			0
		},
		0,
		0,
		0,
		NULL,
		NULL
	},
//WaveOut 1,2
	//Sink PIN
	{
		&PinDispatch_Wave,
		&AutomationTable_Wave,
		{
			SIZEOF_ARRAY( PinInterfaces_Wave ),
			(KSPIN_INTERFACE*) &PinInterfaces_Wave,
			SIZEOF_ARRAY( PinMediums_Wave ),
			(KSPIN_MEDIUM*) &PinMediums_Wave,
			SIZEOF_ARRAY( PinAudioFormatRanges_4xx_WaveSink_Stereo ),
			PinAudioFormatRanges_4xx_WaveSink_Stereo,
			KSPIN_DATAFLOW_IN,
			KSPIN_COMMUNICATION_SINK,
			&pinCategory_Wave,
			&pinName_Wave_line_out,
			0
		},
		//We need this if we want to call KsPinAttemptProcessing
		//synhronically( do KsPinProcessing come back to DPC )
		KSPIN_FLAG_DISPATCH_LEVEL_PROCESSING |
		  //Directly poll the queue at DPC time( transport data to hardware ).
		  KSPIN_FLAG_DO_NOT_INITIATE_PROCESSING |
		  //Pin is responsible to check for available frames.
		  KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING |
		  KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY |
		  //Change status one step at a time.
		  KSPIN_FLAG_USE_STANDARD_TRANSPORT |
		  KSPIN_FLAG_RENDERER,
		1,	//InstancesPossible
		0,	//InstancesNecessary
		&driver_c::m_AllocatorFraming_Wave_Stereo,
		(PFNKSINTERSECTHANDLEREX)&driver_c::Pin_AudioIntersectHandler
	},
	//Bridge PIN
	{
		NULL,
		NULL,
		{
			SIZEOF_ARRAY( PinInterfaces_Wave ),
			(KSPIN_INTERFACE*) &PinInterfaces_Wave,
			SIZEOF_ARRAY( PinMediums_Wave ),
			(KSPIN_MEDIUM*) &PinMediums_Wave,
			SIZEOF_ARRAY( PinAudioFormatRanges_WaveBridge ),
			PinAudioFormatRanges_WaveBridge,
			KSPIN_DATAFLOW_OUT,
			KSPIN_COMMUNICATION_NONE,
			&pinCategory_bridge_wave,
			&pinName_bridge_line_out,
			0
		},
		0,
		0,
		0,
		NULL,
		NULL
	}
};
KSPIN_DESCRIPTOR_EX KsPinDescriptors_824_Wave_All[] = 
{
//WaveIn 1,2
	//Sink Pin
	{
		&PinDispatch_Wave,
		NULL,	//&AutomationTable_Wave,
		{
			SIZEOF_ARRAY( PinInterfaces_Wave ),
			(KSPIN_INTERFACE*) &PinInterfaces_Wave,
			SIZEOF_ARRAY( PinMediums_Wave ),
			(KSPIN_MEDIUM*) &PinMediums_Wave,
			SIZEOF_ARRAY( PinAudioFormatRanges_824_WaveSink_All ),
			PinAudioFormatRanges_824_WaveSink_All,
			KSPIN_DATAFLOW_OUT,
			KSPIN_COMMUNICATION_SINK,
			&pinCategory_Wave,
			&pinName_Wave_line_in,
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
		&driver_c::m_AllocatorFraming_824_Wave_All,
		(PFNKSINTERSECTHANDLEREX)&driver_c::Pin_AudioIntersectHandler
	},
	//Bridge PIN
	{
		NULL,
		NULL,
		{
			SIZEOF_ARRAY( PinInterfaces_Wave ),
			(KSPIN_INTERFACE*) &PinInterfaces_Wave,
			SIZEOF_ARRAY( PinMediums_Wave ),
			(KSPIN_MEDIUM*) &PinMediums_Wave,
			SIZEOF_ARRAY( PinAudioFormatRanges_WaveBridge ),
			PinAudioFormatRanges_WaveBridge,
			KSPIN_DATAFLOW_IN,
			KSPIN_COMMUNICATION_NONE,
			&pinCategory_bridge_wave,
			&pinName_bridge_line_in,
			0
		},
		0,
		0,
		0,
		NULL,
		NULL
	},
//WaveOut 1,2
	//Sink PIN
	{
		&PinDispatch_Wave,
		&AutomationTable_Wave,
		{
			SIZEOF_ARRAY( PinInterfaces_Wave ),
			(KSPIN_INTERFACE*) &PinInterfaces_Wave,
			SIZEOF_ARRAY( PinMediums_Wave ),
			(KSPIN_MEDIUM*) &PinMediums_Wave,
			SIZEOF_ARRAY( PinAudioFormatRanges_824_WaveSink_All ),
			PinAudioFormatRanges_824_WaveSink_All,
			KSPIN_DATAFLOW_IN,
			KSPIN_COMMUNICATION_SINK,
			&pinCategory_Wave,
			&pinName_Wave_line_out,
			0
		},
		//We need this if we want to call KsPinAttemptProcessing
		//synhronically( do KsPinProcessing come back to DPC )
		KSPIN_FLAG_DISPATCH_LEVEL_PROCESSING |
		  //Directly poll the queue at DPC time( transport data to hardware ).
		  KSPIN_FLAG_DO_NOT_INITIATE_PROCESSING |
		  //Pin is responsible to check for available frames.
		  KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING |
		  KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY |
		  //Change status one step at a time.
		  KSPIN_FLAG_USE_STANDARD_TRANSPORT |
		  KSPIN_FLAG_RENDERER,
		1,	//InstancesPossible
		0,	//InstancesNecessary
		&driver_c::m_AllocatorFraming_824_Wave_All,
		(PFNKSINTERSECTHANDLEREX)&driver_c::Pin_AudioIntersectHandler
	},
	//Bridge PIN
	{
		NULL,
		NULL,
		{
			SIZEOF_ARRAY( PinInterfaces_Wave ),
			(KSPIN_INTERFACE*) &PinInterfaces_Wave,
			SIZEOF_ARRAY( PinMediums_Wave ),
			(KSPIN_MEDIUM*) &PinMediums_Wave,
			SIZEOF_ARRAY( PinAudioFormatRanges_WaveBridge ),
			PinAudioFormatRanges_WaveBridge,
			KSPIN_DATAFLOW_OUT,
			KSPIN_COMMUNICATION_NONE,
			&pinCategory_bridge_wave,
			&pinName_bridge_line_out,
			0
		},
		0,
		0,
		0,
		NULL,
		NULL
	}
};
KSPIN_DESCRIPTOR_EX KsPinDescriptors_824_Wave_Stereo[] =
{
//WaveIn 1,2
	//Sink Pin
	{
		&PinDispatch_Wave,
		NULL,	//&AutomationTable_Wave,
		{
			SIZEOF_ARRAY( PinInterfaces_Wave ),
			(KSPIN_INTERFACE*) &PinInterfaces_Wave,
			SIZEOF_ARRAY( PinMediums_Wave ),
			(KSPIN_MEDIUM*) &PinMediums_Wave,
			SIZEOF_ARRAY( PinAudioFormatRanges_824_WaveSink_Stereo ),
			PinAudioFormatRanges_824_WaveSink_Stereo,
			KSPIN_DATAFLOW_OUT,
			KSPIN_COMMUNICATION_SINK,
			&pinCategory_Wave,
			&pinName_Wave_line_in,
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
		&driver_c::m_AllocatorFraming_Wave_Stereo,
		(PFNKSINTERSECTHANDLEREX)&driver_c::Pin_AudioIntersectHandler
	},
	//Bridge PIN
	{
		NULL,
		NULL,
		{
			SIZEOF_ARRAY( PinInterfaces_Wave ),
			(KSPIN_INTERFACE*) &PinInterfaces_Wave,
			SIZEOF_ARRAY( PinMediums_Wave ),
			(KSPIN_MEDIUM*) &PinMediums_Wave,
			SIZEOF_ARRAY( PinAudioFormatRanges_WaveBridge ),
			PinAudioFormatRanges_WaveBridge,
			KSPIN_DATAFLOW_IN,
			KSPIN_COMMUNICATION_NONE,
			&pinCategory_bridge_wave,
			&pinName_bridge_line_in,
			0
		},
		0,
		0,
		0,
		NULL,
		NULL
	},
//WaveOut 1,2
	//Sink PIN
	{
		&PinDispatch_Wave,
		&AutomationTable_Wave,
		{
			SIZEOF_ARRAY( PinInterfaces_Wave ),
			(KSPIN_INTERFACE*) &PinInterfaces_Wave,
			SIZEOF_ARRAY( PinMediums_Wave ),
			(KSPIN_MEDIUM*) &PinMediums_Wave,
			SIZEOF_ARRAY( PinAudioFormatRanges_824_WaveSink_Stereo ),
			PinAudioFormatRanges_824_WaveSink_Stereo,
			KSPIN_DATAFLOW_IN,
			KSPIN_COMMUNICATION_SINK,
			&pinCategory_Wave,
			&pinName_Wave_line_out,
			0
		},
		//We need this if we want to call KsPinAttemptProcessing
		//synhronically( do KsPinProcessing come back to DPC )
		KSPIN_FLAG_DISPATCH_LEVEL_PROCESSING |
		  //Directly poll the queue at DPC time( transport data to hardware ).
		  KSPIN_FLAG_DO_NOT_INITIATE_PROCESSING |
		  //Pin is responsible to check for available frames.
		  KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING |
		  KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY |
		  //Change status one step at a time.
		  KSPIN_FLAG_USE_STANDARD_TRANSPORT |
		  KSPIN_FLAG_RENDERER,
		1,	//InstancesPossible
		0,	//InstancesNecessary
		&driver_c::m_AllocatorFraming_Wave_Stereo,
		(PFNKSINTERSECTHANDLEREX)&driver_c::Pin_AudioIntersectHandler
	},
	//Bridge PIN
	{
		NULL,
		NULL,
		{
			SIZEOF_ARRAY( PinInterfaces_Wave ),
			(KSPIN_INTERFACE*) &PinInterfaces_Wave,
			SIZEOF_ARRAY( PinMediums_Wave ),
			(KSPIN_MEDIUM*) &PinMediums_Wave,
			SIZEOF_ARRAY( PinAudioFormatRanges_WaveBridge ),
			PinAudioFormatRanges_WaveBridge,
			KSPIN_DATAFLOW_OUT,
			KSPIN_COMMUNICATION_NONE,
			&pinCategory_bridge_wave,
			&pinName_bridge_line_out,
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
const GUID KsCategories_Wave[] = 
{
	STATICGUIDOF( KSCATEGORY_AUDIO ),
	STATICGUIDOF( KSCATEGORY_CAPTURE ),
	STATICGUIDOF( KSCATEGORY_RENDER )
};

//Driver supports ADC and DAC node types.
//This means that card can only retrieve or send audio data
//without any capabilities to change them.
const GUID ADCNodeType = {STATICGUIDOF( KSNODETYPE_ADC )};
const GUID DACNodeType = {STATICGUIDOF( KSNODETYPE_DAC )};

const KSNODE_DESCRIPTOR KsNodeDescriptors_Wave[] = 
{
	DEFINE_NODE_DESCRIPTOR( NULL, &ADCNodeType, &ADCNodeType ),
	DEFINE_NODE_DESCRIPTOR( NULL, &DACNodeType, &DACNodeType )
};

//Stereo filter description (all card types):
//
//                   --------------------------
//                   | "Filter"               |
//                   |  (1) ------------ (0)  |
//  Bridge Pin  1 o--|-->o--|ADC Node 0|--o>--|--o 0  Sink Pin
//                   |      ------------      |      (WaveIn 1,2)
//                   |                        |
//                   |  (1) ------------ (0)  |
//    Sink Pin  2 o--|-->o--|DAC Node 1|--o>--|--o 3  Bridge Pin
//   (WaveOut 1,2)   |      ------------      |
//                   |                        |
//                   --------------------------
//
const KSTOPOLOGY_CONNECTION KsConnections_Wave[]  = 
{//        FromNode,          FromPin,        ToNode,             ToPin
//WaveIn 1,2
	{ KSFILTER_NODE, WAVEIN_12_BRIDGE,   ADC_NODE_12,    NODE_PIN_INPUT },
	{   ADC_NODE_12,  NODE_PIN_OUTPUT, KSFILTER_NODE,         WAVEIN_12 },
//WaveOut 1,2
	{ KSFILTER_NODE,      WAVEOUT_12,   DAC_NODE_12,    NODE_PIN_INPUT },
	{   DAC_NODE_12, NODE_PIN_OUTPUT, KSFILTER_NODE, WAVEOUT_12_BRIDGE }
};


//÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷
//Filter factory:
const KSFILTER_DESCRIPTOR FilterDescriptor_4xx_Wave_All = 
{
	NULL,							//Filter Dispatch Table
	NULL,							//Automation Table
	KSFILTER_DESCRIPTOR_VERSION,	//Version
	0,								//Flags
	&KSNAME_Filter,					//Reference GUID
	DEFINE_KSFILTER_PIN_DESCRIPTORS ( KsPinDescriptors_4xx_Wave_All ),
	DEFINE_KSFILTER_CATEGORIES ( KsCategories_Wave ),
	DEFINE_KSFILTER_NODE_DESCRIPTORS( KsNodeDescriptors_Wave ),
	DEFINE_KSFILTER_CONNECTIONS( KsConnections_Wave ),
	NULL
};
const KSFILTER_DESCRIPTOR FilterDescriptor_4xx_Wave_Stereo = 
{
	NULL,							//Filter Dispatch Table
	NULL,							//Automation Table
	KSFILTER_DESCRIPTOR_VERSION,	//Version
	0,								//Flags
	&KSNAME_Filter,					//Reference GUID
	DEFINE_KSFILTER_PIN_DESCRIPTORS ( KsPinDescriptors_4xx_Wave_Stereo ),
	DEFINE_KSFILTER_CATEGORIES ( KsCategories_Wave ),
	DEFINE_KSFILTER_NODE_DESCRIPTORS( KsNodeDescriptors_Wave ),
	DEFINE_KSFILTER_CONNECTIONS( KsConnections_Wave ),
	NULL
};
const KSFILTER_DESCRIPTOR FilterDescriptor_824_Wave_All = 
{
	NULL,							//Filter Dispatch Table
	NULL,							//Automation Table
	KSFILTER_DESCRIPTOR_VERSION,	//Version
	0,								//Flags
	&KSNAME_Filter,					//Reference GUID
	DEFINE_KSFILTER_PIN_DESCRIPTORS ( KsPinDescriptors_824_Wave_All ),
	DEFINE_KSFILTER_CATEGORIES ( KsCategories_Wave ),
	DEFINE_KSFILTER_NODE_DESCRIPTORS( KsNodeDescriptors_Wave ),
	DEFINE_KSFILTER_CONNECTIONS( KsConnections_Wave ),
	NULL
};
const KSFILTER_DESCRIPTOR FilterDescriptor_824_Wave_Stereo = 
{
	NULL,							//Filter Dispatch Table
	NULL,							//Automation Table
	KSFILTER_DESCRIPTOR_VERSION,	//Version
	0,								//Flags
	&KSNAME_Filter,					//Reference GUID
	DEFINE_KSFILTER_PIN_DESCRIPTORS ( KsPinDescriptors_824_Wave_Stereo ),
	DEFINE_KSFILTER_CATEGORIES ( KsCategories_Wave ),
	DEFINE_KSFILTER_NODE_DESCRIPTORS( KsNodeDescriptors_Wave ),
	DEFINE_KSFILTER_CONNECTIONS( KsConnections_Wave ),
	NULL
};

//***************************************************************************
// ProcedureName:
//		Channels_RegisterToSystem_Wave/
//		Channels_UnregisterFromSystem_Wave
//
// Explanation:
//		Procedure will register/unregister channel-filter interfaces
//		and set registry values.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return:
//		STATUS_SUCCESS or the error code that was returned
//		from the attempt to perform the operation.
//
#pragma CODE_PAGED
NTSTATUS driver_c::Channels_RegisterToSystem_Wave
(
	card_c*		pCard
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	NTSTATUS				Status = STATUS_SUCCESS;

	WCHAR					buffer_friendlyName[64];
	WCHAR					buffer_interfaceName[32];
	unsigned long			cardType;
	unsigned long			numberOfStereoChannels;
	KSFILTER_DESCRIPTOR*	pFilterDescriptor_All;
	KSFILTER_DESCRIPTOR*	pFilterDescriptor_Stereo;
	const GUID*				pGUID;
	KSPIN_DESCRIPTOR_EX*	pPinDescriptors_All;
	KSPIN_DESCRIPTOR_EX*	pPinDescriptors_Stereo;

//Define card type.
	switch ( pCard->m_Type )
	{
	case card_c::CARD_TYPE_424:
		cardType = 424;
		numberOfStereoChannels = 2;

		pFilterDescriptor_All = (KSFILTER_DESCRIPTOR*) &FilterDescriptor_4xx_Wave_All;
		pFilterDescriptor_Stereo = (KSFILTER_DESCRIPTOR*) &FilterDescriptor_4xx_Wave_Stereo;

		pPinDescriptors_All = (KSPIN_DESCRIPTOR_EX*) &KsPinDescriptors_4xx_Wave_All;
		pPinDescriptors_Stereo = (KSPIN_DESCRIPTOR_EX*) &KsPinDescriptors_4xx_Wave_Stereo;
		break;
	case card_c::CARD_TYPE_496:
		cardType = 496;
		numberOfStereoChannels = 2;

		pFilterDescriptor_All = (KSFILTER_DESCRIPTOR*) &FilterDescriptor_4xx_Wave_All;
		pFilterDescriptor_Stereo = (KSFILTER_DESCRIPTOR*) &FilterDescriptor_4xx_Wave_Stereo;

		pPinDescriptors_All = (KSPIN_DESCRIPTOR_EX*) &KsPinDescriptors_4xx_Wave_All;
		pPinDescriptors_Stereo = (KSPIN_DESCRIPTOR_EX*) &KsPinDescriptors_4xx_Wave_Stereo;
		break;
	case card_c::CARD_TYPE_824:
		cardType = 824;
		numberOfStereoChannels = 4;

		pFilterDescriptor_All = (KSFILTER_DESCRIPTOR*) &FilterDescriptor_824_Wave_All;
		pFilterDescriptor_Stereo = (KSFILTER_DESCRIPTOR*) &FilterDescriptor_824_Wave_Stereo;

		pPinDescriptors_All = (KSPIN_DESCRIPTOR_EX*) &KsPinDescriptors_824_Wave_All;
		pPinDescriptors_Stereo = (KSPIN_DESCRIPTOR_EX*) &KsPinDescriptors_824_Wave_Stereo;
		break;
	};

//Define filters... register stereo channels.
	for ( unsigned long i = 0; i < numberOfStereoChannels; i++ )
	{
	//Convert stereo to mono (channel numbers) and convert to 1 based number.
		unsigned long	j = i*2 + 1;

	//Create strings.
		swprintf( buffer_friendlyName, L"WavePro%d(%d) ch. %d-%d", cardType, pCard->m_SerialNumber+1, j, j+1 );

		swprintf( buffer_interfaceName, L"GLI%d(%d) %d-%d", cardType, pCard->m_SerialNumber+1, j, j+1 );

	//Create copy of structures.
		Status = Channels_CopyDescriptors_Wave( &pFilterDescriptor_Stereo, &pPinDescriptors_Stereo );
		if ( FAILED( Status ) )
		{
			break;
		}

		//Backup for 'free resources'.
		m_pFilterDescriptors_Wave[pCard->m_SerialNumber][i] = pFilterDescriptor_Stereo;
		m_pPinDescriptors_Wave[pCard->m_SerialNumber][i] = pPinDescriptors_Stereo;

	//Set copied pin as part of filter.
		pFilterDescriptor_Stereo->PinDescriptors = pPinDescriptors_Stereo;

	//Set pins name
		pGUID = NULL;
		Status = Interface_SetPinNames( buffer_friendlyName, &pGUID );
		if ( FAILED( Status ) )
		{
			break;
		}
		//Replace dummy GUID with drivers one.
		pPinDescriptors_Stereo[WAVEIN_12_BRIDGE].PinDescriptor.Name = pGUID;
		pPinDescriptors_Stereo[WAVEOUT_12_BRIDGE].PinDescriptor.Name = pGUID;

	//Register filter interface.
		Status = Interface_Register_Filter( pCard, buffer_interfaceName, buffer_friendlyName );
		if ( FAILED( Status ) )
		{
			break;
		}

	//Register filter to system.
		Status = KsCreateFilterFactory(
		  pCard->m_DeviceObject->FunctionalDeviceObject,
		  pFilterDescriptor_Stereo,
		  buffer_interfaceName,
		  NULL,
		  0,
		  NULL,
		  NULL,
		  &pCard->m_pFilterFactories_Stereo[i]
		);
		if ( FAILED( Status ) )
		{
			break;
		}

		Status = KsFilterFactorySetDeviceClassesState( pCard->m_pFilterFactories_Stereo[i], TRUE );
		if ( FAILED( Status ) )
		{
			break;
		}
	}

//Define filters... register all channel.
	if ( SUCCESS( Status ) )
	{
	//Create strings.
		swprintf( buffer_friendlyName, L"WavePro%d(%d) ch. All", cardType, pCard->m_SerialNumber+1 );

		swprintf( buffer_interfaceName, L"GLI%d(%d) All", cardType, pCard->m_SerialNumber+1 );

	//Create copy of structures.
		Status = Channels_CopyDescriptors_Wave( &pFilterDescriptor_All, &pPinDescriptors_All );
		if ( SUCCESS( Status ) )
		{
		//Backup for 'free resources'.
			m_pFilterDescriptors_Wave[pCard->m_SerialNumber][numberOfStereoChannels] = pFilterDescriptor_All;
			m_pPinDescriptors_Wave[pCard->m_SerialNumber][numberOfStereoChannels] = pPinDescriptors_All;
		}

	//Set copied pin as part of filter.
		if ( SUCCESS( Status ) )
		{
			pFilterDescriptor_All->PinDescriptors = pPinDescriptors_All;
		}

	//Set pins names.
		if ( SUCCESS( Status ) )
		{
			pGUID = NULL;
			Status = Interface_SetPinNames( buffer_friendlyName, &pGUID );
			if ( SUCCESS( Status ) )
			{
			//Replace dummy GUID with drivers one.
				pPinDescriptors_All[WAVEIN_12_BRIDGE].PinDescriptor.Name = pGUID;
				pPinDescriptors_All[WAVEOUT_12_BRIDGE].PinDescriptor.Name = pGUID;
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
			  pFilterDescriptor_All,
			  buffer_interfaceName,
			  NULL,
			  0,
			  NULL,
			  NULL,
			  &pCard->m_pFilterFactories_All
			);
		}
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

#pragma CODE_PAGED
void driver_c::Channels_UnregisterFromSystem_Wave
(
	card_c*		pCard
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	WCHAR					buffer_interfaceName[32];
	unsigned long			cardType;
	unsigned long			numberOfStereoChannels;

//Define card type.
	switch ( pCard->m_Type )
	{
	case card_c::CARD_TYPE_424:
		cardType = 424;
		numberOfStereoChannels = 2;
		break;
	case card_c::CARD_TYPE_496:
		cardType = 496;
		numberOfStereoChannels = 2;
		break;
	case card_c::CARD_TYPE_824:
		cardType = 824;
		numberOfStereoChannels = 4;
		break;
	};

//Undefine filters... unregister all channel.
	//Create strings.
	swprintf( buffer_interfaceName, L"GLI%d(%d) All", cardType, pCard->m_SerialNumber + 1 );

	//Disable filter factory.
	KsFilterFactorySetDeviceClassesState( pCard->m_pFilterFactories_All, FALSE );
	Interface_Disable_Filter( pCard, buffer_interfaceName );

	//Release filter factories
	KsDeleteFilterFactory( pCard->m_pFilterFactories_All );
	pCard->m_pFilterFactories_All = NULL;

//Undefine filters... unregister stereo channels.
	for ( unsigned long i = 0; i < numberOfStereoChannels; i++ )
	{
		if ( pCard->m_pFilterFactories_Stereo[i] == NULL )
		{
		//There are no more pFilterFactories after 1st NULL.
			break;
		}

	//Convert stereo to mono (channel numbers) and convert to 1 based number.
		unsigned long		j = i*2 + 1;

	//Create strings.
		swprintf(
		  buffer_interfaceName,
		  L"GLI%d(%d) %d-%d",
		  cardType,
		  pCard->m_SerialNumber + 1,
		  j,
		  j+1
		);

	//Disable FilterFactory.
		KsFilterFactorySetDeviceClassesState( pCard->m_pFilterFactories_Stereo[i], FALSE );
		Interface_Disable_Filter( pCard, buffer_interfaceName );

	//Release filter factories
		KsDeleteFilterFactory( pCard->m_pFilterFactories_Stereo[i] );
		pCard->m_pFilterFactories_Stereo[i] = NULL;
	}

//Free descriptors.
	for ( int i = 0; i < ELEMENTS_IN_ARRAY( m_pFilterDescriptors_Wave[pCard->m_SerialNumber] ); i++ )
	{
		KSFILTER_DESCRIPTOR*	pFilterDescriptor = m_pFilterDescriptors_Wave[pCard->m_SerialNumber][i];
		if ( pFilterDescriptor != NULL )
		{
			ExFreePoolWithTag( pFilterDescriptor, MEMORY_TAG_DRIVER_FILTER_DESCRIPTOR );
			m_pFilterDescriptors_Wave[pCard->m_SerialNumber][i] = NULL;
		}
	}

	for ( int i = 0; i < ELEMENTS_IN_ARRAY( m_pPinDescriptors_Wave[pCard->m_SerialNumber] ); i++ )
	{
		KSPIN_DESCRIPTOR_EX*	pPinDescriptor = m_pPinDescriptors_Wave[pCard->m_SerialNumber][i];
		if ( pPinDescriptor != NULL )
		{
			ExFreePoolWithTag( pPinDescriptor, MEMORY_TAG_DRIVER_PIN_DESCRIPTOR );
			m_pPinDescriptors_Wave[pCard->m_SerialNumber][i] = NULL;
		}
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( STATUS_SUCCESS );
#endif
	return;
}

#pragma CODE_PAGED
NTSTATUS driver_c::Channels_CopyDescriptors_Wave
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
		  sizeof(FilterDescriptor_4xx_Wave_All),	//all FilterDescriptors have the same size
		  MEMORY_TAG_DRIVER_FILTER_DESCRIPTOR
		);
		if ( pFilterDescriptor != NULL )
		{
		//Copy data.
			RtlCopyMemory( pFilterDescriptor, *ppFilterDescriptor, sizeof(FilterDescriptor_4xx_Wave_All) );

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
		  sizeof(KsPinDescriptors_4xx_Wave_All),	//all PinDescriptors have the same size
		  MEMORY_TAG_DRIVER_PIN_DESCRIPTOR
		);
		if ( pPinDescriptor != NULL )
		{
		//Copy data.
			RtlCopyMemory( pPinDescriptor, *ppPinDescriptors, sizeof(KsPinDescriptors_4xx_Wave_All) );

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
