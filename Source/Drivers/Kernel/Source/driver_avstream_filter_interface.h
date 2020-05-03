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
//		File contains definitions for main class.
//
//***************************************************************************
//***************************************************************************
#pragma once

//Includes


//***************************************************************************
//Guids
#define	guid_CLSID_Value_W		L"{17CCA71B-ECD7-11D0-B908-00A0C9223196}"
#define	sCLSID					L"CLSID"
#define	FRIENDLY_NAME			L"FriendlyName"
#define	CHANNEL_NAME			L"Name"

//Wave Pin and Midi Node Names
static const GUID GUID_AVSTREAM_NAME_1 = { 0x1C2C57A7, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };
static const GUID GUID_AVSTREAM_NAME_2 = { 0x1C2C57A8, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };
static const GUID GUID_AVSTREAM_NAME_3 = { 0x1C2C57A9, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };
static const GUID GUID_AVSTREAM_NAME_4 = { 0x1C2C57AA, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };
static const GUID GUID_AVSTREAM_NAME_5 = { 0x1C2C57AB, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };
static const GUID GUID_AVSTREAM_NAME_6 = { 0x1C2C57AC, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };
static const GUID GUID_AVSTREAM_NAME_7 = { 0x1C2C57AD, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };
static const GUID GUID_AVSTREAM_NAME_8 = { 0x1C2C57AE, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };
static const GUID GUID_AVSTREAM_NAME_9 = { 0x1C2C57AF, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };
static const GUID GUID_AVSTREAM_NAME_10 = { 0x1C2C57B0, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };
static const GUID GUID_AVSTREAM_NAME_11 = { 0x1C2C57B1, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };
static const GUID GUID_AVSTREAM_NAME_12 = { 0x1C2C57B2, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };
static const GUID GUID_AVSTREAM_NAME_13 = { 0x1C2C57B3, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };
static const GUID GUID_AVSTREAM_NAME_14 = { 0x1C2C57B4, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };
static const GUID GUID_AVSTREAM_NAME_15 = { 0x1C2C57B5, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };
static const GUID GUID_AVSTREAM_NAME_16 = { 0x1C2C57B6, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };
static const GUID GUID_AVSTREAM_NAME_17 = { 0x1C2C57B7, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };
static const GUID GUID_AVSTREAM_NAME_18 = { 0x1C2C57B8, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };
static const GUID GUID_AVSTREAM_NAME_19 = { 0x1C2C57B9, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };
static const GUID GUID_AVSTREAM_NAME_20 = { 0x1C2C57BA, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };
static const GUID GUID_AVSTREAM_NAME_21 = { 0x1C2C57BB, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };
static const GUID GUID_AVSTREAM_NAME_22 = { 0x1C2C57BC, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };
static const GUID GUID_AVSTREAM_NAME_23 = { 0x1C2C57BD, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };
static const GUID GUID_AVSTREAM_NAME_24 = { 0x1C2C57BE, 0x1F1F, 0x4689, { 0x96, 0x21, 0xAC, 0x52, 0xDE, 0x4E, 0x36, 0x3E } };

#define GUID_AVSTREAM_NAME_1_REGISTRY		L"{1C2C57A7-1F1F-4689-9621-AC52DE4E363E}"
#define GUID_AVSTREAM_NAME_2_REGISTRY		L"{1C2C57A8-1F1F-4689-9621-AC52DE4E363E}"
#define GUID_AVSTREAM_NAME_3_REGISTRY		L"{1C2C57A9-1F1F-4689-9621-AC52DE4E363E}"
#define GUID_AVSTREAM_NAME_4_REGISTRY		L"{1C2C57AA-1F1F-4689-9621-AC52DE4E363E}"
#define GUID_AVSTREAM_NAME_5_REGISTRY		L"{1C2C57AB-1F1F-4689-9621-AC52DE4E363E}"
#define GUID_AVSTREAM_NAME_6_REGISTRY		L"{1C2C57AC-1F1F-4689-9621-AC52DE4E363E}"
#define GUID_AVSTREAM_NAME_7_REGISTRY		L"{1C2C57AD-1F1F-4689-9621-AC52DE4E363E}"
#define GUID_AVSTREAM_NAME_8_REGISTRY		L"{1C2C57AE-1F1F-4689-9621-AC52DE4E363E}"
#define GUID_AVSTREAM_NAME_9_REGISTRY		L"{1C2C57AF-1F1F-4689-9621-AC52DE4E363E}"
#define GUID_AVSTREAM_NAME_10_REGISTRY		L"{1C2C57B0-1F1F-4689-9621-AC52DE4E363E}"
#define GUID_AVSTREAM_NAME_11_REGISTRY		L"{1C2C57B1-1F1F-4689-9621-AC52DE4E363E}"
#define GUID_AVSTREAM_NAME_12_REGISTRY		L"{1C2C57B2-1F1F-4689-9621-AC52DE4E363E}"
#define GUID_AVSTREAM_NAME_13_REGISTRY		L"{1C2C57B3-1F1F-4689-9621-AC52DE4E363E}"
#define GUID_AVSTREAM_NAME_14_REGISTRY		L"{1C2C57B4-1F1F-4689-9621-AC52DE4E363E}"
#define GUID_AVSTREAM_NAME_15_REGISTRY		L"{1C2C57B5-1F1F-4689-9621-AC52DE4E363E}"
#define GUID_AVSTREAM_NAME_16_REGISTRY		L"{1C2C57B6-1F1F-4689-9621-AC52DE4E363E}"
#define GUID_AVSTREAM_NAME_17_REGISTRY		L"{1C2C57B7-1F1F-4689-9621-AC52DE4E363E}"
#define GUID_AVSTREAM_NAME_18_REGISTRY		L"{1C2C57B8-1F1F-4689-9621-AC52DE4E363E}"
#define GUID_AVSTREAM_NAME_19_REGISTRY		L"{1C2C57B9-1F1F-4689-9621-AC52DE4E363E}"
#define GUID_AVSTREAM_NAME_20_REGISTRY		L"{1C2C57BA-1F1F-4689-9621-AC52DE4E363E}"
#define GUID_AVSTREAM_NAME_21_REGISTRY		L"{1C2C57BB-1F1F-4689-9621-AC52DE4E363E}"
#define GUID_AVSTREAM_NAME_22_REGISTRY		L"{1C2C57BC-1F1F-4689-9621-AC52DE4E363E}"
#define GUID_AVSTREAM_NAME_23_REGISTRY		L"{1C2C57BD-1F1F-4689-9621-AC52DE4E363E}"
#define GUID_AVSTREAM_NAME_24_REGISTRY		L"{1C2C57BE-1F1F-4689-9621-AC52DE4E363E}"
