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
// PURPOSE:
//      File defines main card class.
//
//      It's intention is to group/handle all card operations.
//
// STATUS:
//      Unfinished.
//
//***************************************************************************
//***************************************************************************

#pragma once

//Includes...
#include "delay.h"
#include "channel_all.h"
#include "channel_stereo.h"
#include "channel_midi.h"
#include "port.h"
#include "driver.h"
#include "card_pll.h"


struct ioctl_channelProperty_s;

class card_c : public port_c, public delay_c
{
	friend class asioHandle_c;
	friend class driver_c;
	friend class logger_c;
	
//--------------------------------------------------------------------------
	public: card_c( IN PKSDEVICE DeviceObject, unsigned long serialNumber );
	public: ~card_c();

	public: NTSTATUS Initialize( IN PCM_RESOURCE_LIST TranslatedResourceList );

//--------------------------------------------------------------------------
//Class memory
	#define MEMORY_TAG_CARD_CLASS		'DRAC'	//tag: "CARD"

	public: static void* __cdecl operator new( size_t size );
	public: static void __cdecl operator delete( void* pMemory );

//--------------------------------------------------------------------------
//System resources.
	private: PKSDEVICE					m_DeviceObject;
	private: UNICODE_STRING				m_SymbolicLinkName;

//--------------------------------------------------------------------------
//Card type
	private: NTSTATUS SetCardType();

	private: enum CARD_TYPE
	{
		CARD_TYPE_NOT_SET = 0,
		CARD_TYPE_424,
		CARD_TYPE_496,
		CARD_TYPE_824
	};
	private: CARD_TYPE					m_Type;

	private: unsigned long				m_Type_subVersion;

//--------------------------------------------------------------------------
//Resources
	private: NTSTATUS AcquireIOResources( IN PCM_RESOURCE_LIST TranslatedResourceList );
	private: void ReleaseIOResources();

	private: NTSTATUS AttachIRQ( void );
	private: void DetachIRQ( void );

	private: PULONG						m_IoPlx;
	private: PUSHORT					m_IoWave;
	private: PUCHAR						m_IoUart;
	public: __forceinline PUSHORT Get_IoWave() { return m_IoWave; };
	public: __forceinline PUCHAR Get_IoUart() { return m_IoUart; };

	private: PULONG						m_Mem_Plx;
	private: PULONG						m_Mem_Analog;

	private: ULONG						m_Mem_Plx_length;
	private: ULONG						m_Mem_Analog_length;

	private: struct interrupt_s
	{
		ULONG			Level;
		ULONG			Vector;
		KAFFINITY		Affinity;
		USHORT			flags;
	};
	private: interrupt_s				m_IRQ;
	private: PKINTERRUPT				m_pIRQObject;
	private: KDPC						m_DPCObject_wave;
	private: KDPC						m_DPCObject_midi_in;
	private: KDPC						m_DPCObject_midi_out;
	private: KTIMER						m_Timer_midi;
	private: KSPIN_LOCK					m_SpinLock_channel_midi_in;			//splinLock for every channel... MIDI can run separately on any CPU
	private: KSPIN_LOCK					m_SpinLock_channel_midi_out;		//splinLock for every channel... MIDI can run separately on any CPU

//--------------------------------------------------------------------------
//Sample Rate
//
//Notice:
//Any access to sampleRate variables must be protected with driver mutex pDriver->LockDriver().
	private: enum CLOCK_SOURCE
	{
		CLOCK_SOURCE_NOT_SET = 0,
		CLOCK_SOURCE_CARD,			//Local card clock.
		CLOCK_SOURCE_EXTERN			//Card is synced with the syncCable.
	};

	NTSTATUS AddressPointers_Reset();
	NTSTATUS CanMasterAndSlaveCardsHandleRisedSampleRate();
	NTSTATUS SetSampleRate( IN unsigned long sampleRate_new );
	NTSTATUS SetSampleRate_ForceDesync( IN unsigned long sampleRate_new );
	void SetSampleRate_ToHardware( IN WORD data, IN const pll_header_s* p4xx_Header, IN const pll_registerData_s* p4xx_RegisterData );
	NTSTATUS SetClockSource( IN CLOCK_SOURCE clockSourceNew );

	private: unsigned long				m_SampleRate;
	//Increase reference for every channel playing with current sample rate.
	private: unsigned long				m_SampleRate_owned;

	private: CLOCK_SOURCE				m_ClockSource;

//--------------------------------------------------------------------------
//Timestamp
	private: sampleClock_s				m_Clock_samples;
	private: sampleClock_s				m_Clock_samples_previous;	//lost buffer detector
	private: ULONGLONG					m_TimeStamp_wave;

