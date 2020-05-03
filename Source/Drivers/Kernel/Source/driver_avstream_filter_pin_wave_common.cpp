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
//		avstream.cpp
//
// PURPOSE:
//		File contains the avstream subdriver class.
//
// STATUS:
//		Unfinished/Untested.
//
//***************************************************************************
//***************************************************************************
#include "main.h"


#pragma DATA_LOCKED

//***************************************************************************
// ProcedureName:
//		GetCard_FromFilter
//
// Explanation:
//		Procedure will try to find the filters's owner card.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// pFilter: IN
//		Pointer of a filter to retrieve the owner.
//
// ppCard: IN OUT
//		Pointer to variable that will get card_c pointer of filter owner.
//
// --------------------------------------------------------------------------
// Return:
//		STATUS_SUCCESS or
//		STATUS_UNSUCCESSFUL (failed to retrieve the filter owner).
//
#pragma CODE_LOCKED
NTSTATUS driver_c::GetCard_FromFilter
(
	IN		PKSFILTER	pFilter,
	IN OUT	card_c**	ppCard
)
{
//#ifdef DBG_PRINT
//	DbgPrint( "%s Start\n", __FUNCTION__ );
//#endif

	NTSTATUS		Status = STATUS_UNSUCCESSFUL;

	PKSDEVICE		pDeviceObject;

//Get filters's card class.
	if ( NULL != ( pDeviceObject = KsFilterGetDevice( pFilter ) ) )
	{
	//Get pin's card class.
		for ( int i = 0; i < ELEMENTS_IN_ARRAY(m_Cards); i++ )
		{
			card_c*		pCard_local = m_Cards[i];
			if ( pCard_local == NULL )
			{
				continue;
			}

		//Did we find it?
			if ( pCard_local->m_DeviceObject == pDeviceObject )
			{
				if ( ppCard != NULL )
				{
					*ppCard = pCard_local;
					Status = STATUS_SUCCESS;
				}
				break;
			}
		}
	}

//#ifdef DBG_PRINT
//	DbgPrint( "%s Exit -> Return: %d\n", __FUNCTION__, Status );
//#endif
	return Status;
}

//***************************************************************************
// ProcedureName:
//		GetCard_FromPin
//
// Explanation:
//		Procedure will try to find the pin's owner card.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// Pin: IN
//		Pointer of a pin to retrieve the owner.
//
// pCardClass: IN OUT
//		Pointer to variable that will get card_c pointer of pin owner.
//
// --------------------------------------------------------------------------
// Return:
//		STATUS_SUCCESS or
//		STATUS_UNSUCCESSFUL (failed to retrieve the pin owner).
//
#pragma CODE_LOCKED
NTSTATUS driver_c::GetCard_FromPin
(
	IN		PKSPIN		pPin,
	IN OUT	card_c**	ppCard
)
{
//#ifdef DBG_PRINT
//	DbgPrint( "%s Start\n", __FUNCTION__ );
//#endif

	NTSTATUS		Status = STATUS_UNSUCCESSFUL;

	PKSDEVICE		pDeviceObject;

//Get pin's card class.
	if ( NULL != ( pDeviceObject = KsPinGetDevice( pPin ) ) )
	{
	//Get pin's card class.
		for ( int i = 0; i < ELEMENTS_IN_ARRAY(m_Cards); i++ )
		{
			card_c*		pCard_local = m_Cards[i];
			if ( pCard_local == NULL )
			{
				continue;
			}

		//Did we find it?
			if ( pCard_local->m_DeviceObject == pDeviceObject )
			{
				if ( ppCard != NULL )
				{
					*ppCard = pCard_local;
					Status = STATUS_SUCCESS;
				}
				break;
			}
		}
	}

//#ifdef DBG_PRINT
//	DbgPrint( "%s Exit -> Return: %d\n", __FUNCTION__, Status );
//#endif
	return Status;
}
