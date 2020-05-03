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

-- Gain register for 824 cards.
--
-- bit15 - RW - (0) - Output channel 8 -> 0 = -10dBv, 1 = +4dBu
-- bit14 - RW - (0) - Output channel 7 -> 0 = -10dBv, 1 = +4dBu
-- bit13 - RW - (0) - Output channel 6 -> 0 = -10dBv, 1 = +4dBu
-- bit12 - RW - (0) - Output channel 5 -> 0 = -10dBv, 1 = +4dBu
-- bit11 - RW - (0) - Output channel 4 -> 0 = -10dBv, 1 = +4dBu
-- bit10 - RW - (0) - Output channel 3 -> 0 = -10dBv, 1 = +4dBu
-- bit9  - RW - (0) - Output channel 2 -> 0 = -10dBv, 1 = +4dBu
-- bit8  - RW - (0) - Output channel 1 -> 0 = -10dBv, 1 = +4dBu
-- bit7  - RW - (0) - Input channel 8 -> 0 = -10dBv, 1 = +4dBu
-- bit6  - RW - (0) - Input channel 7 -> 0 = -10dBv, 1 = +4dBu
-- bit5  - RW - (0) - Input channel 6 -> 0 = -10dBv, 1 = +4dBu
-- bit4  - RW - (0) - Input channel 5 -> 0 = -10dBv, 1 = +4dBu
-- bit3  - RW - (0) - Input channel 4 -> 0 = -10dBv, 1 = +4dBu
-- bit2  - RW - (0) - Input channel 3 -> 0 = -10dBv, 1 = +4dBu
-- bit1  - RW - (0) - Input channel 2 -> 0 = -10dBv, 1 = +4dBu
-- bit0  - RW - (0) - Input channel 1 -> 0 = -10dBv, 1 = +4dBu
--
LIBRARY IEEE;
use IEEE.std_logic_1164.all;

entity reg_SC8_GAIN is
	port
	(
	-- Signals from main.
		LB_CLK: in std_logic;
		nLB_ENABLE_READ: in std_logic;
		nLB_ENABLE_WRITE: in std_logic;
		nLB_RESET: in std_logic;
		LB_DATA: inout std_logic_vector ( 15 downto 0 );
	
	-- Pins dedicated to register.
		GAIN_CH8_IN: out std_logic;	-- 83
		GAIN_CH7_IN: out std_logic;	-- 84
		GAIN_CH6_IN: out std_logic;	-- 85
		GAIN_CH5_IN: out std_logic;	-- 86
		GAIN_CH4_IN: out std_logic;	-- 87
		GAIN_CH3_IN: out std_logic;	-- 88
		GAIN_CH2_IN: out std_logic;	-- 89
		GAIN_CH1_IN: out std_logic;	-- 90

		GAIN_CH8_OUT: out std_logic; -- 91		
		GAIN_CH7_OUT: out std_logic; -- 92		
		GAIN_CH6_OUT: out std_logic; -- 93		
		GAIN_CH5_OUT: out std_logic; -- 94		
		GAIN_CH4_OUT: out std_logic; -- 97		
		GAIN_CH3_OUT: out std_logic; -- 98		
		GAIN_CH2_OUT: out std_logic; -- 99		
		GAIN_CH1_OUT: out std_logic -- 100		
	);
	
end reg_SC8_GAIN;

architecture opis of reg_SC8_GAIN is
	signal regData: std_logic_vector ( 15 downto 0 );
	
begin
-- Register read/write.
	read: process ( nLB_ENABLE_READ, regData )
	begin
		if ( nLB_ENABLE_READ = '0' ) then
			LB_DATA <= regData;
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
				regData <= LB_DATA;
			end if;
		end if;
	end process;
	
-- Connect output pins.
-- bit15 - RW - (0) - Output channel 8 -> 0 = -10dBv, 1 = +4dBu
-- bit14 - RW - (0) - Output channel 7 -> 0 = -10dBv, 1 = +4dBu
-- bit13 - RW - (0) - Output channel 6 -> 0 = -10dBv, 1 = +4dBu
-- bit12 - RW - (0) - Output channel 5 -> 0 = -10dBv, 1 = +4dBu
-- bit11 - RW - (0) - Output channel 4 -> 0 = -10dBv, 1 = +4dBu
-- bit10 - RW - (0) - Output channel 3 -> 0 = -10dBv, 1 = +4dBu
-- bit9  - RW - (0) - Output channel 2 -> 0 = -10dBv, 1 = +4dBu
-- bit8  - RW - (0) - Output channel 1 -> 0 = -10dBv, 1 = +4dBu

-- bit7  - RW - (0) - Input channel 8 -> 0 = -10dBv, 1 = +4dBu
-- bit6  - RW - (0) - Input channel 7 -> 0 = -10dBv, 1 = +4dBu
-- bit5  - RW - (0) - Input channel 6 -> 0 = -10dBv, 1 = +4dBu
-- bit4  - RW - (0) - Input channel 5 -> 0 = -10dBv, 1 = +4dBu
-- bit3  - RW - (0) - Input channel 4 -> 0 = -10dBv, 1 = +4dBu
-- bit2  - RW - (0) - Input channel 3 -> 0 = -10dBv, 1 = +4dBu
-- bit1  - RW - (0) - Input channel 2 -> 0 = -10dBv, 1 = +4dBu
-- bit0  - RW - (0) - Input channel 1 -> 0 = -10dBv, 1 = +4dBu
	GAIN_CH8_OUT <= not regData( 15 );
	GAIN_CH7_OUT <= not regData( 14 );
	GAIN_CH6_OUT <= not regData( 13 );
	GAIN_CH5_OUT <= not regData( 12 );
	GAIN_CH4_OUT <= not regData( 11 );
	GAIN_CH3_OUT <= not regData( 10 );
	GAIN_CH2_OUT <= not regData( 9 );
	GAIN_CH1_OUT <= not regData( 8 );
		
	GAIN_CH8_IN <= regData( 7 );
	GAIN_CH7_IN <= regData( 6 );
	GAIN_CH6_IN <= regData( 5 );
	GAIN_CH5_IN <= regData( 4 );
	GAIN_CH4_IN <= regData( 3 );
	GAIN_CH3_IN <= regData( 2 );
	GAIN_CH2_IN <= regData( 1 );
	GAIN_CH1_IN <= regData( 0 );

end opis;