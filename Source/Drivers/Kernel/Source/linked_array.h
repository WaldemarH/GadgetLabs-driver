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


class linkedArray_c
{
	public: linkedArray_c( linkedArray_c* pPrevious );
	public: virtual ~linkedArray_c();

//Class memory
	#define MEMORY_TAG_LINKED_ARRAY_CLASS		'LCAL'	//tag: "LACL"

	public: static void* __cdecl operator new( size_t size );
	public: static void __cdecl operator delete( void* pMemory );

	void FreeAll();

//Linking...
	public: linkedArray_c* GetLast();
	public: linkedArray_c* GetNext();

	protected: linkedArray_c*		m_pNext;
	protected: linkedArray_c*		m_pPrevious;
};