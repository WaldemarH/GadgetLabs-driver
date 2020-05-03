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

-- Monitor and mute register for 824 cards.
--
-- bit15 - RW - (0) - Mute channel 8 -> 0 = off, 1 = on
-- bit14 - RW - (0) - Mute channel 7 -> 0 = off, 1 = on
-- bit13 - RW - (0) - Mute channel 6 -> 0 = off, 1 = on
-- bit12 - RW - (0) - Mute channel 5 -> 0 = off, 1 = on
-- bit11 - RW - (0) - Mute channel 4 -> 0 = off, 1 = on
-- bit10 - RW - (0) - Mute channel 3 -> 0 = off, 1 = on
-- bit9  - RW - (0) - Mute channel 2 -> 0 = off, 1 = on
-- bit8  - RW - (0) - Mute channel 1 -> 0 = off, 1 = on
-- bit7  - RW - (0) - Monitor channel 8 -> 0 = disabled, 1 = enabled
-- bit6  - RW - (0) - Monitor channel 7 -> 0 = disabled, 1 = enabled
-- bit5  - RW - (0) - Monitor channel 6 -> 0 = disabled, 1 = enabled
-- bit4  - RW - (0) - Monitor channel 5 -> 0 = disabled, 1 = enabled
-- bit3  - RW - (0) - Monitor channel 4 -> 0 = disabled, 1 = enabled
-- bit2  - RW - (0) - Monitor channel 3 -> 0 = disabled, 1 = enabled
-- bit1  - RW - (0) - Monitor channel 2 -> 0 = disabled, 1 = enabled
-- bit0  - RW - (0) - Monitor channel 1 -> 0 = disabled, 1 = enabled
--
LIBRARY IEEE;
use IEEE.std_logic_1164.all;

entity reg_SC8_MON_MUTE is
	port
	(
	-- Signals from main.
		LB_CLK: in std_logic;
		nLB_ENABLE_READ: in std_logic;
		nLB_ENABLE_WRITE: in std_logic;
		nLB_RESET: in std_logic;
		LB_DATA: inout std_logic_vector ( 15 downto 0 );
	
	-- Pins dedicated to register.
		MUTE_CH8: out std_logic; -- 145
		MUTE_CH7: out std_logic; -- 141
		MUTE_CH6: out std_logic; -- 146
		MUTE_CH5: out std_logic; -- 142
		MUTE_CH4: out std_logic; -- 151
		MUTE_CH3: out std_logic; -- 143
		MUTE_CH2: out std_logic; -- 152
		MUTE_CH1: out std_logic; -- 144

		MON_CH8: out std_logic; -- 161
		MON_CH7: out std_logic; -- 162
		MON_CH6: out std_logic; -- 163
		MON_CH5: out std_logic; -- 164
		MON_CH4: out std_logic; -- 167
		MON_CH3: out std_logic; -- 168
		MON_CH2: out std_logic; -- 169
		MON_CH1: out std_logic -- 170
	);
	
end reg_SC8_MON_MUTE;

architecture opis of reg_SC8_MON_MUTE is
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
-- bit15 - RW - (0) - Mute channel 8 -> 0 = off, 1 = on
-- bit14 - RW - (0) - Mute channel 7 -> 0 = off, 1 = on
-- bit13 - RW - (0) - Mute channel 6 -> 0 = off, 1 = on
-- bit12 - RW - (0) - Mute channel 5 -> 0 = off, 1 = on
-- bit11 - RW - (0) - Mute channel 4 -> 0 = off, 1 = on
-- bit10 - RW - (0) - Mute channel 3 -> 0 = off, 1 = on
-- bit9  - RW - (0) - Mute channel 2 -> 0 = off, 1 = on
-- bit8  - RW - (0) - Mute channel 1 -> 0 = off, 1 = on

-- bit7  - RW - (0) - Monitor channel 8 -> 0 = disabled, 1 = enabled
-- bit6  - RW - (0) - Monitor channel 7 -> 0 = disabled, 1 = enabled
-- bit5  - RW - (0) - Monitor channel 6 -> 0 = disabled, 1 = enabled
-- bit4  - RW - (0) - Monitor channel 5 -> 0 = disabled, 1 = enabled
-- bit3  - RW - (0) - Monitor channel 4 -> 0 = disabled, 1 = enabled
-- bit2  - RW - (0) - Monitor channel 3 -> 0 = disabled, 1 = enabled
-- bit1  - RW - (0) - Monitor channel 2 -> 0 = disabled, 1 = enabled
-- bit0  - RW - (0) - Monitor channel 1 -> 0 = disabled, 1 = enabled
	MUTE_CH8 <= not regData( 15 );
	MUTE_CH7 <= not regData( 14 );
	MUTE_CH6 <= not regData( 13 );
	MUTE_CH5 <= not regData( 12 );
	MUTE_CH4 <= not regData( 11 );
	MUTE_CH3 <= not regData( 10 );
	MUTE_CH2 <= not regData( 9 );
	MUTE_CH1 <= not regData( 8 );

	MON_CH8 <= regData( 7 );
	MON_CH7 <= regData( 6 );
	MON_CH6 <= regData( 5 );
	MON_CH5 <= regData( 4 );
	MON_CH4 <= regData( 3 );
	MON_CH3 <= regData( 2 );
	MON_CH2 <= regData( 1 );
	MON_CH1 <= regData( 0 );

end opis;