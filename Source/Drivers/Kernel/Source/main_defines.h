//*****************************************************************************
//*****************************************************************************
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
//*****************************************************************************
//*****************************************************************************
#pragma once

//*****************************************************************************
//***  Defines  ***************************************************************
//*****************************************************************************
//typedef DWORD						SAMPLE;
#define BUFFER_SIZE_MAX				0x200	//samples (for mono channel)

#define NO							0
#define YES							1

//Is it STATUS_SUCCESS?
#define SUCCESS( status )			( status == STATUS_SUCCESS )
#define FAILED( status )			( status != STATUS_SUCCESS )

#define CODE_INIT					code_seg("INIT")
#define CODE_LOCKED					code_seg()
#define CODE_PAGED					code_seg("PAGE")

#define DATA_LOCKED					data_seg()
#define DATA_PAGED					data_seg("PAGEDATA")

#define ELEMENTS_IN_ARRAY( a )		( sizeof(a)/sizeof(a[0]) )
