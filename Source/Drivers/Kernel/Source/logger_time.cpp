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
//		logger.cpp
//
// PURPOSE:
//		File contains logger procedures that are needed for saving log strings
//		to log file.
//
// STATUS:
//		Unfinished/Untested.
//
//***************************************************************************
//***************************************************************************
#include "main.h"


#ifdef DBG_AND_LOGGER

//***************************************************************************
// ProcedureName:
//		GetDriverTime
//
// Explanation:
//		Sets buffer with current timeStamp (the time since start of driver).
//
// IRQ Level:
//		ANY
//
// Synhronization:
//		None
//
// --------------------------------------------------------------------------
// pBuffer - IN
//		Pointer to buffer that will receive timeStamp text.
//
// type - IN
//		Defines shorter or longer form of returned text.
//
// --------------------------------------------------------------------------
// Return:
//		Number of character written to buffer or -1 if error.
//
#pragma CODE_LOCKED
__int32 logger_c::GetDriverTime
(
	IN OUT char*			pBuffer,
	IN TIMESTAMP_TYPE		type
)
{
	LARGE_INTEGER	currentTime;

	unsigned long	uSeconds;
	unsigned long	mSeconds;
	unsigned long	seconds;
	unsigned long	minutes;
	unsigned long	hours;
	unsigned long	days;

//Validate.
	if ( pBuffer == NULL )
		return -1;

//Get current time -> currentTime = k*100ns.
	KeQuerySystemTime( &currentTime );

//uSeconds = currentTime/10
	currentTime.QuadPart -= m_TimeStamp_driverStart.QuadPart;
	currentTime.QuadPart /= 10;

	uSeconds = (unsigned long) currentTime.QuadPart%1000;
	currentTime.QuadPart /= 1000;

	mSeconds = (unsigned long) currentTime.QuadPart%1000;
	currentTime.QuadPart /= 1000;

	seconds = (unsigned long) currentTime.QuadPart%60;
	currentTime.QuadPart /= 60;

	minutes = (unsigned long) currentTime.QuadPart%60;
	currentTime.QuadPart /= 60;

	hours = (unsigned long) currentTime.QuadPart%24;
	currentTime.QuadPart /= 24;

	days = (unsigned long) currentTime.QuadPart;

//Create timestamp text.
	__int32		retVal;

	if ( type == TIMESTAMP_TYPE_SHORT )
	{
		retVal = sprintf(
		  pBuffer,
		  "%d:%d:%d:%d:%d:%d (d:h:m:s:ms:us)",
		  days,
		  hours,
		  minutes,
		  seconds,
		  mSeconds,
		  uSeconds
		);
	}
	else
	{
		retVal = sprintf(
		  pBuffer,
		  "TimeFromStartOfDriver: %d:%d:%d:%d:%d:%d (d:h:m:s:ms:us)",
		  days,
		  hours,
		  minutes,
		  seconds,
		  mSeconds,
		  uSeconds
		);
	}

	return retVal;
}

////***************************************************************************
//// ProcedureName:
////		Logger_Print_Time
////
//// Explanation:
////      Procedure saves the time from start of the driver to log file.
////
//// IRQ Level:
////		PASSIVE_LEVEL
////
//// --------------------------------------------------------------------------
////
//// --------------------------------------------------------------------------
//// Return:
////      None.
////
//#pragma CODE_LOCKED
//void __stdcall Logger_Print_Time
//(
//)
//{
//	LARGE_INTEGER	currentTime;
//	LARGE_INTEGER	currentTime2;
//
//	unsigned long	seconds;
//	unsigned long	minutes;
//	unsigned long	hours;
//	unsigned long	days;
//
//
//	KeQuerySystemTime( &currentTime );
//
////currentTime = 100ns
////seconds = currentTime/10^7
//
//	currentTime.QuadPart -= driverStartTime.QuadPart;
//	currentTime2.QuadPart = currentTime.QuadPart;
//	currentTime.QuadPart /= 10000000;
//
//	seconds = (unsigned long) currentTime.QuadPart%60;
//	currentTime.QuadPart /= 60;
//
//	minutes = (unsigned long) currentTime.QuadPart%60;
//	currentTime.QuadPart /= 60;
//
//	hours = (unsigned long) currentTime.QuadPart%24;
//	currentTime.QuadPart /= 24;
//
//	days = (unsigned long) currentTime.QuadPart;
//
//	Logger_Print(
//	  "TimeFromStartOfDriver: %d days - %d:%d:%d (h:m:s)  (time in 100ns = 0x%X%X)\n",
//	  days,
//	  hours,
//	  minutes,
//	  seconds,
//	  currentTime2.HighPart,
//	  currentTime2.LowPart
//	);
//
//	return;
//}

#endif	//DBG_AND_LOGGER
