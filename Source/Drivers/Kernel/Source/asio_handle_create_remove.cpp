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
#include "asio_handle.h"


#pragma DATA_LOCKED

#pragma CODE_LOCKED
NTSTATUS asioHandle_c::Handle_CreateNew( OUT asioHandle_c** ppHandle )
{
	NTSTATUS		Status = STATUS_SUCCESS;

//Validate...
	if (
	  ( m_pPrevious != NULL )
	  ||
	  ( ppHandle == NULL )
	)
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//Create new handle.
	if ( SUCCESS( Status ) )
	{
		asioHandle_c*	pHandle_temp = new asioHandle_c( (asioHandle_c*)GetLast() );
		if ( pHandle_temp != NULL )
		{
		//Set ID.
			pHandle_temp->m_HandleID = m_HandleID_last++;

		//Return class.
			*ppHandle = pHandle_temp;
		}
		else
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
		}
	}

	return Status;
}

#pragma CODE_LOCKED
NTSTATUS asioHandle_c::Handle_Remove( unsigned long handleID )
{
	NTSTATUS	Status = STATUS_SUCCESS;

//Get handle and remove it.
//
//Notice:
//Validation is not needed as GetHandle validates everything already.
	asioHandle_c*	pHandle = GetHandle( handleID );
	if ( pHandle != NULL )
	{
		delete pHandle;
	}
	else
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//Was this the last handle removed?
	if ( SUCCESS( Status ) )
	{
		if ( m_pNext == NULL )
		{
		//There are no more handles available -> reset last ID value.
			m_HandleID_last = 1;
		}
	}

	return Status;
}

#pragma CODE_LOCKED
asioHandle_c* asioHandle_c::GetHandle( unsigned long handleID )
{
	NTSTATUS		Status = STATUS_SUCCESS;

//Validate...
	if (
	  ( m_pPrevious != NULL )
	  ||
	  ( handleID < 1 )
	)
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//Find handle.
	asioHandle_c*	pHandle = NULL;

	if ( SUCCESS( Status ) )
	{
		pHandle = (asioHandle_c*)m_pNext;
		while ( pHandle != NULL )
		{
		//Did we find the right handle?
			if ( pHandle->m_HandleID == handleID )
			{
			//Yes.
				break;
			}

		//Next...
			pHandle = (asioHandle_c*)pHandle->m_pNext;
		}
	}

	return pHandle;
}
