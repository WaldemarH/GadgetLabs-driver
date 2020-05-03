--***************************************************************************
--***************************************************************************
--
--                          PROPRIETARY INFORMATION
--
-- This software is supplied under the terms of a license agreement or
-- nondisclosure agreement with Waldemar Haszlakiewicz and may not be
-- copied or disclosed except in accordance with the terms of that agreement.
--
--               Copyright (C) 2002 - 2005 Waldemar Haszlakiewicz
--                            All Rights Reserved.
--
--***************************************************************************
--***************************************************************************

-- The register contains connections for daughter card programing.
--
-- bit15 - R  - (0)
-- bit14 - R  - (1) - Daughter card present(LOW - present, HIGH - not present).
-- bit13 - RW - (0) - DC_DATA_BIT -> OUTput -> data bit signal 
-- bit12 - RW - (0) - DC_DATA_CLK -> OUTput -> data clock signal
-- bit11 - R  - (x) - DC_CONFIG_DONE -> INput -> reports configuration status
-- bit10 - R  - (x) - nDC_STATUS -> INput -> reports error in programing
-- bit9  - RW - (1) - nDC_CONFIG -> OUTput -> signals start of programing
-- bit8  - R  - (0)
-- bit7  - R  - (0)
-- bit6  - R  - (0)
-- bit5  - R  - (0)
-- bit4  - R  - (0)
-- bit3  - R  - (0)
-- bit2  - R  - (0)
-- bit1  - R  - (0)
-- bit0  - R  - (0)
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
		DC_nPRSNT: in std_logic;
		DC_CONF_DONE: in std_logic; -- 173
		DC_DATA_BIT: out std_logic; -- 175
		DC_DATA_CLK: out std_logic; -- 174
		DC_nCONFIG: out std_logic; -- 171
		DC_nSTATUS: in std_logic -- 172 
	);
	
end reg_SC_DEVICES;

architecture opis of reg_SC_DEVICES is
-- Bits order in regData register:
--
-- bit2  - RW - (0) - DC_DATA_BIT -> OUTput -> data bit signal 
-- bit1  - RW - (0) - DC_DATA_CLK -> OUTput -> data clock signal
-- bit0  - RW - (1) - nDC_CONFIG -> OUTput -> signals start of programing
--
-- Notice:
-- Because we can not set the register to 1 we will negate it's
-- inputs and outputs -> initialize to 0 -> output is 1.
--
	signal regData: std_logic_vector ( 2 downto 0 );
	
begin
	read: process
	(
		nLB_ENABLE_READ,
		regData,
		DC_nPRSNT,
		DC_CONF_DONE,
		DC_nSTATUS
	)
	begin
		if ( nLB_ENABLE_READ = '0' ) then
		-- void
			LB_DATA( 15 ) <= '0';
		-- Daughter card.
			LB_DATA( 14 ) <= DC_nPRSNT;
			LB_DATA( 13 downto 12 ) <= regData( 2 downto 1 );
			LB_DATA( 11 downto 10 ) <= DC_CONF_DONE & DC_nSTATUS;
			LB_DATA( 9 ) <= not regData( 0 );
		-- void
			LB_DATA( 8 downto 0 ) <= "000000000";
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
			-- Daughter card.
				regData( 2 downto 1 ) <= LB_DATA( 13 downto 12 );
				regData( 0 ) <= not LB_DATA( 9 );
			end if;
		end if;
	end process;
	
-- Connect output pins.
	-- Daughter card.
	DC_DATA_BIT <= regData( 2 );
	DC_DATA_CLK <= regData( 1 );
	DC_nCONFIG <= not regData( 0 );

end opis;