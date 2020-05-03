//*****************************************************************************
//*****************************************************************************
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
//*****************************************************************************
//*****************************************************************************
//
// PURPOSE:
//		Contains various PLX chip constants.
//
// STATUS:
//		Finished.
//
//*****************************************************************************
//*****************************************************************************

#pragma once


// PLX register map sizes for IO and memory spaces. Both are present for
// PCI devices so device can be accessed via memory or IO. These sizes
// are in bytes.
#define PLX_IO_SIZE							0x80
#define PLX_MEM_SIZE						0x80

//Offsets for PCI Configuration Registers.
#define PCI_VENDOR_DEVICE_ID				0x00
#define PCI_SUBSYSTEM_ID					0x2C

//Local Address Space 3 Local Base Address (Remap) Register
#define PLX_LAS3BA							0x20
#define PLX_LAS3BA__ENABLE					0x00000001

//Local Address Space 3 Bus Region Descriptor Register
#define PLX_LAS3BRD							0x34

#define PLX_LASRBRD__BURST					0x00000001		//bit0
#define PLX_LASRBRD__READY					0x00000002		//bit1
#define PLX_LASRBRD__BTERM					0x00000004		//bit2
#define PLX_LASRBRD__PREFETCH_COUNT_MASK	0x00000018		//bit3-4
#define PLX_LASRBRD__PREFETCH_COUNT_4		0x00000008		//bit3-4
#define PLX_LASRBRD__PREFETCH_COUNT_8		0x00000010		//bit3-4	//this is max as fifo is only 32bytes in size

//Plx Interrupt Control/Status Register
#define PLX_INT_CN_ST						0x4C

#define PLX_INT_CN_ST__INT1_EN				0x00000001	//bit0
#define PLX_INT_CN_ST__INT1_ST				0x00000004	//bit2
#define PLX_INT_CN_ST__PCI_INT_EN			0x00000040	//bit6

//PLX Chip local configuration registers.
#define PLX_CNTR							0x50

#define PLX_CNTR__USER0						0x00000004	//bit2
#define PLX_CNTR__USER1						0x00000020	//bit5
#define PLX_CNTR__READ_MODE					0x00004000	//bit14
#define PLX_CNTR__READ_WITH_WRITE_FLUSH		0x00008000	//bit15
#define PLX_CNTR__READ_NO_FLUSH				0x00010000	//bit16
#define PLX_CNTR__READ_NO_WRITE				0x00020000	//bit17
#define PLX_CNTR__WRITE_MODE				0x00040000	//bit18
#define PLX_CNTR__RETRY_DELAY_MASK			0x00780000	//bit19-bit22
#define PLX_CNTR__DIRECT_SLAVE_LOCK			0x00800000	//bit23
#define PLX_CNTR__EEPROM_CLK				0x01000000	//bit24
#define PLX_CNTR__EEPROM_CS					0x02000000	//bit25
#define PLX_CNTR__EEPROM_WRITE				0x04000000	//bit26
#define PLX_CNTR__EEPROM_READ				0x08000000	//bit27
#define PLX_CNTR__EEPROM_VALID				0x10000000	//bit28
#define PLX_CNTR__LBR						0x40000000	//bit30

