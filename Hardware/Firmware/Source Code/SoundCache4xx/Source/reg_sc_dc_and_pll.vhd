--***************************************************************************
--***************************************************************************
--
--                          PROPRIETARY INFORMATION
--
-- This software is supplied under the terms of a license agreement or
-- nondisclosure agreement with Waldemar Haszlakiewicz and may not be
-- copied or disclosed except in accordance with the terms of that agreement.
--
--               Copyright (C) 2002, 2003 Waldemar Haszlakiewicz
--                            All Rights Reserved.
--
--***************************************************************************
--***************************************************************************

-- It defines FPGA code version and it contains connections for daughter card
-- FPGA programing.
--
-- bit15 - R  - (0)
-- bit14 - R  - (0)
-- bit13 - RW - (0) - DC_DATA_BIT -> OUTput -> data bit signal 
-- bit12 - RW - (0) - DC_DATA_CLK -> OUTput -> data clock signal
-- bit11 - R  - (x) - DC_CONFIG_DONE -> INput -> reports configuration status
-- bit10 - R  - (x) - nDC_STATUS -> INput -> reports error in programing
-- bit9  - RW - (1) - nDC_CONFIG -> OUTput -> signals start of programing
-- bit8  - WR - (0) - PLL_SET_DATA_PIN_TO_HIGH_Z
-- bit7  - WR - (1) - PLL_INTERFACE_DATA -> INput/OUTput -> data bit signal
-- bit6  - WR - (1) - PLL_INTERFACE_CLK -> OUTput -> data clock signal 
-- bit5  - WR - (0) - CODEC_POWER_UP -> OUTput -> power up/down both CODECs
-- bit4  - WR - (0) - CODEC_CS1 -> OUTput -> CODEC1 chip select
-- bit3  - WR - (0) - CODEC_CS2 -> OUTput -> CODEC2 chip select
-- bit2  - WR - (0) - CODEC_CONTROL_IF -> OUTput -> IF signal
-- bit1  - WR - (1) - CODEC_CONTROL_CLK -> OUTput -> data clock signal
-- bit0  - WR - (0) - CODEC_CONTROL_DATA -> OUTput -> data bit signal
--
-- Notes:
-- ( ) means value after reset.
--

LIBRARY IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;

entity reg_SC_DEVICES is
	port
	(
	-- Signals from main.
		LB_CLK: in std_logic;
		nLB_ENABLE_READ: in std_logic;
		nLB_ENABLE_WRITE: in std_logic;
		nLB_RESET: in std_logic;
		LB_DATA: inout std_logic_vector ( 15 downto 0 );
	
	-- Pins dedicated to register.
		-- Daughter card.
		DC_DATA_BIT: out std_logic;
		DC_DATA_CLK: out std_logic;
		DC_CONF_DONE: in std_logic;
		nDC_STATUS: in std_logic;
		nDC_CONFIG: out std_logic;
		
		-- PLL.
		PLL_INTERFACE_DATA: inout std_logic; --111
		PLL_INTERFACE_CLK: out std_logic -- 110
	);
	
end reg_SC_DEVICES;

architecture opis of reg_SC_DEVICES is
-- Bits order in regData register:
--
-- bit11 - RW - (0) - DC_DATA_BIT -> OUTput -> data bit signal 
-- bit10 - RW - (0) - DC_DATA_CLK -> OUTput -> data clock signal
-- bit9  - RW - (1) - nDC_CONFIG -> OUTput -> signals start of programing
-- bit8  - WR - (0) - PLL_SET_DATA_PIN_TO_HIGH_Z
-- bit7  - WR - (1) - PLL_INTERFACE_DATA -> INput/OUTput -> data bit signal
-- bit6  - WR - (1) - PLL_INTERFACE_CLK -> OUTput -> data clock signal 
-- bit5  - WR - (0) - CODEC_POWER_UP -> OUTput -> power up/down both CODECs
-- bit4  - WR - (0) - CODEC_CS1 -> OUTput -> CODEC1 chip select
-- bit3  - WR - (0) - CODEC_CS2 -> OUTput -> CODEC2 chip select
-- bit2  - WR - (0) - CODEC_CONTROL_IF -> OUTput -> IF signal
-- bit1  - WR - (1) - CODEC_CONTROL_CLK -> OUTput -> data clock signal
-- bit0  - WR - (0) - CODEC_CONTROL_DATA -> OUTput -> data bit signal
--
-- Notice:
-- Because we can not set the register to 1 we will negate it's
-- inputs and outputs -> initialize to 0 -> output is 1.
--
	signal regData: std_logic_vector ( 11 downto 0 );
	
