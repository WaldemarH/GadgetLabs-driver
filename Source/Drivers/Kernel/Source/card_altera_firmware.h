//***************************************************************************
//***************************************************************************
//
//                          PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license agreement or
// nondisclosure agreement with Waldemar Haszlakiewicz and may not be
// copied or disclosed except in accordance with the terms of that agreement.
//
//               Copyright (C) 2009 Waldemar Haszlakiewicz
//                            All Rights Reserved.
//
//***************************************************************************
//***************************************************************************
// Notes:
// ( ) means value after reset.

#pragma once


//***************************************************************************
//Notice:
//When the system is reading samples from card it will get a 32bit values.
//Now the upper 8bits will be set according to bit 24 of the 24bit sample ->
//if bit 24 is 0 then upper 8 bits will be 0,
//if bit 24 is 1 then upper 8 bits will be 1.
//***************************************************************************


//***************************************************************************
//**  REGISTERS  ************************************************************
//***************************************************************************
#define SC_DEVICES					0x00
#define SC_ENGINE					0x02
#define SC_SAMPLE_COUNTER			0x04
#define SC_INTERRUPT_DOWNCOUNTER	0x06
#define SC_INTERRUPT				0x08
#define SC4_GAIN_MON				0x0A
#define SC8_GAIN					0x0A
#define SC8_MON_MUTE				0x0E

#define SC_IO_PORT_SIZE				64	//64 bytes

//***************************************************************************
//**  SC_DEVICES  ***********************************************************
//***************************************************************************
// The register contains connections for daughter card, PLL and CODECs
// programing.
//
//4xx cards:
// bit15 - R  - (0)
// bit14 - R  - (1) - Daughter card present(LOW - present, HIGH - not present).
// bit13 - RW - (0) - DC_DATA_BIT -> OUTput -> data bit signal 
// bit12 - RW - (0) - DC_DATA_CLK -> OUTput -> data clock signal
// bit11 - R  - (x) - DC_CONFIG_DONE -> INput -> reports configuration status
// bit10 - R  - (x) - nDC_STATUS -> INput -> reports error in programing
// bit9  - RW - (1) - nDC_CONFIG -> OUTput -> signals start of programing
// bit8  - RW - (0) - PLL_SET_DATA_PIN_TO_HIGH_Z
// bit7  - RW - (1) - PLL_INTERFACE_DATA -> INput/OUTput -> data bit signal
// bit6  - RW - (1) - PLL_INTERFACE_CLK -> OUTput -> data clock signal 
// bit5  - RW - (0) - CODEC_POWER_UP -> OUTput -> powers up/down both CODECs
// bit4  - RW - (0) - CODEC_CS1 -> OUTput -> CODEC1 chip select
// bit3  - RW - (0) - CODEC_CS2 -> OUTput -> CODEC2 chip select
// bit2  - RW - (0) - CODEC_CONTROL_IF -> OUTput -> IF signal
// bit1  - RW - (1) - CODEC_CONTROL_CLK -> OUTput -> data clock signal
// bit0  - RW - (0) - CODEC_CONTROL_DATA -> OUTput -> data bit signal
//
//Notice:
//Codecs work in slave mode by default.
//
//824 cards:
// bit15 - R  - (0)
// bit14 - R  - (1) - Daughter card present(LOW - present, HIGH - not present).
// bit13 - RW - (0) - DC_DATA_BIT -> OUTput -> data bit signal 
// bit12 - RW - (0) - DC_DATA_CLK -> OUTput -> data clock signal
// bit11 - R  - (x) - DC_CONFIG_DONE -> INput -> reports configuration status
// bit10 - R  - (x) - nDC_STATUS -> INput -> reports error in programing
// bit9  - RW - (1) - nDC_CONFIG -> OUTput -> signals start of programing
// bit8  - R  - (0)
// bit7  - R  - (0)
// bit6  - R  - (0)
// bit5  - R  - (0)
// bit4  - R  - (0)
// bit3  - R  - (0)
// bit2  - R  - (0)
// bit1  - R  - (0)
// bit0  - R  - (0)
//

#define SC_nDC_PRSNT					0x4000
#define SC_DC_DATA_BIT					0x2000		// R/W
#define SC_DC_DATA_CLK					0x1000		// R/W
#define SC_DC_CONF_DONE					0x0800		// R
#define SC_DC_nSTATUS					0x0400		// R
#define SC_DC_nCONFIG					0x0200		// R/W