	//private: sampleClock_s			m_Clock_midi;
	private: ULONGLONG					m_TimeStamp_midi;

	#define CLOCK_ZERO_TIMESTAMP		0x10000						//dont't make any different... it must be exactly like this!!!

//--------------------------------------------------------------------------
//Altera PLD handling.
	private: NTSTATUS Altera_LoadAndTestFirmware( void );
	private: NTSTATUS Altera_LoadFirmware( IN BYTE* chipFirmware );
	private: NTSTATUS Altera_MemoryTest_Analog( void );
	private: NTSTATUS Altera_MemoryTest( IN PULONG pMem_Card, IN PULONG pMem_Buffer, IN unsigned long bufferLength, IN unsigned long data );

	#define MAX_SC_PROG_BYTES			32385

	#define ALTERA_PROG_OUT				UART_MCR
	#define ALTERA_PROG_IN				UART_MSR

	#define DATA_CLK_OUT				UART_MCR_OUT1	//out
	#define DATA_BIT_OUT				UART_MCR_OUT2	//out
	#define NCONFIG_OUT					UART_MCR_DTR	//out

	#define CONFIG_DONE					UART_MSR_CTS	//in
	#define NSTATUS						UART_MSR_DCD	//in

//--------------------------------------------------------------------------
//4xx codecs
	private: void CODEC_Set4xxCodecs( void );
	private: void CODEC_WriteCodecsRegister( IN WORD codecSelect, IN WORD registerAddress, IN WORD registerData );

//--------------------------------------------------------------------------
//FS6377 PLL handling
	//void SetSampleRateToHardware( IN WORD data, IN pll_header_s* pHeader, IN pll_registerData_s* pRegisterData );
	private: NTSTATUS PLL_Set4xxPll( void );
	private: NTSTATUS PLL_LoadRegisters( IN pll_info_s* pPllInfo );
	private: NTSTATUS PLL_RegisterWrite_Random( IN const pll_header_s* pHeader, IN const pll_registerData_s* pRegisterData );

	#define SC4_PLL_ADDR				0x58	// I2C slave addr for FS6377 on 424/496

	static const pll_info_sampleRate_s	m_Pll_4xx_44100Hz;
	static const pll_info_sampleRate_s	m_Pll_4xx_48000Hz;
	static const pll_info_sampleRate_s	m_Pll_4xx_88200Hz;
	static const pll_info_sampleRate_s	m_Pll_4xx_96000Hz;

//--------------------------------------------------------------------------
//PLX integrated circuit handling
	private: void PLX_SetTransferFlags( void );

	public: enum CARD_GLOBAL_INTERRUPT
	{
		CARD_GLOBAL_INTERRUPT_ENABLE,
		CARD_GLOBAL_INTERRUPT_DISABLE
	};
	public: void PLX_SetGlobalInteruptState( IN CARD_GLOBAL_INTERRUPT State );

	private: enum CARD_LOCAL_BUS_ACCESS
	{
		CARD_LOCAL_BUS_ACCESS_ALLOWED,
		CARD_LOCAL_BUS_ACCESS_PROHIBITED
	};
	public: void PLX_SetLocalBusState( IN CARD_LOCAL_BUS_ACCESS State );

//--------------------------------------------------------------------------
//UART integrated circuit handling
	private: void UART_SetFlagsForMIDI( void );
	public: void UART_Reset_FIFO_Input( void );
	public: void UART_Reset_FIFO_Output( void );

//--------------------------------------------------------------------------
//Synhronization
//
//Master cards handle all the slave cards at the same time in DPC.
//This is made like this so that we wont have to wait for slave cards
//to copy their data.
//
//When the cards are synced in master<->slave relationship
//slave cards are started at exactly the same time as master card.
//So there is no worry that some data would be missing.
//
//This structure alows any card in the system to be a slave card
//(the card serial number doesn't matter).
//
	public: card_c* GetLastSlaveCard();
	public: card_c* GetMasterCard();

	private: card_c*					m_pCard_master;
	private: card_c*					m_pCard_slave;

