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
//		File contains definitions for main class.
//
//***************************************************************************
//***************************************************************************
#pragma once

//Includes
#include "delay.h"
#include "port.h"
#include "logger.h"

class asioHandle_c;
class card_c;
class notifyEvent_c;
class syncStart_c;

class driver_c : public port_c, public delay_c
{
//As this is a global class we'll simulate constructor and destructor.
	public: driver_c( IN OUT NTSTATUS* pStatus, IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPathName );
	public: ~driver_c();

	private: PDRIVER_OBJECT			m_pDriverObject;
	private: PUNICODE_STRING		m_pRegistryPathName;

//Class memory
	#define MEMORY_TAG_DRIVER_CLASS		'LCRD'	//tag: "DRCL"

	public: static void* __cdecl operator new( size_t size );
	public: static void __cdecl operator delete( void* pMemory );

//Cards
	public: card_c*					m_Cards[MAX_CARDS];

	//It will be used for channel info sinhronization.
	public: KSPIN_LOCK				m_SpinLock_channel_wave;

//ISR and DPC
	public: static BOOLEAN InterruptServiceRoutine_Static( IN struct _KINTERRUPT* Interrupt, IN PVOID ServiceContext );
	public: static void DeferredProcedureCall_Static_Wave( IN PKDPC Dpc, IN PVOID DeferredContext, IN PVOID SystemArgument1, IN PVOID SystemArgument2 );
	public: static void DeferredProcedureCall_Static_Midi_In( IN PKDPC Dpc, IN PVOID DeferredContext, IN PVOID SystemArgument1, IN PVOID SystemArgument2 );
	public: static void DeferredProcedureCall_Static_Midi_Out( IN PKDPC Dpc, IN PVOID DeferredContext, IN PVOID SystemArgument1, IN PVOID SystemArgument2 );

	public: __forceinline BOOLEAN InterruptServiceRoutine( IN card_c* pCard_master );
	private: __forceinline void DeferredProcedureCall_Wave( IN card_c* pCard_master );
	private: __forceinline void DeferredProcedureCall_Midi_In( IN card_c* pCard );
	private: __forceinline void DeferredProcedureCall_Midi_Out( IN card_c* pCard );

	//It will be used for interrupt sinhronization.
	public: ULONG					m_IRQLevel_highest;
	public: KSPIN_LOCK				m_SpinLock_IRQ;

//Buffer size
//
//Notice:
//Biggest buffer size is 0x200 (this will be default value).
	public: void SetBufferSize( IN unsigned long newBufferSize );

	public: unsigned long			m_BufferSize;				//in samples
	public: unsigned short			m_BufferSize_minus1;
	public: unsigned short			m_BufferSize_plus1;
	public: signed long				m_BufferSize_minus1Negated;

//Synhronization mutex between AVStream and ASIO part of the driver.
	public: inline NTSTATUS LockDriver( void )
	{
	//Acquire mutex.
	#ifdef DBG_AND_LOGGER
		Procedure_Start_Local();
	#endif
		NTSTATUS	Status = KeWaitForSingleObject( (void*)&m_Mutex_Driver, Executive, KernelMode, FALSE, NULL );

	#ifdef DBG_AND_LOGGER
		++m_Mutex_locked;
		pLogger->Log( "  Locked: %d.\n", m_Mutex_locked );
	#endif
		return Status;
	}
	public: inline void UnlockDriver( void )
	{
	//Release mutex.
	#ifdef DBG_AND_LOGGER
		if ( m_Mutex_locked != 1 )
		{
			pLogger->Log( "  ERROR -> Unlocking driver without locking.\n" );
		}
		--m_Mutex_locked;
	#endif
		KeReleaseMutex( &m_Mutex_Driver, FALSE );
	#ifdef DBG_AND_LOGGER
		Procedure_Exit( STATUS_SUCCESS );
	#endif
	}

	private: KMUTEX					m_Mutex_Driver;
#ifdef DBG_AND_LOGGER
	private: unsigned long			m_Mutex_locked;
#endif

//*********************************************************************************************************************
//Device IO
//
//GUID: {F3292AD7-71A1-4688-8504 -01641702F5DC}
	public: static NTSTATUS DeviceDispatch_Control( PDEVICE_OBJECT pFunctionalDeviceObject, PIRP pIrp );
	public: static NTSTATUS DeviceDispatch_CreateClose( PDEVICE_OBJECT pFunctionalDeviceObject, PIRP pIrp );

	private: NTSTATUS Interface_Register_DeviceIO( card_c* pCard );
	private: void Interface_Unregister_DeviceIO( card_c* pCard );

