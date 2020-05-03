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
//      File defines channel class. It's intention is to handle all channel
//      related stuff.
//
// STATUS:
//      Unfinished.
//
//***************************************************************************
//***************************************************************************

#pragma once

#define MIDI_COMMAND_BORDER								0x80

//System Common Messages
#define MIDI_COMMAND_SYSTEM_COMMON_EXCLUSIVE_START		0xF0
#define MIDI_COMMAND_SYSTEM_COMMON_MIDI_TIME_CODE		0xF1
#define MIDI_COMMAND_SYSTEM_COMMON_SONG_POSITION		0xF2
#define MIDI_COMMAND_SYSTEM_COMMON_SONG_SELECT			0xF3
#define MIDI_COMMAND_SYSTEM_COMMON_UNDEFINED_1			0xF4
#define MIDI_COMMAND_SYSTEM_COMMON_UNDEFINED_2			0xF5
#define MIDI_COMMAND_SYSTEM_COMMON_TUNE_REQUEST			0xF6
#define MIDI_COMMAND_SYSTEM_COMMON_EXCLUSIVE_END		0xF7

//System Real-Time Messages
#define MIDI_COMMAND_SYSTEM_REALTIME_TIMING_CLOCK		0xF8
#define MIDI_COMMAND_SYSTEM_REALTIME_UNDEFINED_3		0xF9
#define MIDI_COMMAND_SYSTEM_REALTIME_START				0xFA
#define MIDI_COMMAND_SYSTEM_REALTIME_CONTINUE			0xFB
#define MIDI_COMMAND_SYSTEM_REALTIME_STOP				0xFC
#define MIDI_COMMAND_SYSTEM_REALTIME_UNDEFINED_4		0xFD
#define MIDI_COMMAND_SYSTEM_REALTIME_ACTIVE_SENSING		0xFE
#define MIDI_COMMAND_SYSTEM_REALTIME_RESET				0xFF


