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
//		File contains IOCTL codes for GUI<->driver interaction.
//
//***************************************************************************
//***************************************************************************
#pragma once


//-----------------------------------------------------------------------------
//Driver private interface (intended for ASIO and GUI)
//
//DeviceIoCommands: {F3292AD7-71A1-4688-8504 -01641702F5DC}
#define DEVICE_IO_CONTROL_test			L"GLI DIOC 4688 x"
#define DEVICE_IO_CONTROL_small_test	L"\\gli dioc 4688 x"

#define DEVICE_IO_CONTROL_1				L"GLI DIOC 4688 1"
#define DEVICE_IO_CONTROL_2				L"GLI DIOC 4688 2"
#define DEVICE_IO_CONTROL_3				L"GLI DIOC 4688 3"
#define DEVICE_IO_CONTROL_4				L"GLI DIOC 4688 4"
#define DEVICE_IO_CONTROL_small_1		L"\\gli dioc 4688 1"
#define DEVICE_IO_CONTROL_small_2		L"\\gli dioc 4688 2"
#define DEVICE_IO_CONTROL_small_3		L"\\gli dioc 4688 3"
#define DEVICE_IO_CONTROL_small_4		L"\\gli dioc 4688 4"

#define DEVICE_IO_NAME					L"GadgetLabs deviceIoControl Interface"

static const GUID DEVICE_IO_GUID_1 = { 0xf3292ad7, 0x71a1, 0x4688, { 0x85, 0x04, 0x01, 0x64, 0x17, 0x02, 0xf5, 0xdc } };
static const GUID DEVICE_IO_GUID_2 = { 0xf3292ad8, 0x71a1, 0x4688, { 0x85, 0x04, 0x01, 0x64, 0x17, 0x02, 0xf5, 0xdc } };
static const GUID DEVICE_IO_GUID_3 = { 0xf3292ad9, 0x71a1, 0x4688, { 0x85, 0x04, 0x01, 0x64, 0x17, 0x02, 0xf5, 0xdc } };
static const GUID DEVICE_IO_GUID_4 = { 0xf3292ada, 0x71a1, 0x4688, { 0x85, 0x04, 0x01, 0x64, 0x17, 0x02, 0xf5, 0xdc } };

