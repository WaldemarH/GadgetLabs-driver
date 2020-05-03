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
//		File defines port class.
//
//		It's intention is to make a layer that can communicate with
//		Input/Output ports.
//
// STATUS:
//		Finished.
//
//***************************************************************************
//***************************************************************************

#pragma once


class port_c
{
//Read/Write port.
public:
	static __forceinline BYTE ReadByte( IN PUCHAR port )
	{
		return READ_PORT_UCHAR( port );
	}
	static __forceinline WORD ReadWord( IN PUSHORT port )
	{
		return READ_PORT_USHORT( port );
	}
	static __forceinline DWORD ReadDword( IN PULONG port )
	{
		return READ_PORT_ULONG( port );
	}

	static __forceinline void WriteByte( IN PUCHAR port, IN BYTE data )
	{
		WRITE_PORT_UCHAR( port, data );
	}
	static __forceinline void WriteWord( IN PUSHORT port, IN WORD data )
	{
		WRITE_PORT_USHORT( port, data );
	}
	static __forceinline void WriteDword( IN PULONG port, IN DWORD data )
	{
		WRITE_PORT_ULONG( port, data );
	}

//Read/Write port with offset.
	static __forceinline BYTE ReadByte( IN PUCHAR port, IN unsigned long offset )
	{
		return READ_PORT_UCHAR( port+offset );
	}
	static __forceinline WORD ReadWord( IN PUSHORT port, IN unsigned long offset )
	{
		return READ_PORT_USHORT( (PUSHORT)( (PUCHAR)port+offset ) );
	}
	static __forceinline DWORD ReadDword( IN PULONG port, IN unsigned long offset )
	{
		return READ_PORT_ULONG( (PULONG)( (PUCHAR)port+offset ) );
	}

	static __forceinline void WriteByte( IN PUCHAR port, IN unsigned long offset, IN BYTE data )
	{
		WRITE_PORT_UCHAR( port+offset, data );
	}
	static __forceinline void WriteWord( IN PUSHORT port, IN unsigned long offset, IN WORD data )
	{
		WRITE_PORT_USHORT( (PUSHORT)( (PUCHAR)port+offset ), data );
	}
	static __forceinline void WriteDword( IN PULONG port, IN unsigned long offset, IN DWORD data )
	{
		WRITE_PORT_ULONG( (PULONG)( (PUCHAR)port+offset ), data );
	}
};