	//Last two define the order of the sync cable (one based number).
	//If any of those is 0 than the card doesn't have
	//a master or slave card (it doesn't have the sync cable connection).
	//
	//NOTICE!!!
	//Slave cards are not in proper order... their order is the same as system presents it to the driver.
	//Why? Well it doesn't matter. The only important thing is which card is the master.
	//As all clock operations are only done on master card, slave cards only get their counters reset, then
	//master card sets back the clock.
	private: unsigned char				m_SerialNumber;					//zero based
	private: unsigned char				m_SerialNumber_masterCard;		//one based
	private: unsigned char				m_SerialNumber_slaveCard;		//one based
	private: unsigned char				m_align;
//--------------------------------------------------------------------------
//Streaming
//
//Notice:
//Only useful for master cards.
	enum CARDS_STREAMING
	{
		CARDS_STREAMING_STOP,
		CARDS_STREAMING_START
	};
	void Streaming_Midi_Start();
	void Streaming_Midi_Stop();

	NTSTATUS Streaming_Wave_Start();
	NTSTATUS Streaming_Wave_Stop();

	private: unsigned short				m_StopStream_midi;
	private: unsigned short				m_StopStream_wave;

//--------------------------------------------------------------------------
//Channels handling (Wave)
	private: NTSTATUS GetChannel_FromPin_Wave( IN PKSPIN pPin, OUT channel_base_c** ppChannel );
	private: BOOL IsChannelInUse_Wave();
	private: NTSTATUS IsChannelInUse_Wave( channel_base_c* pChannel );
	private: NTSTATUS SetChannels( void );

	private: void GetChannelProperty( IN ioctl_channelProperty_s* pChannel );
	private: void SetChannelProperty( IN ioctl_channelProperty_s* pChannel );

	private: channel_all_c*				m_pChannels_All_INs;
	private: channel_all_c*				m_pChannels_All_OUTs;

	private: channel_stereo_c*			m_pChannels_Stereo_INs[MAX_STEREO_PAIRS_PER_CARD+1];		//NULL defines the end
	private: channel_stereo_c*			m_pChannels_Stereo_OUTs[MAX_STEREO_PAIRS_PER_CARD+1];		//NULL defines the end

	private: unsigned char				m_Channels_opened_INs_ASIO;						//added for GUI
	private: unsigned char				m_Channels_opened_OUTs_ASIO;					//added for GUI
	private: unsigned char				m_Channels_opened_INs_AVStream;					//added for GUI
	private: unsigned char				m_Channels_opened_OUTs_AVStream;				//added for GUI

	private: unsigned long				m_Channels_nInUse_INs_ASIO;						//to speed up DPC processing
	private: unsigned long				m_Channels_nInUse_OUTs_ASIO;					//to speed up DPC processing
	private: unsigned long				m_Channels_nInUse_INs_AVStream;					//to speed up DPC processing
	private: unsigned long				m_Channels_nInUse_OUTs_AVStream;				//to speed up DPC processing

	private: PKSFILTERFACTORY			m_pFilterFactories_All;
	private: PKSFILTERFACTORY			m_pFilterFactories_Stereo[MAX_STEREO_PAIRS_PER_CARD];

	private: unsigned char				m_Property_4xxCards_Mute[4];					//CODEC doesn't support read operation so we'll save mute property like this

	//Buffer processing:
	//(OUT means the place where we should copy the output samples and
	// IN  means the place from where should we copy the input samples)
	//
	//ADC/DACs addresses are set to 0 at start(initialization).
	//This means that the buffers look like:
	//
	//  ---------------------------------
	//  |   |OUT|   |   |   |   |   |IN |
	//  ---------------------------------
	//  |
	//  |
	// start up ADC and DAC offset
	//
	//Now the card runs and in IRQ routine we will retrieve the offsets:
	//  ---------------------------------
	//  |   |IN |   |OUT|   |   |   |   |
	//  ---------------------------------
	//            |
	//            |
	//  current ADC and DAC offset
	//
	//
	//This shows us that DPC must copy new DAC samples to
	//the next buffer and retrieve the ADC samples from
	//previous buffer.
	//
	private: unsigned long				m_ChannelOffset;
	private: unsigned long				m_ChannelOffset_ASIO_out;

	private: unsigned long				m_ChannelOffset_In;		//temporary buffer used in DPC... protected by channel spinlock
	private: unsigned long				m_ChannelOffset_Out;	//temporary buffer used in DPC... protected by channel spinlock

//--------------------------------------------------------------------------
//Channels handling (Midi)
	private: NTSTATUS GetChannel_FromPin_Midi( IN PKSPIN pPin, OUT channel_midi_c** ppChannel );
	private: BOOL IsChannelInUse_Midi();
	private: NTSTATUS IsChannelInUse_Midi( channel_midi_c* pChannel );

	private: PKSFILTERFACTORY			m_pFilterFactories_Midi;

	private: channel_midi_c*			m_pChannels_Midi_INs;
	private: channel_midi_c*			m_pChannels_Midi_OUTs;
};
