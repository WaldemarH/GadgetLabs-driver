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

//****************************************************************************
// ProcedureName:
// 		Pin_Create_Midi
//
// Explanation:
//		An AVStream minidriver's AVStrMiniPinCreate routine is called when a
//		pin is created. Typically, this routine is used by minidrivers that
//		want to initialize the context and resources associated with the pin.
//
// IRQ Level:
// 		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// Pin: IN
//		Pointer to the KSPIN that was just created.
//
// Irp: IN
//		Pointer to the IRP_MJ_CREATE for Pin.
//
// --------------------------------------------------------------------------
// Return:
// 		Should return STATUS_SUCCESS or an error message.
//
#pragma CODE_LOCKED
NTSTATUS driver_c::Pin_Create_Midi
(
	IN PKSPIN	pPin,
	IN PIRP		pIrp
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
	pLogger->Log( "Pin->Id: %d\n", pPin->Id );
#endif

	NTSTATUS			Status = STATUS_SUCCESS;

//Initialize variables.
	//Cards...
	card_c*				pCard = NULL;

	Status = pDriver->GetCard_FromPin( pPin, &pCard );

	//Channel...
	channel_midi_c*		pChannel_midi = NULL;

	if ( SUCCESS( Status ) )
	{
		Status = pCard->GetChannel_FromPin_Midi( pPin, &pChannel_midi );
	}

	//Spinlock...
	PKSPIN_LOCK			pSpinLock = NULL;

	if ( SUCCESS( Status ) )
	{
		if ( pChannel_midi->m_Type == channel_midi_c::CHANNEL_TYPE_IN )
		{
		//Input...
			pSpinLock = &pCard->m_SpinLock_channel_midi_in;
		}
		else
		{
		//Output...
			pSpinLock = &pCard->m_SpinLock_channel_midi_out;
		}
	}

//Get driver type.
	channel_midi_c::CHANNEL_TYPE_CLIENT		driverType = channel_midi_c::CHANNEL_TYPE_CLIENT_NON;

	if ( SUCCESS( Status ) )
	{
		if ( (BOOL)TRUE == IsEqualGUID( pPin->ConnectionFormat->SubFormat, KSDATAFORMAT_SUBTYPE_MIDI ) )
		{
			driverType = channel_midi_c::CHANNEL_TYPE_CLIENT_MME;
		}
		else if ( (BOOL)TRUE == IsEqualGUID( pPin->ConnectionFormat->SubFormat, KSDATAFORMAT_SUBTYPE_DIRECTMUSIC ) )
		{
			driverType = channel_midi_c::CHANNEL_TYPE_CLIENT_DX;
		}
		else
		{
			Status = STATUS_UNSUCCESSFUL;
		}
	}

//Is this channel available?
	if ( SUCCESS( Status ) )
	{
	//Acquire channel spinLock.
		KIRQL		spinLock_oldLevel;

		KeAcquireSpinLock( pSpinLock, &spinLock_oldLevel );

		//Is channel in use?
			Status = pCard->IsChannelInUse_Midi( pChannel_midi );
			if ( SUCCESS( Status ) )
			{
			//No -> allocate it.
				Status = pChannel_midi->Set_AVStream_Pin( pPin );
				if ( SUCCESS( Status ) )
				{
				//Set driver type.
					pChannel_midi->m_Type_client = driverType;

				//Set copy procedure.
					Status = pChannel_midi->Set_CopyBuffersProcedure();
					if ( FAILED( Status ) )
					{
					//Reset variables back to original states.
						pChannel_midi->Set_AVStream_Pin( NULL );
						pChannel_midi->m_Type_client = channel_midi_c::CHANNEL_TYPE_CLIENT_NON;
					}
				}
			}

	//Release SPinLock.
		KeReleaseSpinLock( pSpinLock, spinLock_oldLevel );
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}

//****************************************************************************
// ProcedureName:
// 		Pin_Close_Midi
//
// Explanation:
//		An AVStream minidriver's AVStrMiniPinClose routine is called when a
//		pin is closed. It usually is provided by minidrivers that want to free
//		the context and resources associated with the pin.
//
// IRQ Level:
// 		PASSIVE_LEVEL
//
// --------------------------------------------------------------------------
// Pin: IN
//		Pointer to the KSPIN that was just closed.
//
// Irp: IN
//		Pointer to the IRP_MJ_CLOSE for Pin.
//
// --------------------------------------------------------------------------
// Return:
// 		Should return STATUS_SUCCESS or an error message.
//
#pragma CODE_LOCKED
NTSTATUS driver_c::Pin_Close_Midi
(
	IN PKSPIN	pPin,
	IN PIRP		pIrp
)
{
#ifdef DBG_AND_LOGGER
	Procedure_Start_Global();
	pLogger->Log( "Pin->Id: %d\n", pPin->Id );
#endif

	NTSTATUS		Status = STATUS_SUCCESS;

//Initialize variables.
	//Cards...
	card_c*				pCard = NULL;

	Status = pDriver->GetCard_FromPin( pPin, &pCard );

	//Channel...
	channel_midi_c*		pChannel_midi = NULL;

	if ( SUCCESS( Status ) )
	{
		Status = pCard->GetChannel_FromPin_Midi( pPin, &pChannel_midi );
	}

	//Spinlock...
	PKSPIN_LOCK			pSpinLock = NULL;

	if ( SUCCESS( Status ) )
	{
		if ( pChannel_midi->m_Type == channel_midi_c::CHANNEL_TYPE_IN )
		{
		//Input...
			pSpinLock = &pCard->m_SpinLock_channel_midi_in;
		}
		else
		{
		//Output...
			pSpinLock = &pCard->m_SpinLock_channel_midi_out;
		}
	}

//Free resources.
	if ( SUCCESS( Status ) )
	{
	//Acquire channelInfo spinLock.
		KIRQL		spinLock_oldLevel;

		KeAcquireSpinLock( pSpinLock, &spinLock_oldLevel );

		//Free pin if it's in KSSTATE_STOP state only.
			if ( pChannel_midi->m_State == KSSTATE_STOP )
			{
			//It is -> free resources.
				//pin
				pChannel_midi->Set_AVStream_Pin( NULL );

				//typeClient -> release it.
				pChannel_midi->m_Type_client = channel_midi_c::CHANNEL_TYPE_CLIENT_NON;
			}
			else
			{
				Status = STATUS_INVALID_DEVICE_STATE;
			}

	//Release SPinLock.
		KeReleaseSpinLock( pSpinLock, spinLock_oldLevel );
	}

#ifdef DBG_AND_LOGGER
	Procedure_Exit( Status );
#endif
	return Status;
}
