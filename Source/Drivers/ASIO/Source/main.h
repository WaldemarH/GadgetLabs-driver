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
// PURPOSE:
//		Main header file. Every file should include reference to this file.
//		(This file is intended for precompiled headers.)
//
// STATUS:
//		Unfinished.
//
//***************************************************************************
//***************************************************************************
#pragma once

//#define ASIO_LOGGER
//#define ASIO_PRINT

//External includes...
#include <windows.h>
#include <stdio.h>
#include "..\Resources\resource.h"

#include ".\COM\combase.h"
#include ".\ASIO\iasiodrv.h"

//Local includes....
#include "logger.h"

extern HANDLE GetDeviceIoControlHeader( const GUID* interfaceGuid, BOOL overlapped );


//Extra definitions...
#define ELEMENTS_IN_ARRAY( a )		( sizeof(a)/sizeof(a[0]) )

#define SUCCESS( status ) ( status != FALSE )
#define SUCCESS_ASIO( status ) ( status == ASE_OK )
//#define FAILED( status ) ( status == FALSE )
#define FAILED_ASIO( status ) ( status != ASE_OK )

