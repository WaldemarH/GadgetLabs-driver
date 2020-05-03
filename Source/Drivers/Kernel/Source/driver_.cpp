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
//		driver.cpp
//
// PURPOSE:
//		File contains the class of all classes.
//
// STATUS:
//		Unfinished/Untested.
//
//***************************************************************************
//***************************************************************************
#include "main.h"
#include "asio_handle.h"
#include "notify_event.h"
#include "sync_start.h"


//***************************************************************************
//Global variables.
#pragma DATA_LOCKED
driver_c*		pDriver = NULL;		//main class

#pragma DATA_PAGED
const KSDEVICE_DISPATCH DeviceDispatch = 
{
	NULL,							// Pnp Add Device
	driver_c::Device_Start,			// Pnp Start
	NULL,							// Post-Start
	NULL,							// Pnp Query Stop
	NULL,							// Pnp Cancel Stop
	driver_c::Device_Stop,			// Pnp Stop
	NULL,							// Pnp Query Remove
	NULL,							// Pnp Cancel Remove
	NULL,							// Pnp Remove
	NULL,							// Pnp Query Capabilities
	NULL,							// Pnp Surprise Removal
	NULL,							// Power Query Power
	NULL,							// Power Set Power
	NULL							// Pnp Query Interface
};

//All FilterDescriptor-s will be created in GL_Class::DeviceDispatch_Start.
//Concept:
//- every card is a filter (one object)
//- every filter will have n+1 pin factories: -n: - WAVE OUT (n card out channels -> n pin_CardWaveOut-s/pin_FilterWaveIn-s  )
//                                                - WAVE IN  (n card in  channels -> n pin_CardWaveIn-s /pin_FilterWaveOut-s )
//                                            -1: - MIDI OUT (1 card out channel  -> 1 pin_CardMidiOut  /pin_FilterMidiIn    )
//                                                - MIDI IN  (1 card in  channel  -> 1 pin_CardMidiIn   /pin_FilterMidiOut   )
//
#pragma DATA_PAGED
const KSDEVICE_DESCRIPTOR DeviceDescriptor = 
{
	&DeviceDispatch,
	0,		//SIZEOF_ARRAY (FilterDescriptors),
	0,		//FilterDescriptors,
	KSDEVICE_DESCRIPTOR_VERSION
};

//***************************************************************************
// ProcedureName:
//		Constructor/Destructor
//
// Explanation:
//		Procedure will initialize/free variables of driver class.
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
// Return:
//		STATUS_SUCCESS or one of the error status values defined in ntstatus.h.
//
#pragma CODE_PAGED
driver_c::driver_c( IN OUT NTSTATUS* pStatus, IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPathName )
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	*pStatus = STATUS_SUCCESS;

//Initialize variables.
	m_pDriverObject = pDriverObject;
	m_pRegistryPathName = pRegistryPathName;

	//Cards
	RtlZeroMemory( &m_Cards, sizeof( m_Cards ) );

	KeInitializeSpinLock( &m_SpinLock_channel_wave );

	//ISR and DPC
	m_IRQLevel_highest = 0;
	KeInitializeSpinLock( &m_SpinLock_IRQ );

	//Buffer size
	SetBufferSize( 0x100 );

	//Synhronization mutex between AVStream and ASIO part of the driver.
	KeInitializeMutex( &m_Mutex_Driver, 0 );
#ifdef DBG_AND_LOGGER
	m_Mutex_locked = 0;
#endif

	//Device IO
	m_OriginalDriverDispatch_Close = NULL;
	m_OriginalDriverDispatch_Control = NULL;
	m_OriginalDriverDispatch_Create = NULL;

//Initialize AVStream driver.
	//AVStream variables...
	m_Interface_guidNamesUsed = 0;

	RtlZeroMemory( &m_pFilterDescriptors_Wave, sizeof( m_pFilterDescriptors_Wave ) );
	RtlZeroMemory( &m_pPinDescriptors_Wave, sizeof( m_pPinDescriptors_Wave ) );

	m_pSyncStart_master = new syncStart_c( NULL );
	if ( m_pSyncStart_master == NULL )
	{
		*pStatus = STATUS_INSUFFICIENT_RESOURCES;
	}

	//AVStream driver...
	if ( SUCCESS( *pStatus ) )
	{
		*pStatus = KsInitializeDriver( pDriverObject, pRegistryPathName, &DeviceDescriptor );
	}

