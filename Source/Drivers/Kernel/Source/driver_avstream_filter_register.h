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


//--------------------------------------------------------------------------
//Topology
//
//Pins->this are serial numbers of PINS defined in PinDescriptor_Wave.
enum
{
	WAVEIN_12			= 0,
	WAVEIN_12_BRIDGE	= 1,
	WAVEOUT_12			= 2,
	WAVEOUT_12_BRIDGE	= 3
};

//Nodes->this are serial numbers of NODEs defined in KsNodeDescriptors
enum
{
	ADC_NODE_12		= 0,
	DAC_NODE_12		= 1
};
//Node pins.
//e.g.
//                  |   (1)------------ (0)  |
//  (Sink Pin) 1 o--|-->o--|DAC Node 0|--o>--|--o 3 (Bridge Pin)
//                  |      ------------      |
enum
{
	NODE_PIN_OUTPUT		= 0,
	NODE_PIN_INPUT		= 1
};


//--------------------------------------------------------------------------
//Topology
//
//enum midiPins_e
//{
//	MIDI_IN_MME = 0,
//	MIDI_IN_DX,
//	MIDI_IN_BRIDGE,
//	MIDI_OUT_MME,
//	MIDI_OUT_DX,
//	MIDI_OUT_BRIDGE
//};
enum midiPins_e
{
	MIDI_IN_MME = 0,
	//MIDI_IN_DX,
	MIDI_IN_BRIDGE,
	MIDI_OUT_MME,
	//MIDI_OUT_DX,
	MIDI_OUT_BRIDGE
};

//Nodes->this are serial numbers of NODEs defined in KsNodeDescriptors
enum
{
	SYNTH_NODE_0 = 0,
	SYNTH_NODE_1
};
