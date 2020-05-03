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



class channel_base_c;

class syncStart_c : public linkedArray_c
{
	public: syncStart_c( syncStart_c* pPrevious );
	public: ~syncStart_c();

//Linking...
	public: NTSTATUS FreeSyncStart( syncStart_c** ppSyncStart );
	public: NTSTATUS GetSyncStart( syncStart_c** ppSyncStart, channel_base_c* pChannel );

//Channels...
	public: NTSTATUS AddReference( channel_base_c* pChannel );
	public: NTSTATUS RemoveReference( channel_base_c* pChannel );

	public: NTSTATUS Channel_FromRunState();
	public: NTSTATUS Channel_ToRunState();

	public: bool Channels_StartStreaming();

	private: unsigned long			m_Channels_nReferences;
	private: unsigned long			m_Channels_nInRunningState;

	public: channel_base_c*			m_ppChannels[MAX_CHANNELS_STEREO+1];		//+1 for easier array handling
};

