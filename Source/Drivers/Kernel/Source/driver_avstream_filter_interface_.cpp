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
#include "driver_avstream_filter_interface.h"
#include "driver_avstream_filter_register.h"

#pragma DATA_PAGED

UNICODE_STRING	CLSID_USTR = { sizeof( sCLSID ), sizeof( sCLSID ), (PWCH)&sCLSID };
UNICODE_STRING	FriendlyName_USTR = { sizeof( FRIENDLY_NAME ), sizeof( FRIENDLY_NAME ), (PWCH)&FRIENDLY_NAME };
UNICODE_STRING	ChannelName_USTR = { sizeof( CHANNEL_NAME ), sizeof( CHANNEL_NAME ), (PWCH)&CHANNEL_NAME };


//***************************************************************************
// ProcedureName:
//		Interface_SetPinNames
//
// Explanation:
//		Procedure will replace name GUID in pinDescriptor and replacei it
//		drivers one. In this way user will get proper channel name.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// pPinDescriptor: IN
//		Pointer to pin descriptor copy.
//
// friendlyName: IN
//		The name of the pin that the user will see.
//
// --------------------------------------------------------------------------
// Return:
//		Should return STATUS_SUCCESS or the error code that was returned
//		from the attempt to perform the operation.
//
#pragma CODE_PAGED
NTSTATUS driver_c::Interface_SetPinNames
(
	IN PWCHAR			pFriendlyName,
	OUT const GUID**	ppGUID
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	NTSTATUS Status = STATUS_SUCCESS;

//Get next free GUID.
	const GUID*		pGUID = NULL;
	WCHAR*			pGUID_registry = NULL;

	switch ( m_Interface_guidNamesUsed )
	{
	case 0:
		pGUID = &GUID_AVSTREAM_NAME_1;
		pGUID_registry = GUID_AVSTREAM_NAME_1_REGISTRY;
		break;
	case 1:
		pGUID = &GUID_AVSTREAM_NAME_2;
		pGUID_registry = GUID_AVSTREAM_NAME_2_REGISTRY;
		break;
	case 2:
		pGUID = &GUID_AVSTREAM_NAME_3;
		pGUID_registry = GUID_AVSTREAM_NAME_3_REGISTRY;
		break;
	case 3:
		pGUID = &GUID_AVSTREAM_NAME_4;
		pGUID_registry = GUID_AVSTREAM_NAME_4_REGISTRY;
		break;
	case 4:
		pGUID = &GUID_AVSTREAM_NAME_5;
		pGUID_registry = GUID_AVSTREAM_NAME_5_REGISTRY;
		break;
	case 5:
		pGUID = &GUID_AVSTREAM_NAME_6;
		pGUID_registry = GUID_AVSTREAM_NAME_6_REGISTRY;
		break;
	case 6:
		pGUID = &GUID_AVSTREAM_NAME_7;
		pGUID_registry = GUID_AVSTREAM_NAME_7_REGISTRY;
		break;
	case 7:
		pGUID = &GUID_AVSTREAM_NAME_8;
		pGUID_registry = GUID_AVSTREAM_NAME_8_REGISTRY;
		break;
	case 8:
		pGUID = &GUID_AVSTREAM_NAME_9;
		pGUID_registry = GUID_AVSTREAM_NAME_9_REGISTRY;
		break;
	case 9:
		pGUID = &GUID_AVSTREAM_NAME_10;
		pGUID_registry = GUID_AVSTREAM_NAME_10_REGISTRY;
		break;
	case 10:
		pGUID = &GUID_AVSTREAM_NAME_11;
		pGUID_registry = GUID_AVSTREAM_NAME_11_REGISTRY;
		break;
	case 11:
		pGUID = &GUID_AVSTREAM_NAME_12;
		pGUID_registry = GUID_AVSTREAM_NAME_12_REGISTRY;
		break;
	case 12:
		pGUID = &GUID_AVSTREAM_NAME_13;
		pGUID_registry = GUID_AVSTREAM_NAME_13_REGISTRY;
		break;
	case 13:
		pGUID = &GUID_AVSTREAM_NAME_14;
		pGUID_registry = GUID_AVSTREAM_NAME_14_REGISTRY;
		break;
	case 14:
		pGUID = &GUID_AVSTREAM_NAME_15;
		pGUID_registry = GUID_AVSTREAM_NAME_15_REGISTRY;
		break;
	case 15:
		pGUID = &GUID_AVSTREAM_NAME_16;
		pGUID_registry = GUID_AVSTREAM_NAME_16_REGISTRY;
		break;
	case 16:
		pGUID = &GUID_AVSTREAM_NAME_17;
		pGUID_registry = GUID_AVSTREAM_NAME_17_REGISTRY;
		break;
	case 17:
		pGUID = &GUID_AVSTREAM_NAME_18;
		pGUID_registry = GUID_AVSTREAM_NAME_18_REGISTRY;
		break;
	case 18:
		pGUID = &GUID_AVSTREAM_NAME_19;
		pGUID_registry = GUID_AVSTREAM_NAME_19_REGISTRY;
		break;
	case 19:
		pGUID = &GUID_AVSTREAM_NAME_20;
		pGUID_registry = GUID_AVSTREAM_NAME_20_REGISTRY;
		break;
	case 20:
		pGUID = &GUID_AVSTREAM_NAME_21;
		pGUID_registry = GUID_AVSTREAM_NAME_21_REGISTRY;
		break;
	case 21:
		pGUID = &GUID_AVSTREAM_NAME_22;
		pGUID_registry = GUID_AVSTREAM_NAME_22_REGISTRY;
		break;
	case 22:
		pGUID = &GUID_AVSTREAM_NAME_23;
		pGUID_registry = GUID_AVSTREAM_NAME_23_REGISTRY;
		break;
	case 23:
		pGUID = &GUID_AVSTREAM_NAME_24;
		pGUID_registry = GUID_AVSTREAM_NAME_24_REGISTRY;
		break;
	default:
		Status = STATUS_UNSUCCESSFUL;
		break;
	}
	if ( SUCCESS( Status ) )
	{
		++m_Interface_guidNamesUsed;
	}

//Set friendly name to registry.
	if ( SUCCESS( Status ) )
	{
	//Create registry path.
		UNICODE_STRING		key_path;
		WCHAR				key_path_container[256];

		_snwprintf(
		  key_path_container,
		  ELEMENTS_IN_ARRAY(key_path_container),
		  L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control\\MediaCategories\\%s",
		  pGUID_registry
		);
		RtlInitUnicodeString( &key_path, key_path_container );

	//Open registry handle.
		OBJECT_ATTRIBUTES	attributes;
		HANDLE				hKey_GUID = 0;

		InitializeObjectAttributes(
		  &attributes,
		  &key_path,
		  OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		  NULL,
		  NULL
		);

		Status = ZwOpenKey( &hKey_GUID, KEY_WRITE, &attributes );
		if ( SUCCESS( Status ) )
		{
		//Set new friendly name.
			Status = ZwSetValueKey(
			  hKey_GUID,
			  &ChannelName_USTR,
			  0,
			  REG_SZ,
			  pFriendlyName,
			  ( (unsigned long)wcslen( pFriendlyName ) + 1 )*sizeof( WCHAR )	//+1 is NUL termination
			);

		//Release resources
			ZwClose( hKey_GUID );
		}
	}

//Return GUID.
	if ( SUCCESS( Status ) )
	{
		if ( ppGUID != NULL )
		{
			*ppGUID = pGUID;
		}
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

//***************************************************************************
// ProcedureName:
//		Interface_RegisterFilter/Interface_DisableFilter
//
// Explanation:
//		Emulates the AddInterface request from inf file.
//		Procedure registers/disable KSCATEGORY_AUDIO, KSCATEGORY_RENDER and
//		KSCATEGORY_CAPTURE for every stereo in/out channel.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// interfaceName: IN
//		Interface name that filter will be "connected" to.
//
// friendlyName: IN
//		The name of the filter that the user will see.
//
// --------------------------------------------------------------------------
// Return:
//		Should return STATUS_SUCCESS or the error code that was returned
//		from the attempt to perform the operation.
//
#pragma CODE_PAGED
NTSTATUS driver_c::Interface_Register_Filter
(
	IN card_c*		pCard,
	IN PWCHAR		interfaceName,
	IN PWCHAR		friendlyName
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	NTSTATUS			Status;

	HANDLE				hKey_interface = 0;
	GUID*				interfaceClassGuid;
	UNICODE_STRING		referenceName;
	UNICODE_STRING		symbolicLinkName;

//Convert PWCHAR to UNICODE_STRING.
	RtlInitUnicodeString( &referenceName, interfaceName );

//Register interface and set it's registry values.
	for ( long i = 0; i< 3; i++ )
	{
#ifdef DBG_AND_LOGGER
		pLogger->Log( "Int: %d\n", i );
#endif

	//Get class GUID.
		switch ( i )
		{
		case 0 :
			interfaceClassGuid = (GUID*) &KSCATEGORY_AUDIO;
			break;

		case 1 :
			interfaceClassGuid = (GUID*) &KSCATEGORY_RENDER;
			break;

		default:
		//case 2 :
			interfaceClassGuid = (GUID*) &KSCATEGORY_CAPTURE;
			break;
		}

	//Register Interface
		Status = IoRegisterDeviceInterface(
		  pCard->m_DeviceObject->PhysicalDeviceObject,
		  interfaceClassGuid,
		  &referenceName,
		  &symbolicLinkName
		);

	//Set CLSID and FriendlyName interface values.
		if ( SUCCESS( Status ) )
		{
		//Open interface registry handle.
			Status = IoOpenDeviceInterfaceRegistryKey(
			  &symbolicLinkName,
			  KEY_WRITE,
			  &hKey_interface
			);

		//Set registry values.
			if ( SUCCESS( Status ) )
			{
			//Set CLSID value.
				Status = ZwSetValueKey(
				  hKey_interface,
				  &CLSID_USTR,
				  0,
				  REG_SZ,
				  &guid_CLSID_Value_W,
				  sizeof( guid_CLSID_Value_W ) + sizeof( WCHAR )
				);

			//Set FriendlyName value.
				if ( SUCCESS( Status ) )
				{
					Status = ZwSetValueKey(
					  hKey_interface,
					  &FriendlyName_USTR,
					  0,
					  REG_SZ,
					  friendlyName,
					  ( (unsigned long)wcslen( friendlyName ) + 1 )*sizeof( WCHAR )
					);
				}

			//Release resources
				ZwClose( hKey_interface );
			}

			if ( SUCCESS( Status ) )
			{
			//Enable interface.
				Status = IoSetDeviceInterfaceState( &symbolicLinkName, TRUE );
			}

		//Release resources
			//RtlFreeUnicodeString( &symbolicLinkName );
		}

	//Failed?
		if ( Status != STATUS_SUCCESS )
		{
			break;
		}
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

#pragma CODE_PAGED
NTSTATUS driver_c::Interface_Disable_Filter
(
	IN card_c*		pCard,
	IN PWCHAR		interfaceName
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
#endif

	NTSTATUS			Status;

	GUID*				interfaceClassGuid;
	UNICODE_STRING		referenceName;
	UNICODE_STRING		symbolicLinkName;

//Convert PWCHAR to UNICODE_STRING.
	RtlInitUnicodeString( &referenceName, interfaceName );

//Register interface and set it's registry values.
	for ( long i = 0; i< 3; i++ )
	{
	//Get class GUID.
		switch ( i )
		{
		case 0 :
			interfaceClassGuid = (GUID*) &KSCATEGORY_AUDIO;
			break;

		case 1 :
			interfaceClassGuid = (GUID*) &KSCATEGORY_RENDER;
			break;

		case 2 :
			interfaceClassGuid = (GUID*) &KSCATEGORY_CAPTURE;
			break;
		}

	//Get Interface
		Status = IoRegisterDeviceInterface(
		  pCard->m_DeviceObject->PhysicalDeviceObject,
		  interfaceClassGuid,
		  &referenceName,
		  &symbolicLinkName
		);

		if ( SUCCESS( Status ) )
		{
		//Disable interface->we don't want that system will link
		//the interface to our driver when is not present anymore.
			Status = IoSetDeviceInterfaceState( &symbolicLinkName, FALSE );
		}
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}
