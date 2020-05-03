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
//      File defines delay class.
//
// STATUS:
//      Unfinished.
//
//***************************************************************************
//***************************************************************************

#pragma once

class delay_c
{
public:

//DelayThreadLongXXXXX - Use for longer delays( 0.5ms<x )
//time=100ns*nTicks
//Notice:
//Try to use DelayThreadLong_PassiveLevel as much as possible.
//
//Notice2:
//This delay normally gets exaggerated by the system.
//So at PASSIVE_LEVEL even if you have requested just 0.5ms the delay can even be
//15ms or whatever, so for more an exact delay use some other technique.
	enum DELAY_THREAD_LONG
	{
		DELAY_THREAD_LONG_1ms			= 10000,
		DELAY_THREAD_LONG_2ms			= 20000,
		DELAY_THREAD_LONG_5ms			= 50000,
		DELAY_THREAD_LONG_15ms			= 150000,
		DELAY_THREAD_LONG_20ms			= 200000,
		DELAY_THREAD_LONG_30ms			= 300000,
		DELAY_THREAD_LONG_1s			= 10000000
	};
	__forceinline void DelayThreadLong_PassiveLevel( IN DELAY_THREAD_LONG nTicks )
	{
		LARGE_INTEGER	DTL_Ticks;

		DTL_Ticks.LowPart = 0 - nTicks;
		DTL_Ticks.HighPart = 0xFFFFFFFF;
		KeDelayExecutionThread( KernelMode, FALSE, &DTL_Ticks );
	}

//DelayThreadShort - Use for shorter delays( 1us<x<0.5ms )
//time=1us*nTicks
	enum DELAY_THREAD_SHORT
	{
		DELAY_THREAD_SHORT_1us			= 1,
		DELAY_THREAD_SHORT_2us			= 2,
		DELAY_THREAD_SHORT_4us			= 4,
		DELAY_THREAD_SHORT_8us			= 8,
		DELAY_THREAD_SHORT_10us			= 10,
		DELAY_THREAD_SHORT_16us			= 16,
		DELAY_THREAD_SHORT_30us			= 30,
		DELAY_THREAD_SHORT_35us			= 35,
		DELAY_THREAD_SHORT_50us			= 50,
		DELAY_THREAD_SHORT_100us		= 100,
		DELAY_THREAD_SHORT_400us		= 400
	};
	__forceinline void DelayThreadShort( IN DELAY_THREAD_SHORT nTicks )
	{
		KeStallExecutionProcessor( nTicks );
	}

//DelayThreadVeryShort - Use for very short delays ( 100ns<x<1us )
//time=100ns*nTicks
	enum DELAY_THREAD_VERY_SHORT
	{
		DELAY_THREAD_VERY_SHORT_300ns	= 3,
		DELAY_THREAD_VERY_SHORT_600ns	= 6,
		DELAY_THREAD_VERY_SHORT_900ns	= 9
	};
	__forceinline void DelayThreadVeryShort( IN PUSHORT port, IN unsigned long nTicks )
	{
		while ( nTicks>0 )
		{
			READ_PORT_USHORT( port );
			--nTicks;
		}
	}
};
