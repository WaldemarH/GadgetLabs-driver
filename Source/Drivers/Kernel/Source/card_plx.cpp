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
//		plx.cpp
//
// PURPOSE:
//		File contains the card class PLX integrated circuit functions.
//
// STATUS:
//		Unfinished/Untested.
//
//***************************************************************************
//***************************************************************************
#include "main.h"
#include "card_altera_firmware.h"
#include "card_pci.h"
#include "card_plx.h"
#include "card_uart.h"

//***************************************************************************
// ProcedureName:
//		SetCardType
//
// Explanation:
//		Sets type of the card.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return:
//		Should return STATUS_SUCCESS, STATUS_UNSUCCESSFUL,
//		STATUS_INSUFFICIENT_RESOURCES or the error code that was returned
//		from the attempt to perform the operation.
//
#pragma CODE_PAGED
NTSTATUS card_c::SetCardType
(
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	NTSTATUS Status = STATUS_SUCCESS;

	unsigned long	ID_device;
	unsigned long	ID_subsystem;
	
//Get subSystemID.
	if (
	  ( 0 != KsDeviceGetBusData( m_DeviceObject, 0, &ID_device, PCI_VENDOR_DEVICE_ID, sizeof(ID_device) ) )
	  &&
	  ( 0 != KsDeviceGetBusData( m_DeviceObject, 0, &ID_subsystem, PCI_SUBSYSTEM_ID, sizeof(ID_subsystem) ) )
	)
	{
	//Validate both device and subsystem IDs before setting card type.
		switch ( ID_device )
		{
		case WAVE424_ID:
			if ( ID_subsystem == WAVE4_SUBSYS_ID_000 )
			{
				m_Type = CARD_TYPE_424;
				m_Type_subVersion = 1;
			}
			else
			{
				Status = STATUS_UNSUCCESSFUL;
			}
			break;

		case WAVE496_ID:
			if ( ID_subsystem == WAVE9_SUBSYS_ID_000 )
			{
				m_Type = CARD_TYPE_496;
				m_Type_subVersion = 1;
			}
			else
			{
				Status = STATUS_UNSUCCESSFUL;
			}
			break;

		case WAVE824_ID3:
		case WAVE824_ID4:
			switch ( ID_subsystem )
			{
			case WAVE8_SUBSYS_ID_002:		//824 ver. 2
				m_Type = CARD_TYPE_824;
				m_Type_subVersion = 2;
				break;
			case WAVE8_SUBSYS_ID_003:		//824 ver. 3
				m_Type = CARD_TYPE_824;
				m_Type_subVersion = 3;
				break;
			case WAVE8_SUBSYS_ID_004:		//824 ver. 4
				m_Type = CARD_TYPE_824;
				m_Type_subVersion = 4;
				break;
			default:
				Status = STATUS_UNSUCCESSFUL;
				break;
			}
			break;

		default:
			Status = STATUS_UNSUCCESSFUL;
			break;
		}
	}
	else
	{
		Status = STATUS_INSUFFICIENT_RESOURCES;
	}

#ifdef DBG_AND_LOGGER
	pLogger->Log(
	  logger_c::LOGGER_SEND_TO_LOG_FILE,
	  logger_c::LS_Card_SetCardType,
	  &m_Type,
	  &m_Type_subVersion
	);
	Procedure_Exit( Status );
#endif
	return Status;
}