#define SC4_PLL_SET_DATA_PIN_TO_HIGH_Z	0x0100		// R/W
#define SC4_PLL_INTERFACE_DATA			0x0080		// R/W
#define SC4_PLL_INTERFACE_CLK			0x0040		// R/W

#define SC4_CODEC_POWER_UP				0x0020		// R/W
#define SC4_CODEC_CS1					0x0010		// R/W
#define SC4_CODEC_CS2					0x0008		// R/W
#define SC4_CODEC_CONTROL_IF			0x0004		// R/W
#define SC4_CODEC_CONTROL_CLK			0x0002		// R/W
#define SC4_CODEC_CONTROL_DATA			0x0001		// R/W

//***************************************************************************
//**  SC_ENGINE  ************************************************************
//***************************************************************************
//Register contains audio engine parameters (sample rate, reset flags).
//
// bit15 - R  - (0)
// bit14 - R  - (0)
// bit13 - R  - (0)
// bit12 - R  - (0)
// bit11 - R  - (0)
// bit10 - R  - (0)
// bit9  - R  - (0)
// bit8  - R  - (0)
// bit7  - R  - (0)
// bit6  - WR - (0) - Reset clock detectors( it will reset bit 2 and 3 ).
//                    Usage:
//                    This bit flag should be set to 1, wait for 1us and set back to 0.
//                    Wait another 10us and test the clock present bits.
//
// bit5  - WR - (0) - Release counters from reset state.
//
//                    This flag will allow counter to start running.
//                    Or if we look it at another way this is a
//                    negative going reset state
//                    ( 0 -> reset/freeze, 1 - counters can run ).
//
//                    Notice:
//                    This flag resets the 'audio engine' interrupt flag also.
//
// bit4  - WR - (0) - Start/Stop audio engine(clocks).
//
//                    This flag disables MasterClockOutput so that any
//                    gliches from PLL programming wont cause any problems.
//                    When set to 1 Altera circuit will start the clock glich free.
//
//                    This flag also disconnects PLL clocks from audio engine.
//                    (if set to 0 -> disconnect, 1 - connect.)
//
//                    Extra notes:
//                    The flag (Start/Stop meaning) has connection to
//                    daughter board so the daughter board is Started/Stopped
//                    synchronized to main board.
//                    But you have to reset slave cards.
//
// bit3  -  R - (x) - syncIn clock present
// bit2  -  R - (x) - daughter card clock present (don't test this if daughter card is
//                    not installed (the noise in the system can act as a clock
//                    (can trigger a clock event))
// bit1  - WR - (0) - \
// bit0  - WR - (0) - /-> sample rate selector
//
// Notes:
// ( ) means value after reset.
//
// Sample rate selector (code has automatic mux selector/deselector) for 4xx cards:
// value (bit1,bit0)   sampleRate
// 00                  PLL_A (first pll(A) from the PLL chip) (default setting)
// 01                  PLL_A (first pll(A) from the PLL chip) (default setting)
// 10                  daughter card clock (digital signal clock)
// 11                  use syncIn clock
//
// Sample rate selector (code has automatic mux selector/deselector) for 824 cards:
// value (bit1,bit0)   sampleRate
// 00                  44.1kHz (default setting)
// 01                  48kHz
// 10                  daughter card clock (digital signal clock)
// 11                  use syncIn clock
//
// To change the sample rate do:
// 1) Stop master card -> bit4 to 0.
//
//    This will cause the masterCard clock to stop glichFree (the clock
//    will stop in LOW position).
//    And because the master card clock will be stopped all the
//    slave cards will stop too.
//
//    Delay 1us.
//
// 2) Set new sampleRate.
//
// 3) Reset all cards counters -> bit5 to 0 on all cards.
//    (To reset counters you need to clear the flag, wait a 1us and set it
//     back to 1. )
//
// 4) Set the master card interrupt downcounter value to bufferSize+1.
//    Delay 1us.
//
// 5) Start master card -> bit4 to 1.
//    This will start masterCard and slave cards glichFree.
//
// NOTICE:
// On this start->stop->start period the 824 converters will be reset automatically and
// 4xx CODECs will be stopped and as they don't like to be stopped for a long time this
// procedure should try to be as fast as it can(well just programming of the PLL will take
// a few ms).
//
#define SC_RELEASE_COUNTERS					0x0020
#define SC_START_AUDIO_ENGINE				0x0010