begin
-- Bits order in SC_DC_AND_PLL register:
--
-- bit13 - RW - (0) - DC_DATA_BIT -> OUTput -> data bit signal 
-- bit12 - RW - (0) - DC_DATA_CLK -> OUTput -> data clock signal
-- bit11 - R  - (x) - DC_CONFIG_DONE -> INput -> reports configuration status
-- bit10 - R  - (x) - nDC_STATUS -> INput -> reports error in programing
-- bit9  - RW - (1) - nDC_CONFIG -> OUTput -> signals start of programing
-- bit8  - WR - (0) - PLL_SET_DATA_PIN_TO_HIGH_Z
-- bit7  - WR - (1) - PLL_INTERFACE_DATA -> INput/OUTput -> data bit signal
-- bit6  - WR - (1) - PLL_INTERFACE_CLK -> OUTput -> data clock signal 
-- bit5  - WR - (0) - CODEC_POWER_UP -> OUTput -> power up/down both CODECs
-- bit4  - WR - (0) - CODEC_CS1 -> OUTput -> CODEC1 chip select
-- bit3  - WR - (0) - CODEC_CS2 -> OUTput -> CODEC2 chip select
-- bit2  - WR - (0) - CODEC_CONTROL_IF -> OUTput -> IF signal
-- bit1  - WR - (1) - CODEC_CONTROL_CLK -> OUTput -> data clock signal
-- bit0  - WR - (0) - CODEC_CONTROL_DATA -> OUTput -> data bit signal
	read: process ( nLB_ENABLE_READ, regData )
	begin
		if ( nLB_ENABLE_READ = '0' ) then
			LB_DATA( 15 downto 14 ) <= "00";
			LB_DATA( 7 downto 6 ) <= regData( 5 downto 4 );
			LB_DATA( 5 downto 4 ) <= DC_CONF_DONE & nDC_STATUS;
			LB_DATA( 3 ) <= not( regData( 3 );
			LB_DATA( 2 ) <= regData( 2 );
			LB_DATA( 1 downto 0 ) <= not ( regData( 1 downto 0 ) );
		else
			LB_DATA <= ( LB_DATA'range => 'Z' );
		end if;
	end process;
	
	write: process ( LB_CLK, nLB_ENABLE_WRITE, nLB_RESET )
	begin
		if rising_edge( LB_CLK ) then
			if ( nLB_RESET = '0' ) then
				regData <= ( regData'range => '0' );
			elsif ( nLB_ENABLE_WRITE = '0' ) then
				regData( 5 downto 4 ) <= LB_DATA( 7 downto 6 );
				regData( 3 ) <= not ( LB_DATA( 3 ) );
				regData( 2 ) <= LB_DATA( 2 );
				regData( 1 downto 0 ) <= not ( LB_DATA( 1 downto 0 ) );
			end if;
		end if;
	end process;
	
-- Connect output pins.
	-- Daughter card.
	DC_DATA_BIT <= regData( 5 );
	DC_DATA_CLK <= regData( 4 );
	nDC_CONFIG <= not regData( 3 );
	
	-- Pll.
	PLL_INTERFACE_DATA <= not( regData( 1 ) ) when ( regData( 2 ) = '0' ) else 'Z';
	PLL_INTERFACE_CLK <= not regData( 0 );

	-- CODECs.	
	CODEC_CS1 <= regData( 4 );
	CODEC_CS2 <= regData( 3 );
	CODEC_CONTROL_IF <= regData( 2 );
	CODEC_CONTROL_CLK <= not regData( 1 );
	CODEC_CONTROL_DATA <= regData( 0 );
	CODEC_MASTER_SLAVE <= '0';
	
end opis;