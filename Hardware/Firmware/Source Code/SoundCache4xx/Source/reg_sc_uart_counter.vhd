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

-- UART clock counter.
--
-- This counter counts UART main clock (4MHz).
--
-- The counter will wrap aprox. every 16 ms.
-- bit15 - R  - (x)
-- bit14 - R  - (x)
-- bit13 - R  - (x)
-- bit12 - R  - (x)
-- bit11 - R  - (x)
-- bit10 - R  - (x)
-- bit9  - R  - (x)
-- bit8  - R  - (x)
-- bit7  - R  - (x)
-- bit6  - R  - (x)
-- bit5  - R  - (x)
-- bit4  - R  - (x)
-- bit3  - R  - (x)
-- bit2  - R  - (x)
-- bit1  - R  - (x)
-- bit0  - R  - (x)
--
-- Notes:
-- ( ) means value after reset.
--
LIBRARY IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;

entity reg_SC_UART_COUNTER is
	port
	(
	-- Signals from main.
		LB_CLK:	in std_logic;
		nLB_ENABLE_READ: in std_logic;
		LB_DATA: out std_logic_vector ( 15 downto 0 );
		
	-- Pins dedicated to register.
		CLOCK_UART: in std_logic -- 41
	);
end reg_SC_UART_COUNTER;

architecture opis of reg_SC_UART_COUNTER is
	signal UC_counter: std_logic_vector ( 15 downto 0 );
	signal UC_counter_protector: std_logic_vector ( 15 downto 0 );
	signal UC_counter_status: std_logic_vector ( 15 downto 0 );
	
	signal UC_gate: std_logic_vector( 15 downto 0 );
	
begin
	readValue: process ( nLB_ENABLE_READ, UC_counter_status )
	begin
		if ( nLB_ENABLE_READ = '0' ) then
			UC_gate <= UC_counter_status;
		else
			UC_gate <= ( UC_gate'range => 'Z' );
		end if;
	end process;
	LB_DATA <= UC_gate;

-- counter
	process ( CLOCK_UART, UC_counter )
	begin
		if rising_edge( CLOCK_UART ) then
			UC_counter <= UC_counter + "0000000000000001";
		end if;
	end process;

	-- glich protect
	process ( LB_CLK, UC_counter )
	begin
		if rising_edge( LB_CLK ) then
			UC_counter_protector <= UC_counter;
		end if;
	end process;
	process ( LB_CLK, UC_counter_protector )
	begin
		if rising_edge( LB_CLK ) then
			UC_counter_status <= UC_counter_protector;
		end if;
	end process;

end opis;