	private: card_c*				m_Interface_Card;	//if NULL the interface was not set

	private: PDRIVER_DISPATCH		m_OriginalDriverDispatch_Close;
	private: PDRIVER_DISPATCH		m_OriginalDriverDispatch_Control;
	private: PDRIVER_DISPATCH		m_OriginalDriverDispatch_Create;

//*********************************************************************************************************************
//AVStream
//-----------------------------------------------------------------------------
//Start/Stop AVStream driver.
	public: static NTSTATUS Device_Start( IN PKSDEVICE pDevice, IN PIRP pIrp, IN PCM_RESOURCE_LIST pTranslatedResourceList OPTIONAL, IN PCM_RESOURCE_LIST pUntranslatedResourceList OPTIONAL );
	public: static void Device_Stop( IN PKSDEVICE pDevice, IN PIRP pIrp );

//-----------------------------------------------------------------------------
//Register/Unregister filters... expose channels to system.
	private: NTSTATUS Interface_Disable_Filter( IN card_c* pCard, IN PWCHAR pInterfaceName );
	private: NTSTATUS Interface_Register_Filter( IN card_c* pCard, IN PWCHAR pInterfaceName, IN PWCHAR pFriendlyName );
	private: NTSTATUS Interface_SetPinNames( IN PWCHAR pFriendlyName, OUT const GUID** ppGUID );

	private: NTSTATUS Channels_CopyDescriptors_Wave( IN OUT KSFILTER_DESCRIPTOR** ppFilterDescriptor, IN OUT KSPIN_DESCRIPTOR_EX** ppPinDescriptors );
	private: NTSTATUS Channels_RegisterToSystem_Wave( card_c* pCard );
	private: void Channels_UnregisterFromSystem_Wave( card_c* pCard );

	private: unsigned long			m_Interface_guidNamesUsed;

	private: KSFILTER_DESCRIPTOR*	m_pFilterDescriptors_Wave[MAX_CARDS][5];		//max = 4 stereo + 1 all
	private: KSPIN_DESCRIPTOR_EX*	m_pPinDescriptors_Wave[MAX_CARDS][5];			//max = 4 stereo + 1 all

	#define MEMORY_TAG_DRIVER_FILTER_DESCRIPTOR		'DFRD'	//tag: "DRFD"
	#define MEMORY_TAG_DRIVER_PIN_DESCRIPTOR		'DPRD'	//tag: "DRPD"

//-----------------------------------------------------------------------------
//Pin properties.
	public: static NTSTATUS PropertyItem_Wave_Audio_Position_Handler_Get( IN PIRP pIrp, IN PKSIDENTIFIER pRequest, IN OUT PKSAUDIO_POSITION pData );
	public: static NTSTATUS PropertyItem_Wave_Audio_Position_Handler_Set( IN PIRP pIrp, IN PKSIDENTIFIER pRequest, IN OUT PKSAUDIO_POSITION pData );

	public: static KSALLOCATOR_FRAMING_EX	m_AllocatorFraming_4xx_Wave_All;
	public: static KSALLOCATOR_FRAMING_EX	m_AllocatorFraming_824_Wave_All;
	public: static KSALLOCATOR_FRAMING_EX	m_AllocatorFraming_Wave_Stereo;

//-----------------------------------------------------------------------------
//Pin management.
	public: static NTSTATUS Pin_AudioIntersectHandler( IN PVOID pContext, IN PIRP pIrp, IN PKSP_PIN pPin, IN PKSDATARANGE_AUDIO pCallerDataRange, IN PKSDATARANGE_AUDIO pDescriptorDataRange, IN ULONG dataBufferSize, OUT PVOID pData OPTIONAL, OUT PULONG pDataSize );
	public: static NTSTATUS Pin_Close_Wave( IN PKSPIN pPin, IN PIRP pIrp );
	public: static NTSTATUS Pin_Create_Wave( IN PKSPIN pPin, IN PIRP pIrp );
	public: static NTSTATUS Pin_Process_Wave( IN PKSPIN pPin );
	public: static NTSTATUS Pin_SetDataFormat_Wave( IN PKSPIN pPin, IN PKSDATAFORMAT pOldFormat, IN PKSMULTIPLE_ITEM pOldAttributeList, IN const PKSDATARANGE_AUDIO pAudioDataRange, IN const KSATTRIBUTE_LIST* pAttributeRange );
	public: static NTSTATUS Pin_SetDeviceState_Wave( IN PKSPIN pPin, IN KSSTATE ToState, IN KSSTATE FromState );

