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
#include "driver_ioctl_gui.h"

#pragma DATA_LOCKED


#pragma CODE_LOCKED
NTSTATUS driver_c::DeviceDispatch_Control
(
	PDEVICE_OBJECT		pFunctionalDeviceObject,
	PIRP				pIrp
)
{
	NTSTATUS				Status = STATUS_SUCCESS;

//Initialize.
	PIO_STACK_LOCATION		pStack = IoGetCurrentIrpStackLocation( pIrp );

//Check the ioControlCode.
	ULONG					ioControlCode = pStack->Parameters.DeviceIoControl.IoControlCode;
	ULONG					function = 0x00000FFF & ( ioControlCode >> 2 );

	if ( function >= 0x800 )
	{
	//Get input/output buffer length.
		unsigned long		bufferLength_input = pStack->Parameters.DeviceIoControl.InputBufferLength;
		unsigned long		bufferLength_output = pStack->Parameters.DeviceIoControl.OutputBufferLength;

	//Check the ioControlCode.
		unsigned long		informationSize = 0;
		unsigned long		ioControlCode_processed = YES;

		switch ( ioControlCode )
		{
		//---------------------------------------------------------------------------------------------------------------------------------
		//ASIO
		case IOCTL_ASIO_COPY_OUTPUT_BUFFERS:
		{
		//Notice:
		//This MUST BE the first in the switch.
		//As ioCtrl codes values aren't one after another, switch sentence will be made from if sentences and
		//we need that CopyBuffers command be as fast as it can be.
			Status = pDriver->ASIO_CopyOutputBuffers( (unsigned long*)pIrp->AssociatedIrp.SystemBuffer, bufferLength_input, bufferLength_output, &informationSize );
			break;
		}
		case IOCTL_ASIO_GET_ENGINE_VERSION:
		{
			Status = pDriver->ASIO_EngineVersion( (unsigned long*)pIrp->AssociatedIrp.SystemBuffer, bufferLength_input, bufferLength_output, &informationSize );
			break;
		}
		case IOCTL_ASIO_GET_CLOSE_HANDLE:
		{
			Status = pDriver->ASIO_GetCloseHandle( (unsigned long*)pIrp->AssociatedIrp.SystemBuffer, bufferLength_input, bufferLength_output, &informationSize );
			break;
		}
		case IOCTL_ASIO_SET_PROTECTION_IRP:
		{
			Status = pDriver->ASIO_ProtectionIRP_Set( pIrp, (unsigned long*)pIrp->AssociatedIrp.SystemBuffer, bufferLength_input, bufferLength_output, &informationSize );
			if ( Status == STATUS_PENDING )
			{
			//If pending do nothing.
				return Status;
			}
			break;
		}
		case IOCTL_ASIO_REMOVE_PROTECTION_IRP:
		{
			Status = pDriver->ASIO_ProtectionIRP_Remove( (unsigned long*)pIrp->AssociatedIrp.SystemBuffer, bufferLength_input, bufferLength_output, &informationSize );
			break;
		}
		case IOCTL_ASIO_OPEN_CLOSE_CHANNELS:
		{
			Status = pDriver->ASIO_OpenCloseChannels( (void*)pIrp->AssociatedIrp.SystemBuffer, bufferLength_input, bufferLength_output, &informationSize );
			break;
		}
		case IOCTL_ASIO_LOCK_MEMORY:
		{
			Status = pDriver->ASIO_LockMemory( (void*)pIrp->AssociatedIrp.SystemBuffer, bufferLength_input, bufferLength_output, &informationSize );
			break;
		}
		case IOCTL_ASIO_UNLOCK_MEMORY:
		{
			Status = pDriver->ASIO_UnlockMemory( (unsigned long*)pIrp->AssociatedIrp.SystemBuffer, bufferLength_input, bufferLength_output, &informationSize );
			break;
		}
		case IOCTL_ASIO_SET_SWITCH_EVENT:
		{
			Status = pDriver->ASIO_SwitchEvent_Set( pIrp, (void*)pIrp->AssociatedIrp.SystemBuffer, bufferLength_input, bufferLength_output, &informationSize );
			break;
		}
		case IOCTL_ASIO_RELEASE_SWITCH_EVENT:
		{
			Status = pDriver->ASIO_SwitchEvent_Release( (unsigned long*)pIrp->AssociatedIrp.SystemBuffer, bufferLength_input, bufferLength_output, &informationSize );
			break;
		}
		case IOCTL_ASIO_START:
		{
			Status = pDriver->ASIO_Start( (void*)pIrp->AssociatedIrp.SystemBuffer, bufferLength_input, bufferLength_output, &informationSize );
			break;
		}
		case IOCTL_ASIO_STOP:
		{
			Status = pDriver->ASIO_Stop( (unsigned long*)pIrp->AssociatedIrp.SystemBuffer, bufferLength_input, bufferLength_output, &informationSize );
			break;
		}

		//---------------------------------------------------------------------------------------------------------------------------------
		//DATA
		case IOCTL_DATA_GET_ENGINE_VERSION:
		{
			Status = pDriver->Data_EngineVersion( (unsigned long*)pIrp->AssociatedIrp.SystemBuffer, bufferLength_input, bufferLength_output, &informationSize );
			break;
		}
		case IOCTL_DATA_GET_CARDS_INFORMATION:
		{
			Status = pDriver->Data_CardsInformation( (void*)pIrp->AssociatedIrp.SystemBuffer, bufferLength_input, bufferLength_output, &informationSize );
			break;
		}
		case IOCTL_DATA_GET_SET_CHANNEL_PROPERTIES:
		{
			Status = pDriver->Data_ChannelProperty( (void*)pIrp->AssociatedIrp.SystemBuffer, bufferLength_input, bufferLength_output, &informationSize );
			break;
		}
		case IOCTL_DATA_GET_SET_BUFFER_SIZE:
		{
			Status = pDriver->Data_BufferSize( (unsigned long*)pIrp->AssociatedIrp.SystemBuffer, bufferLength_input, bufferLength_output, &informationSize );
			break;
		}
		case IOCTL_DATA_GET_SET_SAMPLE_RATE:
		{
			Status = pDriver->Data_SampleRate( (void*)pIrp->AssociatedIrp.SystemBuffer, bufferLength_input, bufferLength_output, &informationSize );
			break;
		}
		case IOCTL_DATA_SET_SYNC_CARDS:
		{
			Status = pDriver->Data_SyncCards( (unsigned long*)pIrp->AssociatedIrp.SystemBuffer, bufferLength_input, bufferLength_output );
			break;
		}

		//---------------------------------------------------------------------------------------------------------------------------------
		//GUI
		case IOCTL_GUI_GET_ENGINE_VERSION:
		{
			Status = pDriver->GUI_EngineVersion( (unsigned long*)pIrp->AssociatedIrp.SystemBuffer, bufferLength_input, bufferLength_output, &informationSize );
			break;
		}
		case IOCTL_GUI_SET_PROTECTION_IRP:
		{
			Status = pDriver->GUI_ProtectionIRP_Set( pIrp, bufferLength_input, bufferLength_output );
			if ( Status == STATUS_PENDING )
			{
			//If pending do nothing.
				return Status;
			}
			break;
		}
		case IOCTL_GUI_REMOVE_PROTECTION_IRP:
		{
			Status = pDriver->GUI_ProtectionIRP_Remove( bufferLength_input, bufferLength_output );
			break;
		}
		case IOCTL_GUI_SET_EVENT_NOTIFICATION:
		{
			Status = pDriver->GUI_Notification_Set( pIrp, (HANDLE*)pIrp->AssociatedIrp.SystemBuffer, bufferLength_input, bufferLength_output, &informationSize );
			break;
		}
		case IOCTL_GUI_RELEASE_EVENT_NOTIFICATION:
		{
			Status = pDriver->GUI_Notification_Release( bufferLength_input, bufferLength_output );
			break;
		}
		case IOCTL_GUI_GET_NOTIFICATION_EVENT:
		{
			Status = pDriver->GUI_Notification_GetEvent( (void*)pIrp->AssociatedIrp.SystemBuffer, bufferLength_input, bufferLength_output, &informationSize );
			break;
		}

		//---------------------------------------------------------------------------------------------------------------------------------
		//Not ours
		default:
		//This ioControlCode is not ours -> goto KS supplied DispatchProcedure.
			ioControlCode_processed = NO;
			break;
		}

	//It was our ioControlCode -> finish IRP and return.
		if ( ioControlCode_processed == YES )
		{
			pIrp->IoStatus.Information = informationSize;
			pIrp->IoStatus.Status = Status;
			IoCompleteRequest( pIrp, IO_NO_INCREMENT );

			return Status;
		}
	}

//If we came to here it's not for us.
	return pDriver->m_OriginalDriverDispatch_Control( pFunctionalDeviceObject, pIrp );
}