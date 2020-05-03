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

-- Gain and monitor register for 4xx cards.
--
-- bit15 - R  - (0)
-- bit14 - R  - (0)
-- bit13 - R  - (0)
-- bit12 - R  - (0)
-- bit11 - R  - (0)
-- bit10 - R  - (0)
-- bit9  - R  - (0)
-- bit8  - R  - (0)
-- bit7  - R  - (0)
-- bit6  - WR - (0) - Input2right monitor -> 0 = disabled, 1 = enabled
-- bit5  - WR - (0) - Input2left monitor -> 0 = disabled, 1 = enabled
-- bit4  - WR - (0) - Input1right monitor -> 0 = disabled, 1 = enabled
-- bit3  - WR - (0) - Input1left monitor -> 0 = disabled, 1 = enabled
-- bit2  - WR - (0) - Input gain_B set -> 0 = -10dBv, 1 = +4dBu
-- bit1  - WR - (0) - Input gain_A set -> 0 = -10dBv, 1 = +4dBu
-- bit0  - WR - (0) - Output gain set -> 0 = -10dBv, 1 = +4dBu
--
-- Notes:
-- ( ) means value after reset.
--
LIBRARY IEEE;
use IEEE.std_logic_1164.all;

entity reg_SC4_GAIN_MON is
	port
	(
	-- Signals from main.
		LB_CLK: in std_logic;
		nLB_ENABLE_READ: in std_logic;
		nLB_ENABLE_WRITE: in std_logic;
		LB_DATA: inout std_logic_vector ( 15 downto 0 );
	
	-- Pins dedicated to register.
		-- Gain, monitoring.	
		GAIN_OUT: out std_logic; -- 59
		GAIN_IN_A: out std_logic; -- 66
		GAIN_IN_B: out std_logic; -- 65
		MONITOR_1L: out std_logic; -- 64
		MONITOR_1R: out std_logic; -- 63
		MONITOR_2L: out std_logic; -- 62
		MONITOR_2R: out std_logic -- 61
	);
	
end reg_SC4_GAIN_MON;

architecture opis of reg_SC4_GAIN_MON is
	signal regData: std_logic_vector ( 6 downto 0 );
	
begin
-- Register read/write.
	read: process ( nLB_ENABLE_READ, regData )
	begin
		if ( nLB_ENABLE_READ = '0' ) then
			LB_DATA <= "000000000" & regData;
		else
			LB_DATA <= ( LB_DATA'range => 'Z' );
		end if;
	end process;
	
	write: process ( LB_CLK, nLB_ENABLE_WRITE )
	begin
		if rising_edge( LB_CLK ) then
			if ( nLB_ENABLE_WRITE = '0' ) then
				regData <= LB_DATA( 6 downto 0 );
			end if;
		end if;
	end process;
	
-- Connect output pins.
-- bit6  - WR - (0) - Input2right monitor -> 0 = disabled, 1 = enabled
-- bit5  - WR - (0) - Input2left monitor -> 0 = disabled, 1 = enabled
-- bit4  - WR - (0) - Input1right monitor -> 0 = disabled, 1 = enabled
-- bit3  - WR - (0) - Input1left monitor -> 0 = disabled, 1 = enabled
-- bit2  - WR - (0) - Input gain_B set -> 0 = -10dBv, 1 = +4dBu
-- bit1  - WR - (0) - Input gain_A set -> 0 = -10dBv, 1 = +4dBu
-- bit0  - WR - (0) - Output gain set -> 0 = -10dBv, 1 = +4dBu
	MONITOR_2R <= regData( 6 );
	MONITOR_2L <= regData( 5 );
	MONITOR_1R <= regData( 4 );
	MONITOR_1L <= regData( 3 );
	GAIN_IN_B <= regData( 2 );
	GAIN_IN_A <= not( regData( 1 ) ); -- for 0 = -10dBv, 1 = +4dBu
	GAIN_OUT <= regData( 0 );
	
end opis;