	public: syncStart_c*			m_pSyncStart_master;

//-----------------------------------------------------------------------------
//Common functions.
	private: NTSTATUS GetCard_FromFilter( IN PKSFILTER pFilter, IN OUT card_c** ppCard );
	private: NTSTATUS GetCard_FromPin( IN PKSPIN pPin, IN OUT card_c** ppCard );

//*********************************************************************************************************************
//MIDI
//
//Notice:
//Some functions are shared with AVStream section.
//-----------------------------------------------------------------------------
//Register/Unregister filters... expose channels to system.
	private: KSFILTER_DESCRIPTOR*	m_pFilterDescriptors_Midi[MAX_CARDS];
	private: KSPIN_DESCRIPTOR_EX*	m_pPinDescriptors_Midi[MAX_CARDS];

	private: NTSTATUS Channels_CopyDescriptors_Midi( IN OUT KSFILTER_DESCRIPTOR** ppFilterDescriptor, IN OUT KSPIN_DESCRIPTOR_EX** ppPinDescriptors );
	private: NTSTATUS Channels_RegisterToSystem_Midi( IN card_c* pCard );
	private: void Channels_UnregisterFromSystem_Midi( IN card_c* pCard );

//-----------------------------------------------------------------------------
//Node properties.
	public: static NTSTATUS PropertyItem_Midi_Synth( IN PIRP pIrp, IN PKSIDENTIFIER pRequest, IN OUT PVOID pData );
	public: static NTSTATUS PropertyItem_Midi_Synth_Capabilities_Get( IN PIRP pIrp, IN PKSIDENTIFIER pRequest, IN OUT SYNTHCAPS* pData );
	public: static NTSTATUS PropertyItem_Midi_Synth_Clock( IN PIRP pIrp, IN PKSIDENTIFIER pRequest, IN OUT ULONGLONG* pData );

//-----------------------------------------------------------------------------
//Pin management.
	public: static NTSTATUS Pin_Close_Midi( IN PKSPIN pPin, IN PIRP pIrp );
	public: static NTSTATUS Pin_Create_Midi( IN PKSPIN pPin, IN PIRP pIrp );
	public: static NTSTATUS Pin_Process_Midi( IN PKSPIN pPin );
	public: static NTSTATUS Pin_SetDeviceState_Midi( IN PKSPIN pPin, IN KSSTATE ToState, IN KSSTATE FromState );

//-----------------------------------------------------------------------------
//Common functions.

//*********************************************************************************************************************
//ASIO
	public: NTSTATUS ASIO_CopyOutputBuffers( IN OUT unsigned long* pSystemBuffer, IN unsigned long bufferLength_input, IN unsigned long bufferLength_output, IN OUT unsigned long* pInformationSize );
	public: NTSTATUS ASIO_GetCloseHandle( IN OUT unsigned long* pSystemBuffer, IN unsigned long bufferLength_input, IN unsigned long bufferLength_output, IN OUT unsigned long* pInformationSize );
	public: NTSTATUS ASIO_EngineVersion( IN OUT unsigned long* pSystemBuffer, IN unsigned long bufferLength_input, IN unsigned long bufferLength_output, IN OUT unsigned long* pInformationSize );
	public: NTSTATUS ASIO_OpenCloseChannels( IN OUT void* pSystemBuffer, IN unsigned long bufferLength_input, IN unsigned long bufferLength_output, IN OUT unsigned long* pInformationSize );
	public: NTSTATUS ASIO_ProtectionIRP_Remove( IN OUT unsigned long* pSystemBuffer, IN unsigned long bufferLength_input, IN unsigned long bufferLength_output, IN OUT unsigned long* pInformationSize );
	public: NTSTATUS ASIO_ProtectionIRP_Set( IN PIRP pIrp, IN OUT unsigned long* pSystemBuffer, IN unsigned long bufferLength_input, IN unsigned long bufferLength_output, IN OUT unsigned long* pInformationSize );

	public: NTSTATUS ASIO_LockMemory( IN OUT void* pSystemBuffer, IN unsigned long bufferLength_input, IN unsigned long bufferLength_output, IN OUT unsigned long* pInformationSize );
	public: NTSTATUS ASIO_UnlockMemory( IN OUT unsigned long* pSystemBuffer, IN unsigned long bufferLength_input, IN unsigned long bufferLength_output, IN OUT unsigned long* pInformationSize );