//Initialize MIDI driver.
	RtlZeroMemory( &m_pFilterDescriptors_Midi, sizeof( m_pFilterDescriptors_Midi ) );
	RtlZeroMemory( &m_pPinDescriptors_Midi, sizeof( m_pPinDescriptors_Midi ) );

//Initialize ASIO driver.
	m_ASIO_Handle_master = NULL;

	InitializeListHead( &m_ASIO_ListHead );

	if ( SUCCESS( *pStatus ) )
	{
		m_ASIO_Handle_master = new asioHandle_c( NULL );
		if ( m_ASIO_Handle_master == NULL )
		{
			*pStatus = STATUS_INSUFFICIENT_RESOURCES;
		}
	}

//Initialize DATA.
	m_Data_SyncChecked = 0;

//Initialize GUI.
	InitializeListHead( &m_GUI_ListHead );
	m_GUI_IRP_Pending = NULL;
	KeInitializeSpinLock( &m_GUI_SpinLock_IRP_Pending );

	m_GUI_notifyEvent = NULL;
	m_GUI_notifyEvent_master = NULL;
	if ( SUCCESS( *pStatus ) )
	{
		m_GUI_notifyEvent_master = new notifyEvent_c( NULL );
		if ( m_GUI_notifyEvent_master == NULL )
		{
			*pStatus = STATUS_INSUFFICIENT_RESOURCES;
		}
	}
	KeInitializeSpinLock( &m_GUI_notifyEvent_spinLock );

//Patch major functions to allow private interface to driver.
	if ( SUCCESS( *pStatus ) )
	{
	//IRP_MJ_CREATE
		m_OriginalDriverDispatch_Create = pDriverObject->MajorFunction[IRP_MJ_CREATE];
		pDriverObject->MajorFunction[IRP_MJ_CREATE] = DeviceDispatch_CreateClose;

	//IRP_MJ_CLOSE
		m_OriginalDriverDispatch_Close = pDriverObject->MajorFunction[IRP_MJ_CLOSE];
		pDriverObject->MajorFunction[IRP_MJ_CLOSE] = DeviceDispatch_CreateClose;

	//IRP_MJ_DEVICE_CONTROL
		m_OriginalDriverDispatch_Control = pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL];
		pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceDispatch_Control;
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( *pStatus );
#endif
}

#pragma CODE_PAGED
driver_c::~driver_c()
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

//Free resources.
	//GUI variables.
	if ( m_GUI_notifyEvent_master != NULL )
	{
		delete m_GUI_notifyEvent_master;
		m_GUI_notifyEvent_master = NULL;
	}

	//AVStream variables...
	if ( m_pSyncStart_master != NULL )
	{
		delete m_pSyncStart_master;
		m_pSyncStart_master = NULL;
	}

	//Free cards.
	//
	//Notice:
	//If everything works as it should this loop will not free any of the cards.
	for ( int i = 0; i < MAX_CARDS; i++ )
	{
		if ( m_Cards[i] != NULL )
		{
			delete m_Cards[i];
			m_Cards[i] = NULL;
		}
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( STATUS_SUCCESS );
#endif
}

//***************************************************************************
// ProcedureName:
//		operator new/delete
//
// Explanation:
//		Procedure will allocate/deallocate class memory.
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
//		...
//
#pragma CODE_PAGED
void* __cdecl driver_c::operator new ( size_t size )
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	void*		pVoid;
	if ( size > 0 )
	{
		pVoid = (void*)ExAllocatePoolWithTag( NonPagedPool, size, MEMORY_TAG_DRIVER_CLASS );
	}
	else
	{
		pVoid = NULL;
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( ( pVoid != NULL ) ? STATUS_SUCCESS : STATUS_INSUFFICIENT_RESOURCES );
#endif
	return pVoid;

}

#pragma CODE_PAGED
void __cdecl driver_c::operator delete ( void* pMemory )
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	if ( pMemory != NULL )
	{
		ExFreePoolWithTag( pMemory, MEMORY_TAG_DRIVER_CLASS );
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( STATUS_SUCCESS );
#endif
}

