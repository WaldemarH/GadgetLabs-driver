/*
*****************************************************************************
*****************************************************************************
**
**                 GADGET LABS INC PROPRIETARY INFORMATION
**
** This software is supplied under the terms of a license agreement or
** nondisclosure agreement with Gadget Labs Inc and may not be copied or
** disclosed except in accordance with the terms of that agreement.
**
**        Copyright (C) 1999 Gadget Labs Inc.  All Rights Reserved.
**
*****************************************************************************
*****************************************************************************
**
** MODULE:
**		AK4524
**
** PURPOSE:
**		Contains bit defs for registers in AK4524 codec.
**
** CONTENTS:
**		
**
** NOTES: 
**		* Tabstops are set to 4 and 80 character limit per source line.
**		* Code for input/output volume registers will be calc in driver
**		  if they are used.
**
** REVISION:
**		$Revision: 1.0 $
**			$Date: Wed Jun 16 14:20:14 1999 $
**		 $Modtime$
** 
*****************************************************************************
*****************************************************************************
*/

#pragma once

/***************************************************************************/
/*******************************  REGISTERS  *******************************/
/***************************************************************************/

#define AK_PD				0			// Power down register
#define AK_RST				1			// Reset register
#define AK_CLK				2			// Clock and format control register
#define AK_EMVC				3			// De-emphasis and colume register
#define AK_IGL				4			// ADC input gain left register
#define AK_IGR				5			// ADC input gain right register
#define AK_OGL				6			// DAC output gain left register
#define AK_OGR				7			// DAC output gain right register

/***************************************************************************/
/******************************  POWER DOWN  *******************************/
/***************************************************************************/

#define AK_PD_PWDA			0x01		// DAC power down
#define AK_PD_PWAD			0x02		// ADC power down
#define AK_PD_PWVR			0x04		// VRef power down


/***************************************************************************/
/********************************  RESET  **********************************/
/***************************************************************************/

#define AK_RST_DAC			0x01		// Reset DAC
#define AK_RST_ADC			0x02		// Reset ADC


/***************************************************************************/
/*************************  CLOCK AND FORMAT  ******************************/
/***************************************************************************/

#define AK_CLK_DFS0			0x01		// Sampling rate control
#define AK_CLK_DFS1			0x02		// Sampling rate control
#define AK_CLK_DFS_MASK		(AK_CLK_DFS1|AK_CLK_DFS0)

#define AK_CLK_FSD			0x00		// Srate normal (default)
#define AK_CLK_FSN2			0x01		// Srate 2 * FSN
#define AK_CLK_FSN4S		0x02		// Srate 4 * FSN simple decimation
#define AK_CLK_FSN4T		0x03		// Srate 4 * FSN 2 tap filter


#define AK_CLK_CKS0			0x04		// Master clock frequency
#define AK_CLK_CKS1			0x08		// Master clock frequency
#define AK_CLK_CMODE		0x10		// Master clock frequency
#define AK_CLK_CK_MASK		(AK_CLK_CMODE|AK_CLK_CKS1|AK_CLK_CKS0)

#define AK_CLK_256FSN		0x00		// Master clock frequency 256X
#define AK_CLK_512FSN		0x04		// Master clock frequency 512X
#define AK_CLK_1024FSN		0x08		// Master clock frequency 1024X
#define AK_CLK_384FSN		0x10		// Master clock frequency 384X
#define AK_CLK_768FSN		0x14		// Master clock frequency 768X


#define AK_CLK_DIF0			0x20		// Audio data interface mode
#define AK_CLK_DIF1			0x40		// Audio data interface mode
#define AK_CLK_DIF2			0x80		// Audio data interface mode
#define AK_CLK_DIF_MASK		(AK_CLK_DIF2|AK_CLK_DIF1|AK_CLK_DIF0);

#define AK_CLK_FMT0			0x00		// Out: 24 bit MSB, In: 16 bit LSB
#define AK_CLK_FMT1			0x20		// Out: 24 bit MSB, In: 20 bit LSB
#define AK_CLK_FMT2			0x40		// Out: 24 bit MSB, In: 24 bit MSB
#define AK_CLK_FMT_I2S		0x60		// Out: 24 bit I2S, In: 24 bit I2S
#define AK_CLK_FMT4			0x80		// Out: 24 bit MSB, In: 24 bit LSB


/***************************************************************************/
/***********************  DE-EMPHISIS AND VOLUME  **************************/
/***************************************************************************/

#define AK_EMVC_DEM0		0x01		// De-emphasis
#define AK_EMVC_DEM1		0x02		// De-emphasis

#define AK_EMVC_EM44		0x00		// De-emphasis for 44.1 kHz
#define AK_EMVC_EM_OFF		0x01		// De-emphasis off
#define AK_EMVC_EM48		0x02		// De-emphasis for 48 kHz
#define AK_EMVC_EM32		0x03		// De-emphasis for 32 kHz


#define AK_EMVC_ZTM0		0x04		// Zero crossing time out delay
#define AK_EMVC_ZTM1		0x08		// Zero crossing time out delay
#define AK_EMVC_ZCEI		0x10		// Zero crossing enabled

#define AK_EMVS_256S_TO		0x00		// Volume zero crossing timeout 256
#define AK_EMVS_512S_TO		0x04		// Volume zero crossing timeout 512
#define AK_EMVS_1024S_TO	0x08		// Volume zero crossing timeout 1024
#define AK_EMVS_2048S_TO	0x0C		// Volume zero crossing timeout 2048

#define AK_EMVC_SMUTE		0x80		// Soft mute

/***************************************************************************/
/**********************************  MISC  *********************************/
/***************************************************************************/

/* These are not register definitions but define codecs and channels. */

#define AK_CODEC_0			0
#define AK_CODEC_1			1

#define AK_MUTE_LEFT		1
#define AK_MUTE_RIGHT		2
#define AK_MUTE_BOTH		(AK_MUTE_LEFT | AK_MUTE_RIGHT)

