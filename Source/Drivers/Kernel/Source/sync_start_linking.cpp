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
#include "sync_start.h"


#pragma DATA_LOCKED


//*****************************************************************************
// ProcedureName:
//		GetSyncStart
//
//		Notice:
//		This procedure can be called only on the master syncStart.
//
// Explanation:
//		This procedure will return pointer to syncStart class that current
//		channel belongs to.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
// Return:
//		Should return STATUS_SUCCESS or the error code that was returned by the system.
//
#pragma CODE_LOCKED
NTSTATUS syncStart_c::GetSyncStart( syncStart_c** ppSyncStart, channel_base_c* pChannel )
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
//	Logger_Print( LS_GetPinSyncStartStructure_1, pSyncInfo, nStereoChannelsInSystem + 1 );
#endif

	NTSTATUS			Status = STATUS_SUCCESS;

	syncStart_c*		pSyncStart = NULL;
	syncStart_c*		pSyncStart_last = NULL;

//Validate...
	//Is it master syncStart class?
	if ( m_pPrevious != NULL )
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//Is current channel already part of syncStart class?
	if ( SUCCESS( Status ) )
	{
	//Search through all the syncStart classes and try to find requested channel.
		syncStart_c*		pSyncStart_temp = this;
		while ( pSyncStart_temp != NULL )
		{
			channel_base_c*		pChannel_temp;
			channel_base_c**	ppChannel_temp = m_ppChannels;

			while ( ( pChannel_temp = *ppChannel_temp ) != NULL )
			{
			//Is it the same?
				if ( pChannel_temp == pChannel )
				{
				//Found channel -> define in which syncStart class and exit.
					pSyncStart = pSyncStart_temp;
					goto exitProcedure;
				}

			//Next...
				++ppChannel_temp;
			}

		//Channel is not in current syncStart class -> backup pointer for lower code in case that channel will not be found.
			pSyncStart_last = pSyncStart_temp;

		//Next...
			pSyncStart_temp = (syncStart_c*)pSyncStart_temp->m_pNext;
		}
	}

//If we came to here then current channel is not part of any syncStart class!
//
//Notice:
//Create new only if the last syncStart channel has channels in running state.
	if ( SUCCESS( Status ) )
	{
		if ( pSyncStart_last->m_Channels_nInRunningState > 0 )
		{
		//Create new...
			pSyncStart = new syncStart_c( pSyncStart_last );
			if ( pSyncStart == NULL )
			{
				Status = STATUS_INSUFFICIENT_RESOURCES;
			}
		}
		else
		{
		//Last syncStart class is not yet in running state... return it.
			pSyncStart = pSyncStart_last;
		}
	}

//Exit -> return pointer to syncStart class.
	if ( SUCCESS( Status ) )
	{
exitProcedure:
		if ( ppSyncStart != NULL ) 
		{
			*ppSyncStart = pSyncStart;
		}
		else
		{
			Status = STATUS_UNSUCCESSFUL;
		}
	}

#ifdef DBG_AND_LOGGER
	//Logger_Print(
	//  LS_GetPinSyncStartStructure_2,
	//  pSyncInfo,
	//  nStereoChannelsInSystem + 1
	//);
	Procedure_Exit( Status );
#endif
	return Status;
}

//*****************************************************************************
// ProcedureName:
//		FreeSyncStart
//
//		Notice:
//		This procedure can be called only on the master syncStart.
//
// Explanation:
//		This procedure will free syncStart class if it doesn't contain any reference
//		to any channel.
//
// IRQ Level:
//		PASSIVE_LEVEL
//
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
// Return:
//		Should return STATUS_SUCCESS or the error code that was returned by the system.
//
#pragma CODE_LOCKED
NTSTATUS syncStart_c::FreeSyncStart( syncStart_c** ppSyncStart )
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Local();
	//Logger_Print(
	//  LS_ReleaseSyncStartStructure_1,
	//  pSyncInfo,
	//  nStereoChannelsInSystem + 1
	//);
#endif

	NTSTATUS			Status = STATUS_SUCCESS;

//Validate...
	//Is it master syncStart class?
	if ( m_pPrevious != NULL )
	{
		Status = STATUS_UNSUCCESSFUL;
	}

//Free class, but only if it's a slave class and all channel references are removed.
	if ( SUCCESS( Status ) )
	{
		if ( ppSyncStart != NULL )
		{
			syncStart_c*	pSyncStart = *ppSyncStart;

			if (
			  ( pSyncStart->m_pPrevious != NULL )
			  &&
			  ( pSyncStart->m_Channels_nReferences == 0 )
			)
			{
			//It's a slave class and no channel references present -> free class.
				delete pSyncStart;
				*ppSyncStart = NULL;
			}
		}
		else
		{
			Status = STATUS_UNSUCCESSFUL;
		}
	}

#ifdef DBG_AND_LOGGER
	//Logger_Print(
	//  LS_ReleaseSyncStartStructure_2,
	//  pSyncInfo,
	//  nStereoChannelsInSystem + 1
	//);
	Procedure_Exit( STATUS_SUCCESS );
#endif
	return Status;
}
