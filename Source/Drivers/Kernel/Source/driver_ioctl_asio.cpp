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
#include "driver_ioctl_asio.h"
#include "driver_ioctl_data.h"
#include "asio_handle.h"
#include "channel_base.h"


#pragma DATA_LOCKED

#pragma CODE_LOCKED
NTSTATUS driver_c::ASIO_CopyOutputBuffers(
	IN OUT unsigned long*		pSystemBuffer,
	IN unsigned long			bufferLength_input,
	IN unsigned long			bufferLength_output,
	IN OUT unsigned long*		pInformationSize
)
{
	NTSTATUS		Status = STATUS_SUCCESS;

//Validate.
	if (
	  ( bufferLength_input != sizeof( unsigned long ) )
	  ||
	  ( bufferLength_output != 0 )
	)
	{
		Status = STATUS_INVALID_BUFFER_SIZE;
	}

//Do the honky tonk.
	if ( SUCCESS( Status ) )
	{
		KIRQL		spinLock_oldLevel;

		KeAcquireSpinLock( &m_SpinLock_channel_wave, &spinLock_oldLevel );

		//Get ASIO handle.
			asioHandle_c*	pHandle = m_ASIO_Handle_master->GetHandle( *pSystemBuffer );
			if ( pHandle == NULL )
			{
				Status = STATUS_INVALID_PARAMETER;
			}

		//Transfer data.
			if ( SUCCESS( Status ) )
			{
				pHandle->TransferData_Output();
			}

		KeReleaseSpinLock( &m_SpinLock_channel_wave, spinLock_oldLevel );

		//*pInformationSize = 0;
	}

	return Status;
}

#pragma CODE_LOCKED
NTSTATUS driver_c::ASIO_EngineVersion
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
		*pSystemBuffer = IOCTL_ASIO_ENGINE_VERSION;
		*pInformationSize = sizeof( unsigned long );
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

#pragma CODE_LOCKED
NTSTATUS driver_c::ASIO_GetCloseHandle
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
	  ( bufferLength_input != sizeof( unsigned long ) )
	  ||
	  ( bufferLength_output != sizeof( unsigned long ) )
	)
	{
		Status = STATUS_INVALID_BUFFER_SIZE;
	}

//Do the honky tonk.
	if ( SUCCESS( Status ) )
	{
		KIRQL		spinLock_oldLevel;

		KeAcquireSpinLock( &m_SpinLock_channel_wave, &spinLock_oldLevel );

		//Create or free?
			if ( *pSystemBuffer == 0 )
			{
			//Create new ASIO handle.
				asioHandle_c*	pHandle = NULL;

				Status = m_ASIO_Handle_master->Handle_CreateNew( &pHandle );
				if ( SUCCESS( Status ) )
				{
					*pSystemBuffer = pHandle->GetHandleID();
				}
			}
			else
			{
			//Free ASIO handle.
				//Here we will ignore Status flag and close our eyes if something will go wrong.
				//All that can happen is that current handle is never freed... so we can loose some bytes of memory but that is it.
				m_ASIO_Handle_master->Handle_Remove( *pSystemBuffer );

				*pSystemBuffer = 0;
			}

		KeReleaseSpinLock( &m_SpinLock_channel_wave, spinLock_oldLevel );

		*pInformationSize = sizeof( unsigned long );
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

#pragma CODE_LOCKED
NTSTATUS driver_c::ASIO_LockMemory
(
	IN OUT void*				pSystemBuffer,
	IN unsigned long			bufferLength_input,
	IN unsigned long			bufferLength_output,
	IN OUT unsigned long*		pInformationSize
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
#endif

	NTSTATUS					Status = STATUS_SUCCESS;
	NTSTATUS					Status_locked = STATUS_SUCCESS;

	ioctl_asio_memory_s*		pData = (ioctl_asio_memory_s*)pSystemBuffer;

//Validate.
	if (
	  ( bufferLength_input != sizeof( ioctl_asio_memory_s ) )
	  ||
	  ( bufferLength_output != 0 )
	)
	{
		Status = STATUS_INVALID_BUFFER_SIZE;
	}
	if ( SUCCESS( Status ) )
	{
		if ( pData->m_Memory_Size >= ( PAGE_SIZE * (65535 - sizeof(MDL)) / sizeof(ULONG_PTR) ) )
		{
			Status = STATUS_INVALID_BUFFER_SIZE;
		}
	}

//Get ASIO handle and validate memory size.
	asioHandle_c*		pHandle;

	if ( SUCCESS( Status ) )
	{
		KIRQL				spinLock_oldLevel;

		KeAcquireSpinLock( &m_SpinLock_channel_wave, &spinLock_oldLevel );

		//Get ASIO handle.
			pHandle = m_ASIO_Handle_master->GetHandle( pData->m_hAsio );
			if ( pHandle == NULL )
			{
				Status = STATUS_INVALID_PARAMETER;
			}

		//Validate memory.
			if ( SUCCESS( Status ) )
			{
				Status = pHandle->Memory_ValidateSize( pData->m_Memory_Size );
			}

		KeReleaseSpinLock( &m_SpinLock_channel_wave, spinLock_oldLevel );
	}

//Lock memory.
	unsigned long*		pMemory_locked = NULL;
	_MDL*				pMDL = NULL;

	if ( SUCCESS( Status ) )
	{
	//Get MDL.
		pMDL = IoAllocateMdl(
		  (PVOID)pData->m_Memory_Address,
		  pData->m_Memory_Size,
		  FALSE,
		  FALSE,
		  NULL
		);
		if ( pMDL == NULL )
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
		}
	}
	if ( SUCCESS( Status ) )
	{
	//Lock memory.
		__try
		{
			MmProbeAndLockPages( pMDL, KernelMode, IoModifyAccess );
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
			Status = Status_locked = GetExceptionCode();
		}
	}
	if ( SUCCESS( Status ) )
	{
	//Map locked memory.
		pMemory_locked = (unsigned long*)MmGetSystemAddressForMdlSafe( pMDL, NormalPagePriority );
		if ( pMemory_locked == NULL )
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
		}
	}

