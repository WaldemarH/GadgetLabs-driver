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
#include "card_altera_firmware.h"
#include "card_uart.h"
#include "driver_ioctl_data.h"

#pragma DATA_LOCKED


#pragma CODE_PAGED
card_c::card_c( IN PKSDEVICE DeviceObject, unsigned long serialNumber )
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

//Initialize class members to default values.
	//System resources.
	m_DeviceObject = DeviceObject;
	RtlZeroMemory( &m_SymbolicLinkName, sizeof( m_SymbolicLinkName ) );

	//Card type
	m_Type = CARD_TYPE_NOT_SET;
	m_Type_subVersion = 0;

	//Resources
	m_IoPlx = NULL;
	m_IoWave = NULL;
	m_IoUart = NULL;

	m_Mem_Plx = NULL;
	m_Mem_Analog = NULL;

	m_Mem_Plx_length = 0;
	m_Mem_Analog_length = 0;

	m_IRQ.Level = 0;
	m_IRQ.Vector = 0;
	m_IRQ.Affinity = NULL;
	m_IRQ.flags = 0;
	m_pIRQObject = NULL;
	RtlZeroMemory( &m_DPCObject_wave, sizeof( m_DPCObject_wave ) );
	RtlZeroMemory( &m_DPCObject_midi_in, sizeof( m_DPCObject_midi_in ) );
	RtlZeroMemory( &m_DPCObject_midi_out, sizeof( m_DPCObject_midi_out ) );
	KeInitializeTimerEx( &m_Timer_midi, SynchronizationTimer );
	KeInitializeSpinLock( &m_SpinLock_channel_midi_in );
	KeInitializeSpinLock( &m_SpinLock_channel_midi_out );

	//Sample Rate
	m_SampleRate = 0;
	m_SampleRate_owned = 0;
	m_ClockSource = CLOCK_SOURCE_NOT_SET;

	//Timestamp
	m_Clock_samples.clock = CLOCK_ZERO_TIMESTAMP;
	m_Clock_samples_previous.clock = 0;
	m_TimeStamp_wave = 0;

	m_TimeStamp_midi = 0;

	//Synhronization
	m_pCard_master = NULL;
	m_pCard_slave = NULL;

	m_SerialNumber = (unsigned char)serialNumber;
	m_SerialNumber_masterCard = 0;		//will be set in card ordering function
	m_SerialNumber_slaveCard = 0;		//will be set in card ordering function

	//Streaming
	m_StopStream_midi = YES;
	m_StopStream_wave = YES;

	//Channels handling (Wave)
	m_pChannels_All_INs = NULL;
	m_pChannels_All_OUTs = NULL;

	RtlZeroMemory( m_pChannels_Stereo_INs, sizeof(m_pChannels_Stereo_INs) );
	RtlZeroMemory( m_pChannels_Stereo_OUTs, sizeof(m_pChannels_Stereo_OUTs) );

	m_Channels_opened_INs_ASIO = 0;
	m_Channels_opened_OUTs_ASIO = 0;
	m_Channels_opened_INs_AVStream = 0;
	m_Channels_opened_OUTs_AVStream = 0;

	m_Channels_nInUse_INs_ASIO = 0;
	m_Channels_nInUse_OUTs_ASIO = 0;
	m_Channels_nInUse_INs_AVStream = 0;
	m_Channels_nInUse_OUTs_AVStream = 0;

	m_pFilterFactories_All = NULL;
	RtlZeroMemory( m_pFilterFactories_Stereo, sizeof(m_pFilterFactories_Stereo) );

	m_Property_4xxCards_Mute[0] = PROPERTY_MUTE_OFF;		//CODECs are initialized with volume to max
	m_Property_4xxCards_Mute[1] = PROPERTY_MUTE_OFF;
	m_Property_4xxCards_Mute[2] = PROPERTY_MUTE_OFF;
	m_Property_4xxCards_Mute[3] = PROPERTY_MUTE_OFF;

	m_ChannelOffset = 0;
	m_ChannelOffset_ASIO_out = 0;

	m_ChannelOffset_In = 0;
	m_ChannelOffset_Out = 0;

	//Channel handling (Midi)
	m_pFilterFactories_Midi = NULL;

	m_pChannels_Midi_INs = NULL;
	m_pChannels_Midi_OUTs = NULL;

#ifdef DBG_AND_LOGGER
	Procedure_Exit( STATUS_SUCCESS );
#endif
}

#pragma CODE_PAGED
card_c::~card_c()
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