#define SC_RESET_CLOCK_DETECTOR				0x0040
#define SC_SYNC_IN_CLOCK_PRESENT			0x0008
#define SC_DAUGHTER_CARD_CLOCK_PRESENT		0x0004

#define SC_SAMPLE_RATE_4xx_PLL_A			0x0000
#define SC_SAMPLE_RATE_824_44100Hz			0x0000
#define SC_SAMPLE_RATE_824_48000Hz			0x0001
#define SC_SAMPLE_RATE_DAUGHTER_CARD		0x0002
#define SC_SAMPLE_RATE_SYNC_IN				0x0003
#define SC_SAMPLE_RATE_MASK					0x0003

//***************************************************************************
//**  SC_SAMPLE_COUNTER(SC_ADDRESS)  ****************************************
//***************************************************************************
//This variable is a common 16bit wide up counter.
//Now this value can be used for a channel address(this is how it's done in
//hardware.
//So the counter is reset when SC_RELEASE_COUNTERS (SC_ENGINE) goes low.
//
//Now to get the channel address all we need to do is mask out the upper
//4bits(4xx cards) or upper 5bits(824 cards).
//
// bit15 -  R - (0) \
// bit14 -  R - (0) |
// bit13 -  R - (0) |
// bit12 -  R - (0) |
// bit11 -  R - (0) |
// bit10 -  R - (0) |
// bit9  -  R - (0) |
// bit8  -  R - (0) |->address of mono channel
// bit7  -  R - (0) |
// bit6  -  R - (0) |
// bit5  -  R - (0) |
// bit4  -  R - (0) |
// bit3  -  R - (0) |
// bit2  -  R - (0) |
// bit1  -  R - (0) |
// bit0  -  R - (0) /
//
// Buffer layout:
// 4xx cards:
// offset    channel
// 0x0000    input1left
// 0x1000    input1right
// 0x2000    input2left
// 0x3000    input2right
// 0x4000    output1left
// 0x5000    output1right
// 0x6000    output2left
// 0x7000    output2right
//
// 824 cards:
// offset    channel
// 0x0000    input1left
// 0x0800    input1right
// 0x1000    input2left
// 0x1800    input2right
// 0x2000    input3left
// 0x2800    input3right
// 0x3000    input4left
// 0x3800    input5right
// 0x4000    output1left
// 0x4800    output1right
// 0x5000    output2left
// 0x5800    output2right
// 0x6000    output3left
// 0x6800    output3right
// 0x7000    output4left
// 0x7800    output4right
//
// This layout is made with only one purpose in mind ->
// as fast ASIO transfer as it can get.
#define SC4_INPUT_CH1		0x0000	//samples
#define SC4_INPUT_CH2		0x1000
#define SC4_INPUT_CH3		0x2000
#define SC4_INPUT_CH4		0x3000
#define SC4_OUTPUT_CH1		0x4000
#define SC4_OUTPUT_CH2		0x5000
#define SC4_OUTPUT_CH3		0x6000
#define SC4_OUTPUT_CH4		0x7000

#define SC8_INPUT_CH1		0x0000	//samples
#define SC8_INPUT_CH2		0x0800
#define SC8_INPUT_CH3		0x1000
#define SC8_INPUT_CH4		0x1800
#define SC8_INPUT_CH5		0x2000
#define SC8_INPUT_CH6		0x2800
#define SC8_INPUT_CH7		0x3000
#define SC8_INPUT_CH8		0x3800
#define SC8_OUTPUT_CH1		0x4000
#define SC8_OUTPUT_CH2		0x4800
#define SC8_OUTPUT_CH3		0x5000
#define SC8_OUTPUT_CH4		0x5800
#define SC8_OUTPUT_CH5		0x6000
#define SC8_OUTPUT_CH6		0x6800
#define SC8_OUTPUT_CH7		0x7000
#define SC8_OUTPUT_CH8		0x7800

#define SC4_MONO_CHANNEL_SIZE		0x1000	//samples
#define SC8_MONO_CHANNEL_SIZE		0x0800	//samples

#define SC4_ADDRESS_MASK			0x0FFF	//samples
#define SC8_ADDRESS_MASK			0x07FF	//samples


#define SC_WAVE_BUFFER_SIZE			0x8000	//32k of samples
#define SC_ADAT_BUFFER_SIZE			0x4000	//16k of samples
#define SC_SPDIF_BUFFER_SIZE		0x2000	//8k of samples