//Save MDL and set channel buffers.
	if ( SUCCESS( Status ) )
	{
		KIRQL				spinLock_oldLevel;

	//Open/Close channels.
		KeAcquireSpinLock( &m_SpinLock_channel_wave, &spinLock_oldLevel );

		//Save MDL.
			pHandle->m_pMDL = pMDL;
			pHandle->m_pMemory_locked = pMemory_locked;

		//Set channels memory.
			pHandle->Channels_SetBuffers( (unsigned long*)( (ioctl_asio_switch*)pMemory_locked + 1 ) );

		KeReleaseSpinLock( &m_SpinLock_channel_wave, spinLock_oldLevel );

		//*pInformationSize = 0;
	}

//Was all ok?
	if ( FAILED( Status ) )
	{
		if ( pMDL != NULL )
		{
		//Unmap memory.
			if ( pMemory_locked != NULL )
			{
				MmUnmapLockedPages( pMemory_locked, pMDL );
				pMemory_locked = NULL;
			}

		//Unlock memory.
			if ( SUCCESS( Status_locked ) )
			{
				MmUnlockPages( pMDL );
			}

		//Free MDL.
			IoFreeMdl( pMDL );
			pMDL = NULL;
		}
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

#pragma CODE_LOCKED
NTSTATUS driver_c::ASIO_UnlockMemory
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

	NTSTATUS					Status = STATUS_SUCCESS;

	ioctl_asio_memory_s*		pData = (ioctl_asio_memory_s*)pSystemBuffer;

//Validate.
	if (
	  ( bufferLength_input != sizeof( unsigned long ) )
	  ||
	  ( bufferLength_output != 0 )
	)
	{
		Status = STATUS_INVALID_BUFFER_SIZE;
	}

//Get ASIO handle.
	if ( SUCCESS( Status ) )
	{
		KIRQL				spinLock_oldLevel;

		KeAcquireSpinLock( &m_SpinLock_channel_wave, &spinLock_oldLevel );

			asioHandle_c*	pHandle = m_ASIO_Handle_master->GetHandle( pData->m_hAsio );
			if ( pHandle == NULL )
			{
				Status = STATUS_INVALID_PARAMETER;
			}

		//Unlock memory.
			if ( SUCCESS( Status ) )
			{
				if ( pHandle->m_pMDL != NULL )
				{
				//Unmap locked memory.
					if ( pHandle->m_pMemory_locked != NULL )
					{
						MmUnmapLockedPages( pHandle->m_pMemory_locked, pHandle->m_pMDL );
						pHandle->m_pMemory_locked = NULL;
					}

				//Unlock memory.
					MmUnlockPages( pHandle->m_pMDL );

				//Free MDL.
					IoFreeMdl( pHandle->m_pMDL );
					pHandle->m_pMDL = NULL;
				}
			}

		KeReleaseSpinLock( &m_SpinLock_channel_wave, spinLock_oldLevel );
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

#pragma CODE_LOCKED
NTSTATUS driver_c::ASIO_OpenCloseChannels
(
	IN OUT void*				pSystemBuffer,
	IN unsigned long			bufferLength_input,
	IN unsigned long			bufferLength_output,
	IN OUT unsigned long*		pInformationSize
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
#endif

	NTSTATUS							Status = STATUS_SUCCESS;
	NTSTATUS							Status_locked = STATUS_UNSUCCESSFUL;

	ioctl_asio_channels_header_s*		pHeader = (ioctl_asio_channels_header_s*)pSystemBuffer;

//Validate.
	if (
	  ( bufferLength_input != sizeof( ioctl_asio_channels_header_s ) )
	  ||
	  ( bufferLength_output != 0 )
	)
	{
		Status = STATUS_INVALID_BUFFER_SIZE;
	}

	if ( SUCCESS( Status ) )
	{
	//Lock driver.
		Status = Status_locked = LockDriver();
	}

	if ( SUCCESS( Status ) )
	{
	//Validate that cards are present.
		unsigned long	nCardsUsed = 0;

		for ( int i = 0; i < ELEMENTS_IN_ARRAY( pHeader->m_Channels_Opened ); i++ )
		{
			if ( pHeader->m_Channels_Opened[i].m_Type != (unsigned char)ioctl_cardInfo_s::CARD_TYPE_NOT_SET )
			{
				if ( pHeader->m_Channels_Opened[i].m_SerialNumber < MAX_CARDS )
				{
					card_c*		pCard = m_Cards[pHeader->m_Channels_Opened[i].m_SerialNumber];
					if ( pCard != NULL )
					{
						if ( (unsigned char)pCard->m_Type == pHeader->m_Channels_Opened[i].m_Type )
						{
						//All ok...
							++nCardsUsed;
						}
						else
						{
						//Types are not the same.
							Status = STATUS_UNSUCCESSFUL;
							break;
						}
					}
					else
					{
					//Card doesn't exist.
						Status = STATUS_UNSUCCESSFUL;
						break;
					}
				}
				else
				{
				//Wrong serial number.
					Status = STATUS_UNSUCCESSFUL;
					break;
				}
			}
		}
	#ifdef DBG_AND_LOGGER
		pLogger->Log( "  Cards present status: %d\n", Status );
	#endif

		if ( SUCCESS( Status ) )
		{
			if ( nCardsUsed == 0 )
			{
				Status = STATUS_UNSUCCESSFUL;
			}
			else if ( nCardsUsed > 1 )
			{
			//If more then 1 card is used the cards must be synced.
				for ( int i = 0; i < ELEMENTS_IN_ARRAY( pHeader->m_Channels_Opened ); i++ )
				{
					if ( pHeader->m_Channels_Opened[i].m_Type != (unsigned char)ioctl_cardInfo_s::CARD_TYPE_NOT_SET )
					{
						card_c*		pCard = m_Cards[pHeader->m_Channels_Opened[i].m_SerialNumber];

						if (
						  ( pCard->m_pCard_master == NULL )
						  &&
						  ( pCard->m_pCard_slave == NULL )
						)
						{
						//Current card is not synced.
							Status = STATUS_UNSUCCESSFUL;
							break;
						}
					}
				}
			#ifdef DBG_AND_LOGGER
				pLogger->Log( "  Are cards synced status: %d\n", Status );
			#endif
			}
		}
	}

//Do the honky tonk.
	if ( SUCCESS( Status ) )
	{
		KIRQL				spinLock_oldLevel;

	//Open/Close channels.
		KeAcquireSpinLock( &m_SpinLock_channel_wave, &spinLock_oldLevel );

		//Get ASIO handle.
			asioHandle_c*	pHandle = m_ASIO_Handle_master->GetHandle( pHeader->m_hAsio );
			if ( pHandle == NULL )
			{
				Status = STATUS_INVALID_PARAMETER;
			}

		//Open/Close?
			if ( SUCCESS( Status ) )
			{
				if ( pHeader->m_Request == ASIO_CHANNELS_OPEN )
				{
				//Open.
					Status = pHandle->Channels_OpenAll( pHeader );
				}
				else
				{
				//Close.
					pHandle->Channels_CloseAll();
				}
			}

		KeReleaseSpinLock( &m_SpinLock_channel_wave, spinLock_oldLevel );

		//*pInformationSize = 0;
	}

	if ( SUCCESS( Status_locked ) )
	{
		UnlockDriver();
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

#pragma CODE_LOCKED
NTSTATUS driver_c::ASIO_ProtectionIRP_Remove
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
	  ( bufferLength_input != sizeof( unsigned long ) )
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

		KeAcquireSpinLock( &m_SpinLock_channel_wave, &spinLock_oldLevel );

		//Get ASIO handle and complete pending IRP.
			asioHandle_c*		pHandle = m_ASIO_Handle_master->GetHandle( *pSystemBuffer );
			if ( pHandle != NULL )
			{
				pIrp = pHandle->m_pIRP_pending;
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
						pHandle->m_pIRP_pending = NULL;
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
			}
			else
			{
			//We failed to get ASIO handle... now we'll fail the call, but as closing of the ASIO application will
			//call the pending IRP this isn't such a terible thing.
				Status = STATUS_INVALID_PARAMETER;
			}

		KeReleaseSpinLock( &m_SpinLock_channel_wave, spinLock_oldLevel );

	//Complete pending IRP.
		if ( SUCCESS( Status ) )
		{
			pIrp->IoStatus.Status = STATUS_SUCCESS;
			pIrp->IoStatus.Information = 0;
			IoCompleteRequest( pIrp, IO_NO_INCREMENT );
		}

		//*pInformationSize = 0;
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

#pragma CODE_LOCKED
NTSTATUS driver_c::ASIO_ProtectionIRP_Set
(
	IN PIRP						pIrp,
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
	  ( bufferLength_input != sizeof( unsigned long ) )
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

		KeAcquireSpinLock( &m_SpinLock_channel_wave, &spinLock_oldLevel );

		//Get ASIO handle and set IRP to pending.
			asioHandle_c*		pHandle = m_ASIO_Handle_master->GetHandle( *pSystemBuffer );
			if ( pHandle != NULL )
			{
				if ( pHandle->m_pIRP_pending == NULL )
				{
				//Set variables.
					pIrp->Tail.Overlay.DriverContext[0] = (PVOID)pHandle->GetHandleID();
					//pIrp->Tail.Overlay.DriverContext[1] = (PVOID)pIrp;
					//pIrp->Tail.Overlay.DriverContext[2] = (PVOID)pIrp;
					//pIrp->Tail.Overlay.DriverContext[3] = (PVOID)pIrp;

				//Install a cancel routine and check for this IRP having already been cancelled.
					IoSetCancelRoutine( pIrp, driver_c::ASIO_PendingIoctl_Cancel_Static );

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
							InsertTailList( &m_ASIO_ListHead, &pIrp->Tail.Overlay.ListEntry );
							Status = STATUS_PENDING;

							pHandle->m_pIRP_pending = pIrp;
						}
					}
					else
					{
					//Cancel was not called yet... set IRP pending.
						IoMarkIrpPending( pIrp );
						InsertTailList( &m_ASIO_ListHead, &pIrp->Tail.Overlay.ListEntry );
						Status = STATUS_PENDING;

						pHandle->m_pIRP_pending = pIrp;
					}
				}
				else
				{
					Status = STATUS_UNSUCCESSFUL;
				}
			}
			else
			{
				Status = STATUS_INVALID_PARAMETER;
			}

		KeReleaseSpinLock( &m_SpinLock_channel_wave, spinLock_oldLevel );

		//*pInformationSize = 0;
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

#pragma CODE_LOCKED
NTSTATUS driver_c::ASIO_Start
(
	IN OUT void*			pSystemBuffer,
	IN unsigned long		bufferLength_input,
	IN unsigned long		bufferLength_output,
	IN OUT unsigned long*	pInformationSize
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
#endif

	NTSTATUS						Status = STATUS_SUCCESS;

	ioctl_asio_start_header_s*		pHeader = (ioctl_asio_start_header_s*)pSystemBuffer;

//Validate.
	if (
	  ( bufferLength_input != sizeof( ioctl_asio_start_header_s ) )
	  ||
	  ( bufferLength_output != 0 )
	)
	{
		Status = STATUS_INVALID_BUFFER_SIZE;
	}

	if ( SUCCESS( Status ) )
	{
	//Validate that cards are present.
		unsigned long	nCardsUsed = 0;

		for ( int i = 0; i < ELEMENTS_IN_ARRAY( pHeader->m_Channels_Started ); i++ )
		{
			if ( pHeader->m_Channels_Started[i].m_Type != (unsigned char)ioctl_cardInfo_s::CARD_TYPE_NOT_SET )
			{
				if ( pHeader->m_Channels_Started[i].m_SerialNumber < MAX_CARDS )
				{
					card_c*		pCard = m_Cards[pHeader->m_Channels_Started[i].m_SerialNumber];
					if ( pCard != NULL )
					{
						if ( (unsigned char)pCard->m_Type == pHeader->m_Channels_Started[i].m_Type )
						{
						//All ok...
							++nCardsUsed;
						}
						else
						{
						//Types are not the same.
							Status = STATUS_UNSUCCESSFUL;
							break;
						}
					}
					else
					{
					//Card doesn't exist.
						Status = STATUS_UNSUCCESSFUL;
						break;
					}
				}
				else
				{
				//Wrong serial number.
					Status = STATUS_UNSUCCESSFUL;
					break;
				}
			}
		}
	#ifdef DBG_AND_LOGGER
		pLogger->Log( "  Are cards present status: %d\n", Status );
	#endif

		if ( SUCCESS( Status ) )
		{
			if ( nCardsUsed == 0 )
			{
				Status = STATUS_UNSUCCESSFUL;
			}
			else if ( nCardsUsed > 1 )
			{
			//If more then 1 card is used the cards must be synced.
				for ( int i = 0; i < ELEMENTS_IN_ARRAY( pHeader->m_Channels_Started ); i++ )
				{
					if ( pHeader->m_Channels_Started[i].m_Type != (unsigned char)ioctl_cardInfo_s::CARD_TYPE_NOT_SET )
					{
						card_c*		pCard = m_Cards[pHeader->m_Channels_Started[i].m_SerialNumber];
						if ( pCard != NULL )
						{
							if (
							  ( pCard->m_pCard_master == NULL )
							  &&
							  ( pCard->m_pCard_slave == NULL )
							)
							{
							//Current card is not synced.
								Status = STATUS_UNSUCCESSFUL;
								break;
							}
						}
						else
						{
						//Failed to get the card... input data is faulty.
							Status = STATUS_UNSUCCESSFUL;
							break;
						}
					}
				#ifdef DBG_AND_LOGGER
					pLogger->Log( "  Are cards synced status: %d\n", Status );
				#endif
				}
			}
		}
	}
#ifdef DBG_AND_LOGGER
	pLogger->Log( "  Validation Status: %d\n", Status );
#endif

//Do the honky tonk.
	if ( SUCCESS( Status ) )
	{
		KIRQL				spinLock_oldLevel;

	//Open/Close channels.
		KeAcquireSpinLock( &m_SpinLock_channel_wave, &spinLock_oldLevel );

		//Get ASIO handle.
			asioHandle_c*	pHandle = m_ASIO_Handle_master->GetHandle( pHeader->m_hAsio );
			if ( pHandle == NULL )
			{
				Status = STATUS_INVALID_PARAMETER;
			}

		//Start...
			if ( SUCCESS( Status ) )
			{
				Status = pHandle->Channels_Start( pHeader );
			}

		KeReleaseSpinLock( &m_SpinLock_channel_wave, spinLock_oldLevel );

	#ifdef DBG_AND_LOGGER
		for ( int i = 0; i < ELEMENTS_IN_ARRAY(m_Cards); i++ )
		{
			card_c*		pCard = m_Cards[i];
			if ( pCard != NULL )
			{
				pLogger->Log( "  Card(%d; 0x%p): pMasterCard = 0x%p, pSlaveCard = 0x%p\n", i, pCard, pCard->m_pCard_master, pCard->m_pCard_slave );
			}
			else
			{
				pLogger->Log( "  Card(%d; 0x%p): not present\n", i, NULL );
			}
		}

		pLogger->Log(
		  logger_c::LOGGER_SEND_TO_LOG_FILE,
		  logger_c::LS_IOCTL_ASIO_Start,
		  &pHandle->m_HandleID,
		  &pHandle->m_pChannels_Stereo_INs,
		  &pHandle->m_pChannels_Stereo_OUTs
		);
	#endif

		//*pInformationSize = 0;
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}
#pragma CODE_LOCKED
NTSTATUS driver_c::ASIO_Stop
(
	IN OUT unsigned long*	pSystemBuffer,
	IN unsigned long		bufferLength_input,
	IN unsigned long		bufferLength_output,
	IN OUT unsigned long*	pInformationSize
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
#endif

	NTSTATUS				Status = STATUS_SUCCESS;

//Validate.
	if (
	  ( bufferLength_input != sizeof( unsigned long ) )
	  ||
	  ( bufferLength_output != 0 )
	)
	{
		Status = STATUS_INVALID_BUFFER_SIZE;
	}

//Do the honky tonk.
	if ( SUCCESS( Status ) )
	{
		KIRQL				spinLock_oldLevel;

	//Stop channels.
		KeAcquireSpinLock( &m_SpinLock_channel_wave, &spinLock_oldLevel );

		//Get ASIO handle.
			asioHandle_c*	pHandle = m_ASIO_Handle_master->GetHandle( *pSystemBuffer );
			if ( pHandle == NULL )
			{
				Status = STATUS_INVALID_PARAMETER;
			}

		//Stop.
			if ( SUCCESS( Status ) )
			{
				pHandle->Channels_Stop();
			}

		KeReleaseSpinLock( &m_SpinLock_channel_wave, spinLock_oldLevel );

	//#ifdef GL_LOGGER
	//	pLogger->LogFast_Save();
	//#endif

		//*pInformationSize = 0;
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

#pragma CODE_LOCKED
NTSTATUS driver_c::ASIO_SwitchEvent_Release
(
	IN OUT unsigned long*	pSystemBuffer,
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
	  ( bufferLength_input != sizeof( unsigned long ) )
	  ||
	  ( bufferLength_output != 0 )
	)
	{
		Status = STATUS_INVALID_BUFFER_SIZE;
	}

//Do the honky tonk.
	if ( SUCCESS( Status ) )
	{
	//Dereference the switch event.
		KIRQL		spinLock_oldLevel;

		KeAcquireSpinLock( &m_SpinLock_channel_wave, &spinLock_oldLevel );

		//Get ASIO handle.
			asioHandle_c*	pHandle = m_ASIO_Handle_master->GetHandle( *pSystemBuffer );
			if ( pHandle == NULL )
			{
				Status = STATUS_INVALID_PARAMETER;
			}

		//Dereference the old if it exist.
			if ( SUCCESS( Status ) )
			{
				if ( pHandle->m_pSwitchEvent != NULL )
				{
					ObDereferenceObject( pHandle->m_pSwitchEvent );
					pHandle->m_pSwitchEvent = NULL;
				}
			}

		KeReleaseSpinLock( &m_SpinLock_channel_wave, spinLock_oldLevel );
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

#pragma CODE_LOCKED
NTSTATUS driver_c::ASIO_SwitchEvent_Set
(
	IN PIRP					pIrp,
	IN void*				pSystemBuffer,
	IN unsigned long		bufferLength_input,
	IN unsigned long		bufferLength_output,
	IN OUT unsigned long*	pInformationSize
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
#endif

	NTSTATUS						Status = STATUS_SUCCESS;

	ioctl_asio_switch_event*		pData= (ioctl_asio_switch_event*)pSystemBuffer;

//Validate.
	if (
	  ( bufferLength_input != sizeof( ioctl_asio_switch_event ) )
	  ||
	  ( bufferLength_output != 0 )
	)
	{
		Status = STATUS_INVALID_BUFFER_SIZE;
	}
	if ( SUCCESS( Status ) )
	{
		if ( (HANDLE)pData->m_SwitchEvent == NULL )
		{
			Status = STATUS_INVALID_PARAMETER;
		}
	}

//Do the honky tonk.
	PKEVENT		pEvent = NULL;

	if ( SUCCESS( Status ) )
	{
	//Create a link to ASIO's switch event.
		Status = ObReferenceObjectByHandle(
		  (HANDLE)pData->m_SwitchEvent,
		  EVENT_MODIFY_STATE,
		  *ExEventObjectType,
		  KernelMode,
		  (PVOID*)&pEvent,
		  NULL
		);
	}
	if ( SUCCESS( Status ) )
	{
	//Save reference.
		KIRQL		spinLock_oldLevel;

		KeAcquireSpinLock( &m_SpinLock_channel_wave, &spinLock_oldLevel );

		//Get ASIO handle.
			asioHandle_c*	pHandle = m_ASIO_Handle_master->GetHandle( pData->m_hAsio );
			if ( pHandle == NULL )
			{
				Status = STATUS_INVALID_PARAMETER;
			}

		//Save event.
			if ( SUCCESS( Status ) )
			{
				if ( pHandle->m_pSwitchEvent == NULL )
				{
					pHandle->m_pSwitchEvent = pEvent;
				}
				else
				{
					Status = STATUS_UNSUCCESSFUL;

					ObDereferenceObject( pHandle->m_pSwitchEvent );
					pHandle->m_pSwitchEvent = NULL;
				}
			}

		KeReleaseSpinLock( &m_SpinLock_channel_wave, spinLock_oldLevel );
	}

//Was all ok?
	if ( FAILED( Status ) )
	{
		if ( pEvent != NULL )
		{
			ObDereferenceObject( pEvent );
			pEvent = NULL;
		}
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

//IRP pending.
#pragma CODE_LOCKED
VOID driver_c::ASIO_PendingIoctl_Cancel_Static
(
	PDEVICE_OBJECT		pDeviceObject,
	PIRP				pIrp
)
{
	pDriver->ASIO_PendingIoctl_Cancel( pIrp );
	return;
}
void driver_c::ASIO_PendingIoctl_Cancel
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
	asioHandle_c*		pHandle;

	KeAcquireSpinLockAtDpcLevel( &m_SpinLock_channel_wave );

	//Get ASIO handle and set IRP to pending.
		pHandle = m_ASIO_Handle_master->GetHandle( (unsigned long)pIrp->Tail.Overlay.DriverContext[0] );
		if ( pHandle != NULL )
		{
		//Remove the IRP from the pending list.
		//
		//If IRP has already been removed from the list the m_pIRP_pending will be NULL.
			if ( pHandle->m_pIRP_pending != NULL )
			{
				RemoveEntryList( &pHandle->m_pIRP_pending->Tail.Overlay.ListEntry );
				pHandle->m_pIRP_pending = NULL;
			}
		}
		//else
		//{
		//Now if this happens I don't know what to do... as it looks that handle was removed and ASIO crashed... so not much we can do.
		//}

	//Clear the driver resources.
	//
	//Release event notification.
		if ( pHandle != NULL )
		{
			pHandle->Channels_CloseAll();
		}

	//Dereference switch event.
		if ( pHandle->m_pSwitchEvent != NULL )
		{
			ObDereferenceObject( pHandle->m_pSwitchEvent );
			pHandle->m_pSwitchEvent = NULL;
		}

	//Unlock memory and free ASIO handle.
		if ( pHandle != NULL )
		{
		//Unlock buffer memory.
			if ( pHandle->m_pMDL != NULL )
			{
			//Unmap locked memory.
				if ( pHandle->m_pMemory_locked != NULL )
				{
					MmUnmapLockedPages( pHandle->m_pMemory_locked, pHandle->m_pMDL );
					pHandle->m_pMemory_locked = NULL;
				}

			//Unlock memory.
				MmUnlockPages( pHandle->m_pMDL );

			//Free MDL.
				IoFreeMdl( pHandle->m_pMDL );
				pHandle->m_pMDL = NULL;
			}

		//Remove handle.
			m_ASIO_Handle_master->Handle_Remove( pHandle->GetHandleID() );
			pHandle = NULL;
		}

	KeReleaseSpinLock( &m_SpinLock_channel_wave, spinLock_oldLevel );
}
