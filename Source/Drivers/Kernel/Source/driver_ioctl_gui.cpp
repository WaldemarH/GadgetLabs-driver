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
#include "driver_ioctl_gui.h"
#include "notify_event.h"


#pragma DATA_LOCKED


#pragma CODE_LOCKED
NTSTATUS driver_c::GUI_EngineVersion
(
	IN OUT unsigned long*		pSystemBuffer,
	IN unsigned long			bufferLength_input,
	IN unsigned long			bufferLength_output,
	IN OUT unsigned long*		pInformationSize
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
#endif

	NTSTATUS		Status = STATUS_SUCCESS;

//Validate.
	if (
	  ( bufferLength_input != 0 )
	  ||
	  ( bufferLength_output != sizeof( unsigned long ) )
	)
	{
		Status = STATUS_INVALID_BUFFER_SIZE;
	}

//Do the honky tonk.
	if ( SUCCESS( Status ) )
	{
		*pSystemBuffer = IOCTL_GUI_ENGINE_VERSION;
		*pInformationSize = sizeof( unsigned long );
	}

#ifdef GL_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

#pragma CODE_LOCKED
NTSTATUS driver_c::GUI_Notification_GetEvent
(
	IN OUT void*			pSystemBuffer,
	IN unsigned long		bufferLength_input,
	IN unsigned long		bufferLength_output,
	IN OUT unsigned long*	pInformationSize
)
{
//#ifdef DBG_AND_LOGGER
//	Procedure_Start_Global();
//#endif

	NTSTATUS		Status = STATUS_SUCCESS;

//Validate.
	if (
	  ( bufferLength_input != 0 )
	  ||
	  ( bufferLength_output != sizeof( ioctl_notify_event_s ) )
	)
	{
		Status = STATUS_INVALID_BUFFER_SIZE;
	}

//Do the honky tonk.
	if ( SUCCESS( Status ) )
	{
		KIRQL		spinLock_oldLevel;

		KeAcquireSpinLock( &m_GUI_notifyEvent_spinLock, &spinLock_oldLevel );

		//Get event data.
			if ( m_GUI_notifyEvent_master != NULL )
			{
				m_GUI_notifyEvent_master->GetEvent( (ioctl_notify_event_s*)pSystemBuffer );
			}

		KeReleaseSpinLock( &m_GUI_notifyEvent_spinLock, spinLock_oldLevel );

		*pInformationSize = sizeof( ioctl_notify_event_s );
	}

#ifdef GL_LOGGER
	pLogger->Log(
	  logger_c::LOGGER_SEND_TO_BOTH,
	  logger_c::LS_IOCTL_GUI_Notification_GetEvent,
	  pSystemBuffer
	);
	
	Procedure_Exit( Status );
#endif
	return Status;
}

#pragma CODE_LOCKED
NTSTATUS driver_c::GUI_Notification_Release
(
	IN unsigned long		bufferLength_input,
	IN unsigned long		bufferLength_output
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
#endif

	NTSTATUS		Status = STATUS_SUCCESS;

//Validate.
	if (
	  ( bufferLength_input != 0 )
	  ||
	  ( bufferLength_output != 0 )
	)
	{
		Status = STATUS_INVALID_BUFFER_SIZE;
	}

//Do the honky tonk.
	if ( SUCCESS( Status ) )
	{
	//Dereference the notifyEvent.
		KIRQL		spinLock_oldLevel;

		KeAcquireSpinLock( &m_GUI_notifyEvent_spinLock, &spinLock_oldLevel );

		//Dereference the old if it exist.
			if ( m_GUI_notifyEvent != NULL )
			{
				ObDereferenceObject( m_GUI_notifyEvent );
				m_GUI_notifyEvent = NULL;
			}

		//Free all waiting events.
			m_GUI_notifyEvent_master->FreeAll();

		KeReleaseSpinLock( &m_GUI_notifyEvent_spinLock, spinLock_oldLevel );
	}

#ifdef GL_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

#pragma CODE_LOCKED
NTSTATUS driver_c::GUI_Notification_Set
(
	IN PIRP					pIrp,
	IN HANDLE*				pHandle,
	IN unsigned long		bufferLength_input,
	IN unsigned long		bufferLength_output,
	IN OUT unsigned long*	pInformationSize
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
#endif

	NTSTATUS		Status = STATUS_SUCCESS;

//Validate.
	if (
	  ( bufferLength_input != sizeof( __int64 ) )
	  ||
	  ( bufferLength_output != 0 )
	)
	{
		Status = STATUS_INVALID_BUFFER_SIZE;
	}
	if ( SUCCESS( Status ) )
	{
		if ( *pHandle == NULL )
		{
			Status = STATUS_INVALID_PARAMETER;
		}
	}

//Do the honky tonk.
	PKEVENT		pEvent = NULL;

	if ( SUCCESS( Status ) )
	{
	//Create a link to GUI's common notification event.
		Status = ObReferenceObjectByHandle(
		  *pHandle,
		  EVENT_MODIFY_STATE,
		  *ExEventObjectType,
		  KernelMode,
		  (PVOID*)&pEvent,
		  NULL
		);
	}
	if ( SUCCESS( Status ) )
	{
	//Driver remembers only the last event, so if ControlPanel will be closed
	//in some non normal manner the old event will be substituted with the new one.
		KIRQL		spinLock_oldLevel;

		KeAcquireSpinLock( &m_GUI_notifyEvent_spinLock, &spinLock_oldLevel );

		//Dereference the old if it exist.
			if ( m_GUI_notifyEvent != NULL )
			{
				ObDereferenceObject( m_GUI_notifyEvent );
			}
			//Save the new events.
			m_GUI_notifyEvent = pEvent;

		//Free all old waiting events.
			m_GUI_notifyEvent_master->FreeAll();

		KeReleaseSpinLock( &m_GUI_notifyEvent_spinLock, spinLock_oldLevel );

		*pInformationSize = sizeof( ioctl_notify_event_s );
	}

#ifdef GL_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

#pragma CODE_LOCKED
NTSTATUS driver_c::GUI_ProtectionIRP_Remove
(
	IN unsigned long		bufferLength_input,
	IN unsigned long		bufferLength_output
)
{
	NTSTATUS		Status = STATUS_SUCCESS;

//Validate.
	if (
	  ( bufferLength_input != 0 )
	  ||
	  ( bufferLength_output != 0 )
	)
	{
		Status = STATUS_INVALID_BUFFER_SIZE;
	}

//Do the honky tonk.
	if ( SUCCESS( Status ) )
	{
	//Acquire the lock that guards pending IOCTL operations for this client.
		PIRP		pIrp;
		KIRQL		spinLock_oldLevel;

		KeAcquireSpinLock( &m_GUI_SpinLock_IRP_Pending, &spinLock_oldLevel );

			pIrp = m_GUI_IRP_Pending;
			if ( pIrp != NULL )
			{
			//Uncache the request. Once we claim the spin lock, it's not necessary to
			//further synchronize access to the pointer cell.
			//
			//Clear the cancel pointer for this IRP. Since both we and the
			//completion routine use a spin lock, it cannot happen that this
			//IRP pointer is suddenly invalid but the cache pointer cell
			//wasn't already NULL.
				if ( NULL != IoSetCancelRoutine( pIrp, NULL ) )
				{
				//Uncache the IRP
					RemoveEntryList( &pIrp->Tail.Overlay.ListEntry );

				//Clear the pointer.
					m_GUI_IRP_Pending = NULL;
				}
				else
				{
					Status = STATUS_UNSUCCESSFUL;
				}
			}
			else
			{
				Status = STATUS_UNSUCCESSFUL;
			}

		KeReleaseSpinLock( &m_GUI_SpinLock_IRP_Pending, spinLock_oldLevel );

	//Complete pending IRP.
		if ( SUCCESS( Status ) )
		{
			pIrp->IoStatus.Status = STATUS_SUCCESS;
			pIrp->IoStatus.Information = 0;
			IoCompleteRequest( pIrp, IO_NO_INCREMENT );
		}
	}

	return Status;
}

#pragma CODE_LOCKED
NTSTATUS driver_c::GUI_ProtectionIRP_Set
(
	IN PIRP					pIrp,
	IN unsigned long		bufferLength_input,
	IN unsigned long		bufferLength_output
)
{
	NTSTATUS		Status = STATUS_SUCCESS;

//Validate.
	if (
	  ( bufferLength_input != 0 )
	  ||
	  ( bufferLength_output != 0 )
	)
	{
		Status = STATUS_INVALID_BUFFER_SIZE;
	}
	if ( SUCCESS( Status ) )
	{
		if ( pIrp == NULL )
		{
			Status = STATUS_UNSUCCESSFUL;
		}
	}

//Do the honky tonk.
	if ( SUCCESS( Status ) )
	{
	//Acquire the lock that guards pending IOCTL operations for this client.
		KIRQL		spinLock_oldLevel;

		KeAcquireSpinLock( &m_GUI_SpinLock_IRP_Pending, &spinLock_oldLevel );

		//Set IRP to pending.
		//
		//Driver allows only 1 ControlPanel at a time.
			if ( m_GUI_IRP_Pending == NULL )
			{
			//Set variables.
				//pIrp->Tail.Overlay.DriverContext[0] = (PVOID)pIrp;
				//pIrp->Tail.Overlay.DriverContext[1] = (PVOID)pIrp;
				//pIrp->Tail.Overlay.DriverContext[2] = (PVOID)pIrp;
				//pIrp->Tail.Overlay.DriverContext[3] = (PVOID)pIrp;

			//Install a cancel routine and check for this IRP having already been cancelled.
				IoSetCancelRoutine( pIrp, driver_c::GUI_PendingIoctl_Cancel_Static );

				if ( pIrp->Cancel == TRUE )
				{
				//Remove cancel routine and check if it was already called?
					if ( NULL != IoSetCancelRoutine( pIrp, NULL ) )
					{
					//Not yet... then there is no point calling it.. just complete current IRP with STATUS_CANCELLED.
						Status = STATUS_CANCELLED;
					}
					else
					{
					//Already called.
					//
					//Note that our cancel routine, if running, will be waiting for the spin lock before completing the IRP.
					//So we will let the cancel routine to do it's job.
					//
					//So we need to act as if the IRP was pending and that cancel routine was called after we pended IRP.
						IoMarkIrpPending( pIrp );
						InsertTailList( &m_GUI_ListHead, &pIrp->Tail.Overlay.ListEntry );
						Status = STATUS_PENDING;

						m_GUI_IRP_Pending = pIrp;
					}
				}
				else
				{
				//Cancel was not called yet... set IRP pending.
					IoMarkIrpPending( pIrp );
					InsertTailList( &m_GUI_ListHead, &pIrp->Tail.Overlay.ListEntry );
					Status = STATUS_PENDING;

					m_GUI_IRP_Pending = pIrp;
				}
			}
			else
			{
				Status = STATUS_UNSUCCESSFUL;
			}

		KeReleaseSpinLock( &m_GUI_SpinLock_IRP_Pending, spinLock_oldLevel );
	}

	return Status;
}

//IRP pending.
#pragma CODE_LOCKED
VOID driver_c::GUI_PendingIoctl_Cancel_Static
(
	PDEVICE_OBJECT		pDeviceObject,
	PIRP				pIrp
)
{
	pDriver->GUI_PendingIoctl_Cancel( pIrp );
	return;
}
void driver_c::GUI_PendingIoctl_Cancel
(
	PIRP		pIrp
)
{
//Release CancelSpinLock.
	KIRQL		spinLock_oldLevel = pIrp->CancelIrql;

	//Don't allow any other process to come in between this procedure, except really high priority stuff.
	IoReleaseCancelSpinLock( DISPATCH_LEVEL );

//Remove the IRP from the pending IOCTL list.
//
//Also nullify the pointer cell that points to this IRP.
//The following weird possibility exists:
//1. Someone calls IoCancelIrp for this IRP.
//2. Someone calls UncacheControlRequest for this pointer cell, and it
//   gets to the point of calling IoSetCancelRoutine, whereupon it sees that
//   IoCancelIrp has already nulled the cancel pointer. It releases the spin
//   lock and returns NULL to its caller.
//3. Someone calls CacheControlRequest for this pointer cell. It acquires the
//   spin lock and sees that the pointer cell is NULL (from step 2), so it caches
//   a new IRP and releases the spin lock.
//4. We acquire the spin lock.
//At this point, we need to be careful how we nullify the pointer cell to avoid
//wiping out what happened in step 3. The hypothetical other IRP cannot be at 
//the same address as this one because we haven't completed this one yet.
	KeAcquireSpinLockAtDpcLevel( &m_GUI_SpinLock_IRP_Pending );

	//Remove the IRP from the pending list.
	//
	//If IRP has already been removed from the list the m_GUI_IRP_Pending will be NULL.
		if ( m_GUI_IRP_Pending != NULL )
		{
			RemoveEntryList( &pIrp->Tail.Overlay.ListEntry );

			m_GUI_IRP_Pending = NULL;
		}

	KeReleaseSpinLock( &m_GUI_SpinLock_IRP_Pending, spinLock_oldLevel );

//Clear the driver resources.
//
//Release event notification.
	KeAcquireSpinLock( &m_GUI_notifyEvent_spinLock, &spinLock_oldLevel );

	//Dereference the old if it exist.
		if ( m_GUI_notifyEvent != NULL )
		{
			ObDereferenceObject( m_GUI_notifyEvent );
			m_GUI_notifyEvent = NULL;
		}

	//Free all old waiting events.
		m_GUI_notifyEvent_master->FreeAll();

	KeReleaseSpinLock( &m_GUI_notifyEvent_spinLock, spinLock_oldLevel );

//As GUI has crashed the cards must be de-synced as uninstall procedure
//must be sure that no card are in sync when the system calls it.
//
//Notice:
//Next procedure will create havoc in sound that could be playing righ now...
//and as counters get reset the application will probably stop the channels as
//counters will be completely off.
//
//Notice2:
//Normally we would have to lock the driver to change sampleRate, but:
//1. driver locking is done with mutex which cannot run at current level
//2. slave card would never get sampleRate change request anyway
//   The only thing that slave card could get is desync request, but as GUI has crashed this wont happen.
//
//
//So... break the card sync.
//
	for ( int i = 0; i < ELEMENTS_IN_ARRAY(m_Cards); i++ )
	{
	//Find any card... the sync is always on all cards installed in the system.
		card_c*		pCard = m_Cards[i];
		if ( pCard == NULL )
		{
			continue;
		}

	//Card exist -> get it's master card.
		card_c*		pCard_master = pCard->GetMasterCard();

	//Remove slave relationship from the last to the front.
		card_c*		pCard_slave;
		
		while ( ( pCard_slave = pCard_master->GetLastSlaveCard() ) != NULL )
		{
		//Slave becomes master again.
			pCard_slave->m_pCard_master = NULL;

		//Change clockSource to local oscillator and set the sampleRate to same as currently held by the master card.
			pCard_slave->SetSampleRate_ForceDesync( (unsigned long)pCard_master->m_SampleRate );
		}

	//We don't have to check other cards as the sync is already broken.
		break;
	}

//Finish the cancelIrpRoutine with completing the IRP.
	pIrp->IoStatus.Status = STATUS_CANCELLED;
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );

	return;
}