//***************************************************************************
//**  SC_INTERRUPT_DOWNCOUNTER  *********************************************
//***************************************************************************
// It's a downcounter which will signal audioEngine interrupt when
// it commes to 0.
// Because of the hardware design the new value should be:
// bufferSize + 1 - (channel address & (bufferSize-1))
//
// Ex.:
// At start (when the streaming starts):
// bufferSize = 0x100
// channel address = 0
// new value = bufferSize + 1 = 0x101
//
// In between:
// bufferSize = 0x100
// channel address = 0x502
// new value = bufferSize + 1 - (channel address & (bufferSize-1)) =
//           = 0x100 + 1 - (0x502 & 0xFF) = 0x100 + 1 - 2 = 0x0FF
//
// bit15 - R  - (0)
// bit14 - R  - (0)
// bit13 - R  - (0)
// bit12 - R  - (0)
// bit11 - R  - (0)
// bit10 - R  - (0)
// bit9  - W  - (x) - \
// bit8  - W  - (x) - |
// bit7  - W  - (x) - |
// bit6  - W  - (x) - |
// bit5  - W  - (x) - |-> new value
// bit4  - W  - (x) - |
// bit3  - W  - (x) - |
// bit2  - W  - (x) - |
// bit1  - W  - (x) - |
// bit0  - W  - (x) - /
//
// Notes:
// ( ) means value after reset.
//


//***************************************************************************
//**  SC_INTERRUPT  *********************************************************
//***************************************************************************
//Interrupt flags.
//
// bit15 -  R - (0)
// bit14 -  R - (0)
// bit13 -  R - (0)
// bit12 -  R - (0)
// bit11 -  R - (0)
// bit10 -  R - (0)
// bit9  -  R - (0)
// bit8  -  R - (0)
// bit7  -  R - (0)
// bit6  -  R - (0)
// bit5  -  R - (0) - MIDI interrupt flag
// bit4  -  R - (0) - daughter card card error flag
// bit3  -  R - (0) - audio engine interrupt flag (the current buffer is full (or it was read))
// bit2  - WR - (0) - enable MIDI interrupt
// bit1  - WR - (0) - enable daughter card error interrupt 
// bit0  - WR - (0) - enable audio engine interrupt
//
// If enable bit isn't set even if the source will signal an interrupt the
// flags wont go high.
//
// To test if the interrupt came from the card the user should read the
// bit2 of the PLX 'Interrupt Control/Status Register'.
 
#define SC_UART_INT_ISSUED				0x0020
#define SC_DAUGHTER_CARD_INT_ISSUED		0x0010
#define SC_AUDIO_ENGINE_INT_ISSUED		0x0008

#define SC_ENABLE_UART_INT				0x0004
#define SC_ENABLE_DAUGHTER_CARD_INT		0x0002
#define SC_ENABLE_AUDIO_ENGINE_INT		0x0001


//***************************************************************************
//**  SC4_GAIN_MON  *********************************************************
//***************************************************************************
//Gain, monitor and mute register for 4xx cards.
//
// bit15 - R  - (0)
// bit14 - R  - (0)
// bit13 - R  - (0)
// bit12 - R  - (0)
// bit11 - R  - (0)
// bit10 - R  - (0)
// bit9  - R  - (0)
// bit8  - R  - (0)
// bit7  - R  - (0)
// bit6  - RW - (0) - Input2right monitor -> 0 = disabled, 1 = enabled
// bit5  - RW - (0) - Input2left monitor -> 0 = disabled, 1 = enabled
// bit4  - RW - (0) - Input1right monitor -> 0 = disabled, 1 = enabled
// bit3  - RW - (0) - Input1left monitor -> 0 = disabled, 1 = enabled
// bit2  - RW - (0) - Input gain_B set -> 0 = -10dBv, 1 = +4dBu
// bit1  - RW - (0) - Input gain_A set -> 0 = -10dBv, 1 = +4dBu
// bit0  - RW - (0) - Output gain set -> 0 = -10dBv, 1 = +4dBu
//
//Notice:
//On 424 cards:
//- output gain (bit0) should be set to 0
//- input gain_A (bit1) should be set to 0
//- input gain_B (bit2) acts as GAIN_IN flag
//
//On 496 cards:
//- output gain (bit0) acts as GAIN_OUT flag
//- input gain_A (bit1) acts as GAIN_IN flag
//- input gain_B (bit2) should be set to 0
//
#define SC4_MONITOR_CHANNEL_4	0x0040
#define SC4_MONITOR_CHANNEL_3	0x0020
#define SC4_MONITOR_CHANNEL_2	0x0010
#define SC4_MONITOR_CHANNEL_1	0x0008

