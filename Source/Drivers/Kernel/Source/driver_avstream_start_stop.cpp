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


//***************************************************************************
// ProcedureName:
//		Device_Start
//
// Explanation:
//		An AVStream minidriver's AVStrMiniDeviceStart routine. It is
//		called when an IRP_MN_START_DEVICE request is sent for a
//		specified device.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// Synhronization:
//		Device mutex is held by the system.
//
// --------------------------------------------------------------------------
// Device: IN
//		Pointer to a KSDEVICE structure describing the device to be started.
//
// Irp : IN
//		Pointer to the IRP_MN_START_DEVICE that was received.
//
// TranslatedResourceList: IN
//		Pointer to a CM_RESOURCE_LIST structure that contains
//		the translated resource list extracted from Irp.
//		Equals NULL if Device has no assigned resources.
//		This list contains the resources in translated form.
//		Use the translated resources to connect the interrupt vector,
//		map I/O space, and map memory.
//
// UntranslatedResourceList: IN
//		Pointer to a CM_RESOURCE_LIST structure that contains
//		the untranslated resource list extracted from Irp.
//		Equals NULL if the KSDEVICE member of this parameter list has
//		no assigned resources. This list contains the resources in raw form.
//		Use the raw resources to program the device.
//
// --------------------------------------------------------------------------
// Return:
//		Should return STATUS_SUCCESS or the error code that was returned
//		from the attempt to perform the operation.
//
#pragma CODE_PAGED
NTSTATUS driver_c::Device_Start
(
	IN PKSDEVICE			Device,
	IN PIRP					Irp,
	IN PCM_RESOURCE_LIST	TranslatedResourceList OPTIONAL,
	IN PCM_RESOURCE_LIST	UntranslatedResourceList OPTIONAL
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
#endif

	NTSTATUS			Status = STATUS_INTERNAL_ERROR;
	NTSTATUS			Status_registerWave = STATUS_INTERNAL_ERROR;
	NTSTATUS			Status_registerMidi = STATUS_INTERNAL_ERROR;

	unsigned long		serialNumber;
	card_c*				pCard = NULL;

//How many cards are in the system?
//Check if there is an empty place available.
	for ( serialNumber = 0; serialNumber < MAX_CARDS; serialNumber++ )
	{
		if ( pDriver->m_Cards[serialNumber] == NULL )
		{
		//Found an empty space.
			Status = STATUS_SUCCESS;
			break;
		}
	}

//Setup the card.
//
//Notice:
//If anything will go wrong the card class will be freed at the end of this function.
	if ( SUCCESS( Status ) )
	{
		pCard = new card_c( Device, serialNumber );
		if ( pCard != NULL )
		{
			Status = pCard->Initialize( TranslatedResourceList );
		}
		else
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
		}
	}

//Set filters for all available channels.
	if ( SUCCESS( Status ) )
	{
		Status = Status_registerWave = pDriver->Channels_RegisterToSystem_Wave( pCard );
	}
	if ( SUCCESS( Status ) )
	{
		Status = Status_registerMidi = pDriver->Channels_RegisterToSystem_Midi( pCard );
	}

//Register deviceIoControl interface.
	if ( SUCCESS( Status ) )
	{
		Status = pDriver->Interface_Register_DeviceIO( pCard );
	}

//Finish initialization.
	if ( SUCCESS( Status ) )
	{
	//Save pointer -> device mutex is held.
		pDriver->m_Cards[serialNumber] = pCard;

	//Card sync is not valid anymore.
		pDriver->m_Data_SyncChecked = 0;
	}

//Set system time resolution to 1ms for the timeStamping.
//
//Notice:
//Normally the system minimum resolution is around 1ms, but we'll set it to 0.5ms and let
//the system choose the smallest resolution that can handle.
	if ( SUCCESS( Status ) )
	{
		ULONG	resolution = ExSetTimerResolution( 5000, TRUE );
	#ifdef DBG_AND_LOGGER
		pLogger->Log( "System smallest timer resolution: %d * 100ns", resolution );
	#endif
	}

//Has initialization succeeded? If not free everything that was acquired.
	if ( FAILED( Status ) )
	{
		if ( pCard != NULL )
		{
		//Unregister filters.
			if ( SUCCESS( Status_registerWave ) )
			{
				pDriver->Channels_UnregisterFromSystem_Wave( pCard );
			}
			if ( SUCCESS( Status_registerMidi ) )
			{
				pDriver->Channels_UnregisterFromSystem_Midi( pCard );
			}

		//Free card.
			delete pCard;
		}
	}