//Free resources.
	//Detach DPC and ISR.
	DetachIRQ();

	//Channels (Midi)
	if ( m_pChannels_Midi_INs != NULL )
	{
		m_pChannels_Midi_INs = NULL;
	}
	if ( m_pChannels_Midi_OUTs != NULL )
	{
		m_pChannels_Midi_OUTs = NULL;
	}

	//Channels (Wave).
	if ( m_pChannels_All_INs != NULL )
	{
		delete m_pChannels_All_INs;
	}
	if ( m_pChannels_All_OUTs != NULL )
	{
		delete m_pChannels_All_OUTs;
	}

	for ( int i = 0; i < ELEMENTS_IN_ARRAY( m_pChannels_Stereo_INs ); i++ )
	{
		if ( m_pChannels_Stereo_INs[i] != NULL )
		{
			delete m_pChannels_Stereo_INs[i];
		}
		if ( m_pChannels_Stereo_OUTs[i] != NULL )
		{
			delete m_pChannels_Stereo_OUTs[i];
		}
	}

	//IO resources.
	ReleaseIOResources();

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
void* __cdecl card_c::operator new ( size_t size )
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	void*		pVoid;
	if ( size > 0 )
	{
		pVoid = (void*)ExAllocatePoolWithTag( NonPagedPool, size, MEMORY_TAG_CARD_CLASS );
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
void __cdecl card_c::operator delete ( void* pMemory )
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	if ( pMemory != NULL )
	{
		ExFreePoolWithTag( pMemory, MEMORY_TAG_CARD_CLASS );
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( STATUS_SUCCESS );
#endif
}

//*****************************************************************************
// ProcedureName:
//		Initialize
//
// Explanation:
//		This procedure will initialize the hardware and link software to it.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
// Return:
//		Should return STATUS_SUCCESS or the error code that was returned by the system.
//
#pragma CODE_PAGED
NTSTATUS card_c::Initialize( IN PCM_RESOURCE_LIST TranslatedResourceList )
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	NTSTATUS		Status;

//Set card type.
	Status = SetCardType();

//Parse and acquire IO resources.
	if ( SUCCESS( Status ) )
	{
		Status = AcquireIOResources( TranslatedResourceList );
	}

//Protect driver from unwanted card interrupts.
	if ( SUCCESS( Status ) )
	{
		PLX_SetGlobalInteruptState( CARD_GLOBAL_INTERRUPT_DISABLE );
	}

//Repair eeprom PLX initialize values.
	if ( SUCCESS( Status ) )
	{
		PLX_SetTransferFlags();
	}

//Unfreeze local bus and check if card is set corectly.
	if ( SUCCESS( Status ) )
	{
	//Unfreeze local bus.
		PLX_SetLocalBusState( CARD_LOCAL_BUS_ACCESS_ALLOWED );

	//Test if we can write/read to/from the card.
	//
	//Notice:
	//If we wont't be able to write to local bus whole system will freeze.
		WriteByte( m_IoUart, UART_SCR, 0xAA );
		if ( 0xAA != ReadByte( m_IoUart, UART_SCR ) )
		{
			Status = STATUS_UNSUCCESSFUL;
		}
	}

//Initialize UART (used for MIDI).
	if ( SUCCESS( Status ) )
	{
		UART_SetFlagsForMIDI();
	}

//Load firmware to card.
	if ( SUCCESS( Status ) )
	{
		Status = Altera_LoadAndTestFirmware();
	}

//Programm PLL, start card sound engine (in firmware) and program Codecs.
	if ( SUCCESS( Status ) )
	{
		if ( m_Type != CARD_TYPE_824 )
		{
		//4xx
		//
		//Set up PLL.
			Status = PLL_Set4xxPll();

		//Start card sound engine (in firmware) and program Codecs
			if ( SUCCESS( Status ) )
			{
			//Initialize card engine.
				//Release counters (unreleased counters are set to 0 automatically).
				WORD	data = ReadWord( m_IoWave, SC_ENGINE );
				data |= SC_RELEASE_COUNTERS;
				WriteWord( m_IoWave, SC_ENGINE, data );
				DelayThreadShort( DELAY_THREAD_SHORT_1us );

				//Start clocks.
				data |= SC_START_AUDIO_ENGINE;
				WriteWord( m_IoWave, SC_ENGINE, data );
				DelayThreadShort( DELAY_THREAD_SHORT_10us );

			//Set codecs and unpause DACs.
				CODEC_Set4xxCodecs();
			}
		}
		else
		{
		//824
		//
		//Initialize card engine.
		//
		//ADC: As HP DEFEAT is permanently tied to GND (HP DEFEAT ENABLED) it means that
		//     highPass filter is permanently turned on. And as it's a digital filter
		//     it means that it takes some time before it's stable.
		//     As this filter is constantly enabled we don't need any calibration of the
		//     ADCs to be done.
		//
		//DAC: Every time the clock will be changed the converters will go into power down
		//     state. And when the new clocks will apear at their inputs the converters
		//     will go out of the power down state and do the calibration automatically
		//     (2700 * sampleRate => for 44.1kHz it takes 61ms ).
		//     This means that we don't have to wait for DACs also.
		//
		//So all we need to do is to turn up the clocks and let converters do their job.
			//Release counters (unreleased counters are set to 0 automatically).
			WORD	data = ReadWord( m_IoWave, SC_ENGINE );
			data |= SC_RELEASE_COUNTERS;
			WriteWord( m_IoWave, SC_ENGINE, data );
			DelayThreadShort( DELAY_THREAD_SHORT_1us );

			//Start clocks.
			data |= SC_START_AUDIO_ENGINE;
			WriteWord( m_IoWave, SC_ENGINE, data );
			DelayThreadShort( DELAY_THREAD_SHORT_10us );
		}

	//Default sample rate and clockSource.
		m_SampleRate = 44100;
		m_ClockSource = CLOCK_SOURCE_CARD;
	}

//Set channels (buffers pointers, receive/send procedures pointers,..).
	if ( SUCCESS( Status ) )
	{
		Status = SetChannels();
	}

//Attach IRQ to driver and initialize DPC routines.
	if ( SUCCESS( Status ) )
	{
		Status = AttachIRQ();
	}

//Enable card interrupts.
	if ( SUCCESS( Status ) )
	{
		PLX_SetGlobalInteruptState( CARD_GLOBAL_INTERRUPT_ENABLE );
	}

//Exit.
#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

//*****************************************************************************
// ProcedureName:
//		AttachIRQ
//
// Explanation:
//		In this procedure we will set up the IRQ and DPC procedures for
//		curentCard.
//
//		Notice:
//		If IRQ level of curent card is higher than the highest one then
//		procedure will change SynchronizeIrql of all already installed cards.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
// Return:
//		Should return STATUS_SUCCESS or the error code that was returned by the system.
//
#pragma CODE_PAGED
NTSTATUS card_c::AttachIRQ
(
	void
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	NTSTATUS	Status = STATUS_SUCCESS;

//Initialize DPC...
	KeInitializeDpc( &m_DPCObject_midi_in, &driver_c::DeferredProcedureCall_Static_Midi_In, this );
	KeInitializeDpc( &m_DPCObject_midi_out, &driver_c::DeferredProcedureCall_Static_Midi_Out, this );
	KeInitializeDpc( &m_DPCObject_wave, &driver_c::DeferredProcedureCall_Static_Wave, this );

//Initialize m_IRQLevel_highest.
	if ( pDriver->m_IRQLevel_highest == 0 )
	{
	//Set new highest DIRQ level.
		pDriver->m_IRQLevel_highest = (KIRQL)m_IRQ.Level;
	}

//Check if current card IRQ level is higher than that of highest one.
//If it is change the SynchronizeIrql level for all already installed cards.
//
//At start m_IRQLevel_highest = 0 so the first card will set it's IRQLevel as default.
//
	if ( pDriver->m_IRQLevel_highest < m_IRQ.Level )
	{
	#ifdef DBG_AND_LOGGER
		pLogger->Log( "  Current card has higher IRQ level (old: %d, new: %d) -> re-set ISR routine.\n", pDriver->m_IRQLevel_highest, m_IRQ.Level );
	#endif

	//Set new highest DIRQ level.
		pDriver->m_IRQLevel_highest = (KIRQL)m_IRQ.Level;

	//Current card IRQ level is higher than current highest->
	//disconnect all already connected IRQs and reconnect them with new settings.
		for ( int i = 0; i < ELEMENTS_IN_ARRAY( pDriver->m_Cards ); i++ )
		{
			card_c*		pCard_current = pDriver->m_Cards[i];
			if ( pCard_current == NULL )
				continue;

		//Mask out card interrupts.
		//This will be done at PLX chip so other parts of the driver wont even know this has happened.
			pCard_current->PLX_SetGlobalInteruptState( CARD_GLOBAL_INTERRUPT_DISABLE );

			//Wait a little just to be sure the higher level part has stopped.
			DelayThreadLong_PassiveLevel( DELAY_THREAD_LONG_15ms );

		//Now it's safe to disconnect ISR.
			IoDisconnectInterrupt( pCard_current->m_pIRQObject );
			pCard_current->m_pIRQObject = NULL;

		//Connect interrupt with new settings.
			Status = IoConnectInterrupt(
			  &pCard_current->m_pIRQObject,
			  &pDriver->InterruptServiceRoutine_Static,
			  pCard_current,
			  &pDriver->m_SpinLock_IRQ,
			  pCard_current->m_IRQ.Vector,
			  (KIRQL)pCard_current->m_IRQ.Level,
			  (KIRQL)pDriver->m_IRQLevel_highest,
			  (KINTERRUPT_MODE)pCard_current->m_IRQ.flags,
			  TRUE,
			  pCard_current->m_IRQ.Affinity,
			  FALSE
			);
			if ( SUCCESS( Status ) )
			{
			//Enable back card interrupts.
			//
			//If IoConnectInterrupt would fail we would leave currentCard interrupts
			//disabled, so the interrupt could not be fired.
			//Other parts of the driver wont know that the interrupts are disabled,
			//so they would work normally. But of course no sound would come out as data
			//would never be transfered to or from the card.
				pCard_current->PLX_SetGlobalInteruptState( CARD_GLOBAL_INTERRUPT_ENABLE );
			}
		}
	}

//Now set the IRQ for the requested card.
	if ( SUCCESS( Status ) )
	{
		Status = IoConnectInterrupt(
		  &m_pIRQObject,
		  &driver_c::InterruptServiceRoutine_Static,
		  this,
		  &pDriver->m_SpinLock_IRQ,
		  m_IRQ.Vector,
		  (KIRQL)m_IRQ.Level,
		  (KIRQL)pDriver->m_IRQLevel_highest,
		  (KINTERRUPT_MODE)m_IRQ.flags,
		  TRUE,
		  m_IRQ.Affinity,
		  FALSE
		);
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

//*****************************************************************************
// ProcedureName:
//		DetachIRQ
//
// Explanation:
//		In this procedure we detach IRQ and DPC procedures from curent card.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
// Return:
//		Should return STATUS_SUCCESS or the error code that was returned by the system.
//
#pragma CODE_PAGED
void card_c::DetachIRQ
(
	void
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

//Detach ISR and DPC.
	if ( m_pIRQObject != NULL )
	{
		IoDisconnectInterrupt( m_pIRQObject );
		m_pIRQObject = NULL;
	}
	if ( m_DPCObject_wave.DeferredRoutine != NULL )
	{
		KeRemoveQueueDpc( &m_DPCObject_wave );
		m_DPCObject_wave.DeferredRoutine = NULL;
	}
	if ( m_DPCObject_midi_in.DeferredRoutine != NULL )
	{
		KeRemoveQueueDpc( &m_DPCObject_midi_in );
		m_DPCObject_midi_in.DeferredRoutine = NULL;
	}
	if ( m_DPCObject_midi_out.DeferredRoutine != NULL )
	{
		KeRemoveQueueDpc( &m_DPCObject_midi_out );
		m_DPCObject_midi_out.DeferredRoutine = NULL;
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( STATUS_SUCCESS );
#endif
	return;
}

//*****************************************************************************
// ProcedureName:
//		GetMasterCard
//
// Explanation:
//		Procedure return card that is a clock owner... if cards are synced
//		procedure will return the one that is the first in a chain.
//
// IRQ Level:
//		INTERRUPT_LEVEL
//
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
// Return:
//		Returns pointer to master card (it can never be NULL).
//
#pragma CODE_LOCKED
card_c* card_c::GetMasterCard()
{
	card_c*		pCard = this;
	while ( pCard->m_pCard_master != NULL )
	{
		pCard = pCard->m_pCard_master;
	}

	return pCard;
}

//*****************************************************************************
// ProcedureName:
//		GetLastSlaveCard
//
// Explanation:
//		Procedure return card that is a the last in the sync chain.
//
// IRQ Level:
//		INTERRUPT_LEVEL
//
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
// Return:
//		Returns pointer to slave card.
//		Pointer can be NULL if current card doesn't have any slave cards.
//
#pragma CODE_LOCKED
card_c* card_c::GetLastSlaveCard()
{
	card_c*		pCard;

	if ( m_pCard_slave == NULL )
	{
	//If current card doesn't have any slave cards, return NULL.
		pCard = NULL;
	}
	else
	{
	//Current card has a slave card.. get the last in a chain.
		pCard = this;
		while ( pCard->m_pCard_slave != NULL )
		{
			pCard = pCard->m_pCard_slave;
		}
	}

	return pCard;
}