#define SC4_INPUT_GAIN_B		0x0004
#define SC4_INPUT_GAIN_A		0x0002
#define SC4_OUTPUT_GAIN			0x0001

#define SC424_INPUT_GAIN		SC4_INPUT_GAIN_B

#define SC496_OUTPUT_GAIN		SC4_OUTPUT_GAIN
#define SC496_INPUT_GAIN		SC4_INPUT_GAIN_A


//***************************************************************************
//**  SC8_GAIN  *************************************************************
//***************************************************************************
//Gain register for 824 cards.
//
// bit15 - RW - (0) - Output channel 8 -> 0 = -10dBv, 1 = +4dBu
// bit14 - RW - (0) - Output channel 7 -> 0 = -10dBv, 1 = +4dBu
// bit13 - RW - (0) - Output channel 6 -> 0 = -10dBv, 1 = +4dBu
// bit12 - RW - (0) - Output channel 5 -> 0 = -10dBv, 1 = +4dBu
// bit11 - RW - (0) - Output channel 4 -> 0 = -10dBv, 1 = +4dBu
// bit10 - RW - (0) - Output channel 3 -> 0 = -10dBv, 1 = +4dBu
// bit9  - RW - (0) - Output channel 2 -> 0 = -10dBv, 1 = +4dBu
// bit8  - RW - (0) - Output channel 1 -> 0 = -10dBv, 1 = +4dBu
// bit7  - RW - (0) - Input channel 8 -> 0 = -10dBv, 1 = +4dBu
// bit6  - RW - (0) - Input channel 7 -> 0 = -10dBv, 1 = +4dBu
// bit5  - RW - (0) - Input channel 6 -> 0 = -10dBv, 1 = +4dBu
// bit4  - RW - (0) - Input channel 5 -> 0 = -10dBv, 1 = +4dBu
// bit3  - RW - (0) - Input channel 4 -> 0 = -10dBv, 1 = +4dBu
// bit2  - RW - (0) - Input channel 3 -> 0 = -10dBv, 1 = +4dBu
// bit1  - RW - (0) - Input channel 2 -> 0 = -10dBv, 1 = +4dBu
// bit0  - RW - (0) - Input channel 1 -> 0 = -10dBv, 1 = +4dBu

#define SC8_OUTPUT_GAIN_CH_8	0x8000
#define SC8_OUTPUT_GAIN_CH_7	0x4000
#define SC8_OUTPUT_GAIN_CH_6	0x2000
#define SC8_OUTPUT_GAIN_CH_5	0x1000
#define SC8_OUTPUT_GAIN_CH_4	0x0800
#define SC8_OUTPUT_GAIN_CH_3	0x0400
#define SC8_OUTPUT_GAIN_CH_2	0x0200
#define SC8_OUTPUT_GAIN_CH_1	0x0100

#define SC8_INPUT_GAIN_CH_8		0x0080
#define SC8_INPUT_GAIN_CH_7		0x0040
#define SC8_INPUT_GAIN_CH_6		0x0020
#define SC8_INPUT_GAIN_CH_5		0x0010
#define SC8_INPUT_GAIN_CH_4		0x0008
#define SC8_INPUT_GAIN_CH_3		0x0004
#define SC8_INPUT_GAIN_CH_2		0x0002
#define SC8_INPUT_GAIN_CH_1		0x0001


