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
#pragma once


//#define GL_LOGGER
//#define DBG_PRINT
#define DRIVER_VERSION				"4.2.0.1"
#define MAX_CARDS					4
#define MAX_STEREO_PAIRS_PER_CARD	4
#define MAX_CHANNELS_STEREO			MAX_CARDS * 2*MAX_STEREO_PAIRS_PER_CARD		//every 824 card has 4 in and 4 out stereo channels... so MAX_CARDS * 8 is maximum channels in system

extern "C" {
#include <wdm.h>
}

#include <stdio.h>
#include <stdarg.h>
#include <ntstatus.h>
#include <windef.h>

#define NOBITMAP
#include <mmreg.h>
#undef NOBITMAP

#include <ks.h>
#include <ksmedia.h>
#include <wdmguid.h>
#include <dmusprop.h>
#include <portcls.h>
#include <DMusicKS.h>

//Some extra structures that will help us out.
typedef struct	
{
	KSDATAFORMAT			DataFormat;
	WAVEFORMATEXTENSIBLE	WaveFormatEx;
} KSDATAFORMAT_WAVEFORMATEXTENSIBLE, *PKSDATAFORMAT_WAVEFORMATEXTENSIBLE;

__declspec(align(1)) struct SAMPLE
{
	__declspec(align(1)) union
	{
		unsigned char	bytes[4];
		unsigned char	words[2];
		unsigned long	dwords[1];
	} data;
};
__declspec(align(1)) struct SAMPLES_STEREO_24BIT
{
	BYTE		sample[3];
};
__declspec(align(1)) struct clock_WORDS_s
{
	WORD			word_loLo;
	WORD			word_loHi;
	WORD			word_hiLo;
	WORD			word_hiHi;
};
__declspec(align(1)) struct clock_DWORDS_s
{
	DWORD			dword_lo;
	unsigned long	dword_hi;
};
__declspec(align(1)) union sampleClock_s
{
	ULONGLONG			clock;
	clock_WORDS_s		words;
	clock_DWORDS_s		dwords;
};
#define SAMPLE_CLOCKS		//sampleClock_s structure is defined


#pragma data_seg()			//lock all data that could accidentally be left of DATA_XXX macro

#include "logger.h"
#include "driver.h"
#include "card.h"
#include "main_defines.h"

