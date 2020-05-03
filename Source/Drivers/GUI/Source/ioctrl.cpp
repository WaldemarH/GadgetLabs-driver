//*************************************************************************************************
//*************************************************************************************************
//
//                          PROPRIETARY INFORMATION
//
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Waldemar Haszlakiewicz and may not be
//     copied or disclosed except in accordance with the terms of that agreement.
//
//               Copyright (C) 2002, 2003 Waldemar Haszlakiewicz
//                            All Rights Reserved.
//  
//*************************************************************************************************
//*************************************************************************************************
//
//     FILE: $Filename$
//
//  PURPOSE:
//
// CONTENTS:
//
//    NOTES:  Tabstops are set to 4 and 90 character limit per source line.
//
// REVISION: $Revision$ $Date$
// 
//*************************************************************************************************
//*************************************************************************************************
#include <windows.h>
#include <setupapi.h>

HANDLE GetDeviceIoControlHeader( const GUID* interfaceGuid, BOOL overlapped )
{
	SP_DEVINFO_DATA						deviceInfoData;
	SP_DEVICE_INTERFACE_DATA			deviceInterfaceData;
	HANDLE								hDeviceIoControl;
	HDEVINFO							hInfoList = NULL;
	HLOCAL								hPathBuffer = 0;
	TCHAR*								pathBuffer = NULL;
	PSP_INTERFACE_DEVICE_DETAIL_DATA	pDetail;
	DWORD								requiredSize;
	BOOL								retValue;

//Initialize.
	deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	deviceInterfaceData.cbSize = sizeof( deviceInterfaceData );
	hDeviceIoControl = INVALID_HANDLE_VALUE;
	requiredSize = 0;

//Open an enumeration handle so we can locate all devices of our own class.
	hInfoList = SetupDiGetClassDevs(
	  interfaceGuid,
	  NULL,
	  NULL,
	  DIGCF_INTERFACEDEVICE | DIGCF_PRESENT
	);
	if ( hInfoList == INVALID_HANDLE_VALUE )
	{
		return hDeviceIoControl;
	}

//We have only 1 path available so there is no need to search for more.
	retValue = SetupDiEnumDeviceInterfaces(
	  hInfoList,
	  NULL,
	  interfaceGuid,
	  0,	//deviceIndex -> 0 => first path
	  &deviceInterfaceData
	);
	if ( retValue == NULL )
	{
		SetupDiDestroyDeviceInfoList( hInfoList );

		return hDeviceIoControl;
	}

//Get SP_DEVICE_INTERFACE_DETAIL_DATA size.
	retValue = SetupDiGetDeviceInterfaceDetail(
	  hInfoList,
	  &deviceInterfaceData,
	  NULL,
	  0,
	  &requiredSize,
	  NULL
	);
	if ( retValue == FALSE )
	{
		DWORD	errorCode = GetLastError();
		if (
		  ( errorCode != 0 )
		  &&
		  ( errorCode != ERROR_INSUFFICIENT_BUFFER )
		  &&
		  ( errorCode != ERROR_IO_PENDING )
		)
		{
			SetupDiDestroyDeviceInfoList( hInfoList );

			return hDeviceIoControl;
		}
	}

//Allocate memory for structure
	pDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA) malloc( requiredSize );
	if ( pDetail == FALSE )
	{
		SetupDiDestroyDeviceInfoList( hInfoList );

		return hDeviceIoControl;
	}

//Get detailed information.
	//Initialize.
	pDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

	retValue = SetupDiGetDeviceInterfaceDetail(
	  hInfoList,
	  &deviceInterfaceData,
	  pDetail,
	  requiredSize,
	  NULL,
	  &deviceInfoData
	);
	if ( retValue == FALSE )
	{
		free( pDetail );
		SetupDiDestroyDeviceInfoList( hInfoList );

		return hDeviceIoControl;
	}

//Get handle to our device.
	if ( overlapped == TRUE )
	{
		hDeviceIoControl = CreateFile(
		  pDetail->DevicePath,
		  GENERIC_READ | GENERIC_WRITE,
		  FILE_SHARE_READ | FILE_SHARE_WRITE,
		  NULL,
		  OPEN_EXISTING,
		  FILE_FLAG_OVERLAPPED,
		  NULL
		);
	}
	else
	{
		hDeviceIoControl = CreateFile(
		  pDetail->DevicePath,
		  GENERIC_READ | GENERIC_WRITE,
		  FILE_SHARE_READ | FILE_SHARE_WRITE,
		  NULL,
		  OPEN_EXISTING,
		  0,
		  NULL
		);
	}

//Free memory.
	free( pDetail );
	SetupDiDestroyDeviceInfoList( hInfoList );

	return hDeviceIoControl;
}

