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
#include "card.h"
#include "driver_ioctl.h"
#include "driver_avstream_filter_interface.h"

#pragma DATA_LOCKED


//***************************************************************************
// ProcedureName:
//		DeviceDispatch_CreateClose
//
// Explanation:
//		Procedure will monitor if driver's private interface will be
//      opened/closed and it will handle those requests.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// DeviceObject: IN
//      Caller-supplied pointer to a DEVICE_OBJECT structure.
//      This is the device object for the target device, previously
//      created by the driver's AddDevice routine.
//
// Irp : IN
//      Caller-supplied pointer to an IRP structure that describes the
//      requested I/O operation. 
//
// --------------------------------------------------------------------------
// Return:
// 		If the routine succeeds, it must return STATUS_SUCCESS.
//		Otherwise, it must return one of the error status values
//		defined in ntstatus.h
//
#pragma CODE_LOCKED
NTSTATUS driver_c::DeviceDispatch_CreateClose
(
	PDEVICE_OBJECT		pFunctionalDeviceObject,
	PIRP				pIrp
)
{
//#ifdef DBG_AND_LOGGER
//	Procedure_Start_Global();
//#endif

	NTSTATUS				Status;

//Initialize.
	PIO_STACK_LOCATION		pStack = IoGetCurrentIrpStackLocation( pIrp );
//#ifdef GL_LOGGER
//	Logger_Print( LS_DeviceDispatch_Create, stack );
//#endif

//Is command for our driver?
	PFILE_OBJECT			pFileObject = pStack->FileObject;
	if ( pFileObject != NULL )
	{
	//We have file name -> compare if the string is L"\\gli dioc 4688 x".
	//
	//Notice:
	//sizeof counts trailing NUL character, but UNICODE_STRING doesn't, so we'll substract it.
		if ( pFileObject->FileName.Length == ( sizeof(DEVICE_IO_CONTROL_small_test) - sizeof(WCHAR) ) )
		{
		//We have file name -> compare if the string is "\gli dioc 4688 x".
		//Notice:
		//x is really a number, but we aren't interested in it, so we wont compare it.
			int		compare = wcsncmp(
			  (WCHAR*)pFileObject->FileName.Buffer,
			  DEVICE_IO_CONTROL_small_test,
			  ( sizeof(DEVICE_IO_CONTROL_small_test) - sizeof(WCHAR) - sizeof(WCHAR) ) / sizeof(WCHAR)
			);
			if ( compare == 0 )
			{
			//It's glDriver deviceIoControl interface -> finish IRP & return SUCCESS.
				Status = STATUS_SUCCESS;
				pIrp->IoStatus.Status = STATUS_SUCCESS;
				IoCompleteRequest( pIrp, IO_NO_INCREMENT );

				goto DDCreateClose_Exit;
			}
		}
	}

//Send call to lower level driver.
	if ( pStack->MajorFunction == IRP_MJ_CREATE )
	{
	//IRP_MJ_CREATE
		Status = pDriver->m_OriginalDriverDispatch_Create( pFunctionalDeviceObject, pIrp );
	}
	else
	{
	//IRP_MJ_CLOSE
		Status = pDriver->m_OriginalDriverDispatch_Close( pFunctionalDeviceObject, pIrp );
	}

DDCreateClose_Exit:
//#ifdef DBG_AND_LOGGER
//	Procedure_Exit( Status );
//#endif
	return Status;
}


//***************************************************************************
// ProcedureName:
//		Interface_Register_DeviceIO/Interface_Unregister_DeviceIO
//
// Explanation:
//		Procedure registers/unregisters deviceIoControl interface.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
// Return:
//		Should return STATUS_SUCCESS or the error code that was returned
//		from the attempt to perform the operation.
//
#pragma CODE_PAGED
NTSTATUS driver_c::Interface_Register_DeviceIO
(
	card_c*		pCard
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	NTSTATUS			Status = STATUS_SUCCESS;

//Get interface constants.
	PWCHAR				deviceIoControl_text = NULL;
	const GUID*			pGUID = NULL;

	switch ( pCard->m_SerialNumber )
	{
	case 0:
		deviceIoControl_text = DEVICE_IO_CONTROL_1;
		pGUID = &DEVICE_IO_GUID_1;
		break;
	case 1:
		deviceIoControl_text = DEVICE_IO_CONTROL_2;
		pGUID = &DEVICE_IO_GUID_2;
		break;
	case 2:
		deviceIoControl_text = DEVICE_IO_CONTROL_3;
		pGUID = &DEVICE_IO_GUID_3;
		break;
	case 3:
		deviceIoControl_text = DEVICE_IO_CONTROL_4;
		pGUID = &DEVICE_IO_GUID_4;
		break;
	}

//Set deviceIO interface was current card?
//
//As GUI/ASIO protect driver from uninstallation, GUI/ASIO will open handle to every card on its own,
//even if communication will run only through 1 interface only.
	//Convert PWCHAR to UNICODE_STRING.
	UNICODE_STRING		referenceName;

	RtlInitUnicodeString( &referenceName, deviceIoControl_text );

	//Register interface.
	Status = IoRegisterDeviceInterface(
		pCard->m_DeviceObject->PhysicalDeviceObject,
		pGUID,
		&referenceName,
		&pCard->m_SymbolicLinkName
	);

	//Set FriendlyName interface value.
	if ( SUCCESS( Status ) )
	{
	//Open interface registry handle.
		HANDLE				hKey_interface = NULL;

		Status = IoOpenDeviceInterfaceRegistryKey( &pCard->m_SymbolicLinkName, KEY_WRITE, &hKey_interface );

	//Set FriendlyName value.
		if ( SUCCESS( Status ) )
		{
			UNICODE_STRING		FriendlyName_USTR = { sizeof( FRIENDLY_NAME ), sizeof( FRIENDLY_NAME ), (PWCH)&FRIENDLY_NAME };

			Status = ZwSetValueKey(
				hKey_interface,
				&FriendlyName_USTR,
				0,
				REG_SZ,
				(PVOID)&DEVICE_IO_NAME,
				( (unsigned long)wcslen( DEVICE_IO_NAME ) )*sizeof( WCHAR )
			);
		}

	//Release resources
		ZwClose( hKey_interface );

	//Enable interface.
		if ( SUCCESS( Status ) )
		{
			Status = IoSetDeviceInterfaceState( &pCard->m_SymbolicLinkName, TRUE );
		}
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

#pragma CODE_PAGED
void driver_c::Interface_Unregister_DeviceIO
(
	card_c*		pCard 
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

//Unregister deviceIoControl interface.
	if ( pCard->m_SymbolicLinkName.Length > 0 )
	{
	#ifdef DBG_AND_LOGGER
		pLogger->Log( "Unregister deviceIoControl interface." );
	#endif

	//Disable interface.
		IoSetDeviceInterfaceState( &pCard->m_SymbolicLinkName, FALSE );

	//Release resources
		RtlFreeUnicodeString( &pCard->m_SymbolicLinkName );

		pCard->m_SymbolicLinkName.Length = 0;
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( STATUS_SUCCESS );
#endif
	return;
}