//***************************************************************************
//**  SC4_BUS_WAIT_TIMER  ***************************************************
//***************************************************************************
//This is a 16bit counter which shows the maximum PCI cycles that
//the transfer engine had to wait for local bus.
//All this is made only because there is a hardware error or a misfeature.
//The thing is that PLX(PCI communication) chip can't break the transfer if
//system is writting to the card.
//This means that the Altera chip can't access the card's memory and this is
//not good.
//
//
//Function to calculate the number of PCI cycles the engine can wait:
//
//1/sampleRate - 2/(96*sampleRate) - 21/33MHz = x/33MHz
//94/(96*sampleRate) - 21/33MHz = x/33MHz
//94/96 * (33MHz/sampleRate) - 21 = x
//
//X = 94/96 * (33MHz/sampleRate) - 21
//
//sampleRate	X
//44100Hz		711
//48000Hz		652
//88200Hz		345
//96000Hz		315
//
//The upper values for X are ideal values so we should never come even close,
//if we want a stable functioning of Altera code. So the values should be below
//the next ones:
//
//sampleRate	X
//44100Hz		687
//48000Hz		629
//88200Hz		325
//96000Hz		296
//
//The exact values that work on every system must be figured out with testing as
//different systems behave differently.
//example:
//If driver transfers 32 samples and then reads 1 sample on my system the
//maximum wait time is 200 PCI cycles.
//Now in theory it should only be 64 + a few PCI cycles as the PLX chip should only
//transfer 32 samples (every transfer takes 2 PCI cycles on local bus), but
//we all know that theory and practice are 2 totally different things.
//(So this 32 samples transfer can work with 96kHz sample rates too.)
//
//Now 824 cards behave similar, but Altera must transfer samples on every half
//of the sample rate. But this is not a problem as the maximum sample rate frequency
//is half of the 4xx cards. So the timings are similar.
//Notice:
//This register is not available on 824 cards as there was not enough space in
//Altera to fit all the function into it.
//
//NOTICE:
//Do not use the same technique as with the old driver where you had different
//settings for every sampleRate and the values were loaded from registry.
//The user shouldn't be able to set those values.
//If you want you can set the settings in driver, but don't load any values
//from registry!!
//
//NOTICE2:
//The transfer size MUST BE far away from the limits as Altera doesn't have any
//protections agains end of the transfer time. As the Altera engines work in
//parallel, strange things can happen and even some transistors in Altera can
//burn out if those engines get access to local bus at the same time.
//
#define TRANSFER_SIZE	32


//***************************************************************************
//**  SC8_MON_MUTE  *********************************************************
//***************************************************************************
//Monitor and mute register for 824 cards.
//
// bit15 - RW - (0) - Mute channel 8 -> 0 = off, 1 = on
// bit14 - RW - (0) - Mute channel 7 -> 0 = off, 1 = on
// bit13 - RW - (0) - Mute channel 6 -> 0 = off, 1 = on
// bit12 - RW - (0) - Mute channel 5 -> 0 = off, 1 = on
// bit11 - RW - (0) - Mute channel 4 -> 0 = off, 1 = on
// bit10 - RW - (0) - Mute channel 3 -> 0 = off, 1 = on
// bit9  - RW - (0) - Mute channel 2 -> 0 = off, 1 = on
// bit8  - RW - (0) - Mute channel 1 -> 0 = off, 1 = on
// bit7  - RW - (0) - Monitor channel 8 -> 0 = disabled, 1 = enabled
// bit6  - RW - (0) - Monitor channel 7 -> 0 = disabled, 1 = enabled
// bit5  - RW - (0) - Monitor channel 6 -> 0 = disabled, 1 = enabled
// bit4  - RW - (0) - Monitor channel 5 -> 0 = disabled, 1 = enabled
// bit3  - RW - (0) - Monitor channel 4 -> 0 = disabled, 1 = enabled
// bit2  - RW - (0) - Monitor channel 3 -> 0 = disabled, 1 = enabled
// bit1  - RW - (0) - Monitor channel 2 -> 0 = disabled, 1 = enabled
// bit0  - RW - (0) - Monitor channel 1 -> 0 = disabled, 1 = enabled
//
#define SC8_MUTE_CH_8			0x8000
#define SC8_MUTE_CH_7			0x4000
#define SC8_MUTE_CH_6			0x2000
#define SC8_MUTE_CH_5			0x1000
#define SC8_MUTE_CH_4			0x0800
#define SC8_MUTE_CH_3			0x0400
#define SC8_MUTE_CH_2			0x0200
#define SC8_MUTE_CH_1			0x0100

#define SC8_MONITOR_CH_8		0x0080
#define SC8_MONITOR_CH_7		0x0040
#define SC8_MONITOR_CH_6		0x0020
#define SC8_MONITOR_CH_5		0x0010
#define SC8_MONITOR_CH_4		0x0008
#define SC8_MONITOR_CH_3		0x0004
#define SC8_MONITOR_CH_2		0x0002
#define SC8_MONITOR_CH_1		0x0001