	public: NTSTATUS ASIO_Start( IN OUT void* pSystemBuffer, IN unsigned long bufferLength_input, IN unsigned long bufferLength_output, IN OUT unsigned long* pInformationSize );
	public: NTSTATUS ASIO_Stop( IN OUT unsigned long* pSystemBuffer, IN unsigned long bufferLength_input, IN unsigned long bufferLength_output, IN OUT unsigned long* pInformationSize );

	public: NTSTATUS ASIO_SwitchEvent_Release( IN OUT unsigned long* pSystemBuffer, IN unsigned long bufferLength_input, IN unsigned long bufferLength_output, IN OUT unsigned long* pInformationSize );
	public: NTSTATUS ASIO_SwitchEvent_Set( IN PIRP pIrp, IN void* pSystemBuffer, IN unsigned long bufferLength_input, IN unsigned long bufferLength_output, IN OUT unsigned long* pInformationSize );

	public: static VOID ASIO_PendingIoctl_Cancel_Static( PDEVICE_OBJECT pDeviceObject, PIRP pIrp );
	private: void ASIO_PendingIoctl_Cancel( PIRP pIrp );

	private: asioHandle_c*			m_ASIO_Handle_master;		//protect with m_SpinLock_channel_wave

	private: LIST_ENTRY				m_ASIO_ListHead;

//*********************************************************************************************************************
//DATA
	public: NTSTATUS Data_BufferSize( IN OUT unsigned long* pSystemBuffer, IN unsigned long bufferLength_input, IN unsigned long bufferLength_output, IN OUT unsigned long* pInformationSize );
	public: NTSTATUS Data_CardsInformation( IN OUT void* pSystemBuffer, IN unsigned long bufferLength_input, IN unsigned long bufferLength_output, IN OUT unsigned long* pInformationSize );
	public: NTSTATUS Data_ChannelProperty( IN OUT void* pSystemBuffer, IN unsigned long bufferLength_input, IN unsigned long bufferLength_output, IN OUT unsigned long* pInformationSize );
	public: NTSTATUS Data_EngineVersion( IN OUT unsigned long* pSystemBuffer, IN unsigned long bufferLength_input, IN unsigned long bufferLength_output, IN OUT unsigned long* pInformationSize );
	public: NTSTATUS Data_SampleRate( IN OUT void* pSystemBuffer, IN unsigned long bufferLength_input, IN unsigned long bufferLength_output, IN OUT unsigned long* pInformationSize );
	public: NTSTATUS Data_SyncCards( IN OUT unsigned long* pSystemBuffer, IN unsigned long bufferLength_input, IN unsigned long bufferLength_output );

	private: void Data_SetCardsOrder();

	private: unsigned long			m_Data_SyncChecked;			//sync was checked when this number of cards were in the system

//*********************************************************************************************************************
//GUI
	public: NTSTATUS GUI_EngineVersion( IN OUT unsigned long* pSystemBuffer, IN unsigned long bufferLength_input, IN unsigned long bufferLength_output, IN OUT unsigned long* pInformationSize );
	public: NTSTATUS GUI_Notification_GetEvent( IN OUT void* pSystemBuffer, IN unsigned long bufferLength_input, IN unsigned long bufferLength_output, IN OUT unsigned long* pInformationSize );
	public: NTSTATUS GUI_Notification_Release( IN unsigned long bufferLength_input, IN unsigned long bufferLength_output );
	public: NTSTATUS GUI_Notification_Set( IN PIRP pIrp, IN HANDLE* pHandle, IN unsigned long bufferLength_input, IN unsigned long bufferLength_output, IN OUT unsigned long* pInformationSize );
	public: NTSTATUS GUI_ProtectionIRP_Remove( IN unsigned long bufferLength_input, IN unsigned long bufferLength_output );
	public: NTSTATUS GUI_ProtectionIRP_Set( IN PIRP pIrp, IN unsigned long bufferLength_input, IN unsigned long bufferLength_output );

	public: static VOID GUI_PendingIoctl_Cancel_Static( PDEVICE_OBJECT pDeviceObject, PIRP pIrp );
	private: void GUI_PendingIoctl_Cancel( PIRP pIrp );

	private: LIST_ENTRY				m_GUI_ListHead;
	private: PIRP					m_GUI_IRP_Pending;
	private: KSPIN_LOCK				m_GUI_SpinLock_IRP_Pending;

	public: PKEVENT					m_GUI_notifyEvent;
	public: notifyEvent_c*			m_GUI_notifyEvent_master;
	public: KSPIN_LOCK				m_GUI_notifyEvent_spinLock;

};
extern driver_c*		pDriver;