//****************************************************************************
// ProcedureName:
//		AcquireResources
//
// Explanation:
//		Parse thru ResourceList and set class variables. Items are placed
//		in the ResourceList in the order they come out of the PLX chip.
//		That is PLX IO, SC IO and UART IO for IO. For memory it's:
//		PLX mem, daughter card mem and SC mem.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// pTranslatedResourceList: IN
//		Pointer to a CM_RESOURCE_LIST structure that contains
//		the translated resource list extracted from Irp.
//		Equals NULL if Device has no assigned resources.
//		This list contains the resources in translated form.
//		Use the translated resources to connect the interrupt vector,
//		map I/O space, and map memory.
//
// --------------------------------------------------------------------------
// Return:
//		Should return STATUS_SUCCESS or STATUS_UNSUCCESSFUL.
#pragma CODE_PAGED
NTSTATUS card_c::AcquireIOResources
(
	IN PCM_RESOURCE_LIST	pTranslatedResourceList
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	NTSTATUS		Status = STATUS_SUCCESS;

//Get the resources.
	if ( pTranslatedResourceList != NULL  )
	{
	//Get number of resources registered to the system.
		ULONG	count = pTranslatedResourceList->List[ 0 ].PartialResourceList.Count;

	//Did we get enough?
		if ( count >= 7 )
		{
			CM_PARTIAL_RESOURCE_DESCRIPTOR*		pPartialDescriptors = &( pTranslatedResourceList->List[ 0 ].PartialResourceList.PartialDescriptors[ 0 ] );

		//Parse through the resources.
			unsigned long		iPort = 0;		//port index
			unsigned long		iMemory = 0;	//memory index

			for ( ULONG i = 0; i < count; i++ )
			{
				switch ( pPartialDescriptors->Type )
				{
				case CmResourceTypePort:
				{
				//IoPlx  (length=0x80)
				//IoWave (length=0x40)
				//IoUart (length=0x08)
					switch ( pPartialDescriptors->u.Port.Length )
					{
					case PLX_IO_SIZE:
					//IoPlx -> should be the first IO space on card.
						if ( iPort == 0 )
						{
							m_IoPlx = (PULONG)pPartialDescriptors->u.Port.Start.LowPart;
						}
						else
						{
							Status = STATUS_UNSUCCESSFUL;
						}
						break;

					case SC_IO_PORT_SIZE:
					//IoWave -> should come in second.
						if ( iPort == 1 )
						{
							m_IoWave = (PUSHORT)pPartialDescriptors->u.Port.Start.LowPart;
						}
						else
						{
							Status = STATUS_UNSUCCESSFUL;
						}
						break;

					case UART_MAX_REGS:
					//IoUart -> should come in third.
						if ( iPort == 2 )
						{
							m_IoUart = (PUCHAR)pPartialDescriptors->u.Port.Start.LowPart;
						}
						else
						{
							Status = STATUS_UNSUCCESSFUL;
						}
						break;

					default:
						Status = STATUS_UNSUCCESSFUL;
						break;
					}
					++iPort;
					break;
				}
				case CmResourceTypeInterrupt :
				{
					m_IRQ.Level = pPartialDescriptors->u.Interrupt.Level;
					m_IRQ.Vector = pPartialDescriptors->u.Interrupt.Vector;
					m_IRQ.Affinity = pPartialDescriptors->u.Interrupt.Affinity;
					m_IRQ.flags = pPartialDescriptors->Flags;
					break;
				}
				case CmResourceTypeMemory :
				{
				//Mem_Plx     (length=0x80)    (PLX Mem)
				//Mem_Digital (length=0x10000) (SPDIF,ADAT Mem)
				//Mem_Analog  (length=0x20000) (ADC, DAC Mem)
					ULONG	length = pPartialDescriptors->u.Memory.Length;

					switch ( length )
					{
					case PLX_MEM_SIZE:
					//PLX memory -> should be the first one.
						if ( iMemory == 0 )
						{
							m_Mem_Plx = (PULONG) MmMapIoSpace(
							  pPartialDescriptors->u.Memory.Start,
							  length,
							  MmNonCached
							);
							m_Mem_Plx_length = length;
						}
						else
						{
							Status = STATUS_UNSUCCESSFUL;
						}
						break;

					case SC_ADAT_BUFFER_SIZE * sizeof( SAMPLE ):
					case SC_SPDIF_BUFFER_SIZE * sizeof( SAMPLE ):
					//Digital board memory window problem.
					//Only some cards support memory windows of 0x10000 in size.
					//This needs to be tested and fixed -> eprom change.
					//
					//Digital memory -> should come in second.
					//
					//NOTICE:
					//Digital cards will never be supported so we'll ignore this one.
						if ( iMemory != 1 )
						{
							Status = STATUS_UNSUCCESSFUL;
						}
						break;

					case SC_WAVE_BUFFER_SIZE * sizeof( SAMPLE ):
					//Analog memory -> should come in third.
						if ( iMemory == 2 )
						{
							m_Mem_Analog = (PULONG) MmMapIoSpace(
							  pPartialDescriptors->u.Memory.Start,
							  length,
							  MmNonCached
							);
							m_Mem_Analog_length = length;
						}
						else
						{
							Status = STATUS_UNSUCCESSFUL;
						}
						break;

					default:
						Status = STATUS_UNSUCCESSFUL;
						break;
					}
					++iMemory;
					break;
				}
				default:
					break;
				}

				//Check if current loop has failed.
				if ( Status == STATUS_UNSUCCESSFUL )
				{
					break;
				}

			//Update.
				++pPartialDescriptors;
			}
		}
		else
		{
		//Not enough resources.
			Status = STATUS_INSUFFICIENT_RESOURCES;
		}
	}
	else
	{
	//No TranslatedResourceList.
		Status = STATUS_INSUFFICIENT_RESOURCES;
	}

#ifdef DBG_AND_LOGGER
	pLogger->Log(
	  logger_c::LOGGER_SEND_TO_LOG_FILE,
	  logger_c::LS_Card_AcquireResources,
	  this
	);

	Procedure_Exit( Status );
#endif
	return Status;
}

