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
//		card.cpp
//
// PURPOSE:
//		File contains the card class functions.
//
// STATUS:
//		Unfinished/Untested.
//
//***************************************************************************
//***************************************************************************
#include "main.h"
#include "linked_array.h"


#pragma DATA_LOCKED


#pragma CODE_LOCKED
linkedArray_c::linkedArray_c( linkedArray_c* pPrevious )
{
//Initialize variables...
	//Linking...
	m_pNext = NULL;

	m_pPrevious = pPrevious;
	if ( pPrevious != NULL )
	{
		pPrevious->m_pNext = this;
	}
}

#pragma CODE_LOCKED
linkedArray_c::~linkedArray_c()
{
//Remove current class from the link.
	if ( m_pNext != NULL )
	{
		m_pNext->m_pPrevious = m_pPrevious;
	}
	if ( m_pPrevious != NULL )
	{
		m_pPrevious->m_pNext = m_pNext;
	}
}

#pragma CODE_LOCKED
linkedArray_c* linkedArray_c::GetLast()
{
	linkedArray_c*		pLast = this;
	while ( pLast->m_pNext != NULL )
	{
		pLast = pLast->m_pNext;
	}

	return pLast;
}

#pragma CODE_LOCKED
linkedArray_c* linkedArray_c::GetNext()
{
	return m_pNext;
}


//***************************************************************************
// ProcedureName:
//		operator new/delete
//
// Explanation:
//		Procedure will allocate/deallocate class memory.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// Synhronization:
//		None
//
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// Return Codes:
//		...
//
#pragma CODE_LOCKED
void* __cdecl linkedArray_c::operator new ( size_t size )
{
	if ( size > 0 )
	{
		return (void*)ExAllocatePoolWithTag( NonPagedPool, size, MEMORY_TAG_LINKED_ARRAY_CLASS );
	}
	else
	{
		return 0;
	}
}

#pragma CODE_LOCKED
void __cdecl linkedArray_c::operator delete ( void* pMemory )
{
	if ( pMemory != NULL )
	{
		ExFreePoolWithTag( pMemory, MEMORY_TAG_LINKED_ARRAY_CLASS );
	}
}

#pragma CODE_LOCKED
void linkedArray_c::FreeAll()
{
	NTSTATUS		Status = STATUS_SUCCESS;

//Validate...
	if ( m_pPrevious != NULL )
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//Free all except master...
	if ( SUCCESS( Status )  )
	{
		linkedArray_c*		pLast;

		while ( this != ( pLast = GetLast() ) )
		{
			delete pLast;
		}
	}
}