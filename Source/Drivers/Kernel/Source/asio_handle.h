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
#include "linked_array.h"
#include "channel_stereo.h"


struct ioctl_asio_channels_header_s;
struct ioctl_asio_start_header_s;

class asioHandle_c : public linkedArray_c
{
	friend class driver_c;

//Constructor/Destructor...
	public: asioHandle_c( asioHandle_c* pPrevious );
	public: virtual ~asioHandle_c();

//Buffers memory...
	public: NTSTATUS Memory_ValidateSize( unsigned long size_memory );

	public: _MDL*						m_pMDL;
	public: unsigned long*				m_pMemory_locked;

//Channels...
	public: void Channels_CloseAll();
	public: NTSTATUS Channels_OpenAll( ioctl_asio_channels_header_s* pHeader );

	public: void Channels_SetBuffers( unsigned long* pBuffers );

	public: NTSTATUS Channels_Start( ioctl_asio_start_header_s* pHeader );
	public: void Channels_Stop();

	private: channel_stereo_c*			m_pChannels_Stereo_INs[MAX_CARDS*MAX_STEREO_PAIRS_PER_CARD+1];		//NULL defines the end
	private: channel_stereo_c*			m_pChannels_Stereo_OUTs[MAX_CARDS*MAX_STEREO_PAIRS_PER_CARD+1];		//NULL defines the end

//Handle...
	public: NTSTATUS Handle_CreateNew( asioHandle_c** ppHandle );
	public: NTSTATUS Handle_Remove( unsigned long handleID );

	public: asioHandle_c* GetHandle( unsigned long handleID );
	public: unsigned long GetHandleID();

	private: unsigned long				m_HandleID;
	private: unsigned long				m_HandleID_last;		//only for master class

//Pending IRP...
	public: PIRP						m_pIRP_pending;

//Switch event.
	public: PKEVENT						m_pSwitchEvent;

//Transfer data.
	public: void TransferData_Input( ULONGLONG clock_samples, unsigned long clock_samplesLost );
	public: void TransferData_Output();
};