//***************************************************************************
// ProcedureName:
//		ReleaseIOResources
//
// Explanation:
//		Procedure releases all acquired IO card resources. It checks what was
//		defined and releses it.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return:
//		void
//
#pragma CODE_PAGED
void card_c::ReleaseIOResources
(
	void
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

//Unmap IO Space.
	if ( m_Mem_Analog != NULL )
	{
		MmUnmapIoSpace( m_Mem_Analog, m_Mem_Analog_length );
		m_Mem_Analog = NULL;
	}
	if ( m_Mem_Plx != NULL )
	{
		MmUnmapIoSpace( m_Mem_Plx, m_Mem_Plx_length );
		m_Mem_Plx = NULL;
	}

//"Free" ports.
	m_IoUart = NULL;
	m_IoWave = NULL;
	m_IoPlx = NULL;

#ifdef DBG_AND_LOGGER
	Procedure_Exit( STATUS_SUCCESS );
#endif
	return;
}

//***************************************************************************
// ProcedureName:
//		PLX_SetGlobalInteruptState
//
// Explanation:
//		You can globally enable/disable interrupt on GL card.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// IN State:
//		Specify if global interrupt should be enabled or disabled.
//
// --------------------------------------------------------------------------
// Return:
//		void
//
#pragma CODE_PAGED
void card_c::PLX_SetGlobalInteruptState
(
	IN CARD_GLOBAL_INTERRUPT	State
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

//Enable/Disable global interrupt.
	DWORD	data = ReadDword( m_IoPlx, PLX_INT_CN_ST );

	if ( State == CARD_GLOBAL_INTERRUPT_ENABLE )
	{
	//Enable...
		data |= ( PLX_INT_CN_ST__INT1_EN | PLX_INT_CN_ST__PCI_INT_EN );
	}
	else
	{
	//Disable...
		data &= ~( PLX_INT_CN_ST__INT1_EN | PLX_INT_CN_ST__PCI_INT_EN );
	}

	WriteDword( m_IoPlx, PLX_INT_CN_ST, data );

#ifdef DBG_AND_LOGGER
	Procedure_Exit( STATUS_SUCCESS );
#endif
	return;
}

//***************************************************************************
// ProcedureName:
//		PLX_SetLocalBusState
//
// Explanation:
//		Enable/Disable access to local bus.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// IN State:
//		Specify if access to local bus is allowed or prohibited.
//
//		Notice:
//		Access to local bus while the the access is prohibited will
//		freeze the system.
//
// --------------------------------------------------------------------------
// Return:
//		void
//
#pragma CODE_PAGED
void card_c::PLX_SetLocalBusState
(
	IN CARD_LOCAL_BUS_ACCESS	State
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

//Enable/Disable global interrupt.
	DWORD	data = ReadDword( m_IoPlx, PLX_CNTR );

	if ( State == CARD_LOCAL_BUS_ACCESS_ALLOWED )
	{
	//Allowed...
	//
	//Clear local bus reset flag.
		data &= ~( PLX_CNTR__LBR );
	}
	else
	{
	//Prohibited...
	//
	//Set local bus reset flag.
		data |= ( PLX_CNTR__LBR );
	}

	WriteDword( m_IoPlx, PLX_CNTR, data );

#ifdef DBG_AND_LOGGER
	Procedure_Exit( STATUS_SUCCESS );
#endif
	return;
}

//***************************************************************************
// ProcedureName:
//		PLX_SetTransferFlags
//
// Explanation:
//		Some PCI to local bus flags need to be updated to get best performance
//		out of the cards.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return:
//		void
//
#pragma CODE_PAGED
void card_c::PLX_SetTransferFlags
(
	void
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

//Set PLX chip transfer modes.
//(I'm not completely sure but it looks that there is a small decrease in
//card local bus wait time with next settings.)
//
//  bit14 -> PCI Read Mode: 1
//  bit15 -> PCI Read with Flush Mode: 1
//  bit16 -> PCI Read No Flush Mode: 0
//  bit17 -> PCI Read No Write Mode: 1
//  bit18 -> PCI Write Mode: 1
//  bit22-19 -> PCI Target Retry Delay Clocks: 16
//              (this is not used, but we'll set it anyway)
//
//Notice:
//Linux team has determined that setting PLX_CNTR__READ_NO_FLUSH lowers the time system takes
//to transfer data. But unfortunately also risses the write->read delay from 200 to 221 PCI cycles.
	DWORD	data = ReadDword( m_IoPlx, PLX_CNTR );

	data |= PLX_CNTR__READ_MODE;
	data &= ~PLX_CNTR__READ_WITH_WRITE_FLUSH;
	data |= PLX_CNTR__READ_NO_FLUSH;
	data |= PLX_CNTR__READ_NO_WRITE;
	data |= PLX_CNTR__WRITE_MODE;
	data &= ~( 0x00780000 );
	data |= 0x00080000;							//cntrData |= 0x00080000;// = 1

	WriteDword( m_IoPlx, PLX_CNTR, data );

#ifdef DBG_AND_LOGGER
	Procedure_Exit( STATUS_SUCCESS );
#endif
	return;
}
