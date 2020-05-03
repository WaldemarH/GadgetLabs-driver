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

-- Test 16bit register.
--
LIBRARY IEEE;
use IEEE.std_logic_1164.all;

entity reg_testing is
	port
	(
		clk: in std_logic;
		nEnable_read: in std_logic;
		nEnable_write: in std_logic;
		nReset: in std_logic;
		data: inout std_logic_vector ( 15 downto 0 )
	);
	
end reg_testing;

architecture opis of reg_testing is

	signal regData: std_logic_vector ( 15 downto 0 );

begin
-- Register read/write.
	read: process ( nEnable_read, regData )
	begin
		if ( nEnable_read = '0' ) then
			data <= regData;
		else
			data <= ( data'range => 'Z' );
		end if;
	end process;
	
	write: process ( clk, nReset, nEnable_write )
	begin
		if rising_edge( clk ) then
			if ( nReset = '0' ) then
				regData <= ( regData'range => '0' );
			elsif ( nEnable_write = '0' ) then
				regData <= data;
			end if;
		end if;
	end process;
	
end opis;