#ifdef DBG_AND_LOGGER
	if ( SUCCESS( Status ) )
	{
		pLogger->Log(
		  logger_c::LOGGER_SEND_TO_LOG_FILE,
		  logger_c::LS_Driver_AVStream_Start,
		  pCard
		);
	}
	Procedure_Exit( Status );
#endif
	return Status;
}

//***************************************************************************
// ProcedureName:
//		Device_Stop
//
// Explanation:
//		An AVStream minidriver's AVStrMiniDeviceRemove routine.
//		It is called when an IRP_MN_STOP_DEVICE is dispatched by the device.
//
//		Driver is written in the manner that any card can be uninstalled at any time
//		and the other cards should be still functional.
//
//		Both ASIO driver and ControlPanel hold a handle to the driver so it can't
//		be unstalled until all handles are closed.
//
//		Now card synhronization is only possible if ControlPanel is running.
//		So if there is no CP, there is no synhronization.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// Synhronization:
//		Device mutex is held by the system.
//
// --------------------------------------------------------------------------
// Device: IN
//		Pointer to the KSDEVICE that dispatched the IRP_MN_REMOVE_DEVICE.
//
// Irp: IN
//		The IRP_MN_STOP_DEVICE issued by Device.
//
// --------------------------------------------------------------------------
// Return:
// 		None.
//
#pragma CODE_PAGED
void driver_c::Device_Stop
(
	IN PKSDEVICE		Device,
	IN PIRP				Irp
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
#endif

	NTSTATUS		Status;
	NTSTATUS		Status_locked = STATUS_UNSUCCESSFUL;

	card_c*			pCard = NULL;

//Get the card lock.
//
//Notice:
//We'll uninstall the card even if the lock will fail.
	Status_locked = pDriver->LockDriver();

//Remove card sync.
	pDriver->m_Data_SyncChecked = 0;

	//Remove all sync data from the cards.
	for ( long i = 0; i < ELEMENTS_IN_ARRAY( pDriver->m_Cards ); i++ )
	{
		card_c*		pCard_temp = pDriver->m_Cards[i];
		if ( pCard_temp != NULL )
		{
		//Remove sync.
			pCard_temp->m_pCard_master = NULL;
			pCard_temp->m_pCard_slave = NULL;
			pCard_temp->m_SerialNumber_masterCard = 0;
			pCard_temp->m_SerialNumber_slaveCard = 0;
		}
	}

//Get card.
	Status = STATUS_UNSUCCESSFUL;

	for ( long i = 0; i < ELEMENTS_IN_ARRAY( pDriver->m_Cards ); i++ )
	{
		pCard = pDriver->m_Cards[i];
		if ( pCard != NULL )
		{
		//Is this the card that wants to be released?
			if ( pCard->m_DeviceObject == Device )
			{
			//Yes.
				//Remove card from the list.
				pDriver->m_Cards[i] = NULL;

				//Return positive reply.
				Status = STATUS_SUCCESS;
				break;
			}
		}
	}

//Unregister deviceIO interface.
	if ( SUCCESS( Status ) )
	{
	//Unregister...
		pDriver->Interface_Unregister_DeviceIO( pCard );
	}

//Unregister filters.
	if ( SUCCESS( Status ) )
	{
		pDriver->Channels_UnregisterFromSystem_Midi( pCard );
		pDriver->Channels_UnregisterFromSystem_Wave( pCard );
	}

//Release the resources.
	//Release system time resolution.
	ULONG	resolution = ExSetTimerResolution( 0, FALSE );
#ifdef DBG_AND_LOGGER
	pLogger->Log( "System timer resolution: %d * 100ns\n", resolution );
#endif

	//Release card stuff.
	if ( SUCCESS( Status ) )
	{
		delete pCard;
	}

//Release lock.
	if ( SUCCESS( Status_locked ) )
	{
		pDriver->UnlockDriver();
	}

//Was this the last card uninstalled?
	bool	isLastCard = true;
	for ( int i = 0; i < ELEMENTS_IN_ARRAY( pDriver->m_Cards ); i++ )
	{
		if ( pDriver->m_Cards[i] != NULL )
		{
			isLastCard = false;
			break;
		}
	}
	if ( isLastCard == true )
	{
	//It was the last -> uninstall driver and logger.
		delete pDriver;
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );

	if ( isLastCard == true )
	{
		delete pLogger;
	}
#endif
	return;
}