//***************************************************************************
// ProcedureName:
//		SetBufferSize
//
// Explanation:
//		Procedure will set new buffer size.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// Synhronization:
//		m_SpinLock_channelInfo
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return Codes:
//		STATUS_SUCCESS or one of the error status values defined in ntstatus.h.
//
#pragma CODE_LOCKED
void driver_c::SetBufferSize
(
	unsigned long	bufferSize_inSamples
)
{
//Is the new buffer size the same as current one?
	if ( m_BufferSize == bufferSize_inSamples )
	{
	//Yes -> well all has been done in this case.
		return;
	}

//Set new buffer size.
//
//The biggest buffer allowed by the driver is 0x200 = 16ms.
	switch ( bufferSize_inSamples )
	{
	case 0x20:
	case 0x40:
	case 0x80:
	case 0x100:
	case 0x200:
	case 0x400:
		break;
	default:
		bufferSize_inSamples = 0x100;	//for any other value
		break;
	}
	m_BufferSize = bufferSize_inSamples;
	m_BufferSize_minus1 = (unsigned short) ( bufferSize_inSamples - 1 );
	m_BufferSize_minus1Negated = ~( (signed long)m_BufferSize_minus1 );
	m_BufferSize_plus1 = (unsigned short) ( bufferSize_inSamples + 1 );

//Define AllocatorFraming_Wave
//Supported number of frames : 1
//Size of frame : bufferSize
//
//Notice:
//Framing isn't used by the system most of the time... as much as reports go only the input channels are used.
//But as some applications can use direct access to the driver, we have to support this too.
//
//Defining the framing size:
//as the driver supports 16bit (2bytes packed),
//24bit(3 bytes packled) and 32bit (4 bytes) sample containers,
//we need to define this in framing.
//As the lower engine is copying samples and not bytes->
//it divides/multiplies the dataSize so we must be sure it's
//aligned for 3byte and 4byte containers.
//
//framingSize_inBytes =
//  ( bufferSize_inSamples * nChannels )
//  *
//  (4_bytes_perSample*3_byte_perSample)
//
//This is because when we divide with 3 or 4 we will
//get a full number-> no reminder which is important as
//audio engine can't handle the reminding bytes->
//it just loops->freezes the system.
//
	unsigned long	bufferSize_inBytes_1Channel = bufferSize_inSamples * 4 * 3;

	//PhysicalRange
	m_AllocatorFraming_Wave_Stereo.FramingItem[0].PhysicalRange.MinFrameSize = bufferSize_inBytes_1Channel * 2;
	m_AllocatorFraming_Wave_Stereo.FramingItem[0].PhysicalRange.MaxFrameSize = bufferSize_inBytes_1Channel * 2;
	m_AllocatorFraming_Wave_Stereo.FramingItem[0].PhysicalRange.Stepping = 0;

	m_AllocatorFraming_4xx_Wave_All.FramingItem[0].PhysicalRange.MinFrameSize = bufferSize_inBytes_1Channel * 4;
	m_AllocatorFraming_4xx_Wave_All.FramingItem[0].PhysicalRange.MaxFrameSize = bufferSize_inBytes_1Channel * 4;
	m_AllocatorFraming_4xx_Wave_All.FramingItem[0].PhysicalRange.Stepping = 0;

	m_AllocatorFraming_824_Wave_All.FramingItem[0].PhysicalRange.MinFrameSize = bufferSize_inBytes_1Channel * 8;
	m_AllocatorFraming_824_Wave_All.FramingItem[0].PhysicalRange.MaxFrameSize = bufferSize_inBytes_1Channel * 8;
	m_AllocatorFraming_824_Wave_All.FramingItem[0].PhysicalRange.Stepping = 0;

	//FramingRange
	m_AllocatorFraming_Wave_Stereo.FramingItem[0].FramingRange.Range.MinFrameSize = bufferSize_inBytes_1Channel * 2;
	m_AllocatorFraming_Wave_Stereo.FramingItem[0].FramingRange.Range.MaxFrameSize = bufferSize_inBytes_1Channel * 2;
	m_AllocatorFraming_Wave_Stereo.FramingItem[0].FramingRange.Range.Stepping = 0;

	m_AllocatorFraming_4xx_Wave_All.FramingItem[0].FramingRange.Range.MinFrameSize = bufferSize_inBytes_1Channel * 4;
	m_AllocatorFraming_4xx_Wave_All.FramingItem[0].FramingRange.Range.MaxFrameSize = bufferSize_inBytes_1Channel * 4;
	m_AllocatorFraming_4xx_Wave_All.FramingItem[0].FramingRange.Range.Stepping = 0;

	m_AllocatorFraming_824_Wave_All.FramingItem[0].FramingRange.Range.MinFrameSize = bufferSize_inBytes_1Channel * 8;
	m_AllocatorFraming_824_Wave_All.FramingItem[0].FramingRange.Range.MaxFrameSize = bufferSize_inBytes_1Channel * 8;
	m_AllocatorFraming_824_Wave_All.FramingItem[0].FramingRange.Range.Stepping = 0;

	return;
}
