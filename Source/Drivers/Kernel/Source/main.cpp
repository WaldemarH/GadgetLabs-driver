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
//		Contains driver entry point.
//
// STATUS:
//		Finished/Untested.
//
//***************************************************************************
//***************************************************************************
#include "main.h"


//***************************************************************************
// ProcedureName:
//		DriverEntry
//
// Explanation:
//		Procedure is responsible for driver initialization.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// Synhronization:
//		None
//
// --------------------------------------------------------------------------
// DriverObject: IN
//		Caller-supplied pointer to a DRIVER_OBJECT structure.
//		This is the driver's driver object.
//
// RegistryPathName: IN
//		Pointer to a counted Unicode string specifying the path to
//		the driver's registry key.
//
// --------------------------------------------------------------------------
// Return:
//		STATUS_SUCCESS or one of the error status values defined in ntstatus.h.
//
#pragma CODE_INIT
extern "C" NTSTATUS DriverEntry
(
	IN PDRIVER_OBJECT		DriverObject,
	IN PUNICODE_STRING		RegistryPathName
)
{
	NTSTATUS	Status = STATUS_SUCCESS;

#ifdef DBG_AND_LOGGER
//Create and initialize logger class.
	pLogger = new logger_c();
	if ( pLogger == NULL )
	{
		Status = STATUS_INSUFFICIENT_RESOURCES;
	}

//Write driver version right at start.
	if ( SUCCESS( Status ) )
	{
		pLogger->Log( "Driver version: %s\n", DRIVER_VERSION );
	}

//Define procedure that we are in.
	if ( SUCCESS( Status ) )
	{
		Procedure_Start_Global();
	}
#endif

	//DbgBreakPoint();

//Create driver class.
	if ( SUCCESS( Status ) )
	{
		pDriver = new driver_c( &Status, DriverObject, RegistryPathName );
		if ( pDriver == NULL )
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
		}
	}

//Exit.
#ifdef DBG_AND_LOGGER
	if ( SUCCESS( Status ) )
	{
		Procedure_Exit( Status );
	}
#endif

	return Status;
}

