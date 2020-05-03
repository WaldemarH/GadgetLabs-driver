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

-- Interrupt flags.
--
-- bit15 -  R - (0)
-- bit14 -  R - (0)
-- bit13 -  R - (0)
-- bit12 -  R - (0)
-- bit11 -  R - (0)
-- bit10 -  R - (0)
-- bit9  -  R - (0)
-- bit8  -  R - (0)
-- bit7  -  R - (0)
-- bit6  -  R - (0)
-- bit5  -  R - (0) - MIDI interrupt flag
-- bit4  -  R - (0) - daughter card card error flag
-- bit3  -  R - (0) - audio engine interrupt flag (the current buffer is full (or it was read))
-- bit2  - WR - (0) - enable MIDI interrupt
-- bit1  - WR - (0) - enable daughter card error interrupt 
-- bit0  - WR - (0) - enable audio engine interrupt
--
-- Notes:
-- ( ) means value after reset.
--
-- If enable bit isn't set even if the source will signal an interrupt the
-- flags wont go high.
--
-- To test if the interrupt came from the card the user should read the
-- bit2 of the PLX 'Interrupt Control/Status Register'.
--
-- Notice:
-- Reading the register will reset interrupt flags.
-- (Writing to it wont reset the flags.)
--
LIBRARY IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;

entity reg_SC_INTERRUPT is
	port
	(
	-- Signals from main.
		LB_CLK: in std_logic;
		nLB_ENABLE_READ: in std_logic;
		nLB_ENABLE_WRITE: in std_logic;
		nLB_RESET: in std_logic;
		LB_DATA: inout std_logic_vector ( 15 downto 0 );
	
	-- Pins dedicated to register.
		INT_MIDI: in std_logic; --58
		nINT_DAUGHTER_CARD: in std_logic; --57
		INT_ENGINE: in std_logic;
		nINT_OUTPUT_REQUEST: out std_logic -- 43 outputDrain pin
	);
	
end reg_SC_INTERRUPT;

architecture opis of reg_SC_INTERRUPT is
	-- regData contains enable flags (it's running on localBus clock)
	-- glichProtector contains regData and with input interrupt flags
	-- outputReg is in pair with glichProtector (it's just a buffer for glichProtector)
	signal regData: std_logic_vector ( 2 downto 0 );
	signal interrupts: std_logic_vector ( 2 downto 0 );
	signal interrupts_status: std_logic_vector ( 2 downto 0 );
	
	signal ORedInterrupts: std_logic;
	
begin
-- Register read/write.
	read: process ( nLB_ENABLE_READ, interrupts_status, regData )
	begin
		if ( nLB_ENABLE_READ = '0' ) then
			LB_DATA <= "0000000000" & interrupts_status & regData;
		else
			LB_DATA <= ( LB_DATA'range => 'Z' );
		end if;
	end process;

	write: process ( LB_CLK, nLB_ENABLE_WRITE, nLB_RESET )
	begin
		if rising_edge( LB_CLK ) then
			if ( nLB_RESET = '0' ) then
			-- clear enable flags
				regData( 2 downto 0 ) <= b"000";
			elsif ( nLB_ENABLE_WRITE = '0' ) then
				regData( 2 downto 0 ) <= LB_DATA( 2 downto 0 );
			end if;
		end if;
	end process;
	
-- Protect agains gliches
	process ( LB_CLK )
	begin
		if rising_edge( LB_CLK ) then
			interrupts( 2 ) <= regData( 2 ) and INT_MIDI; -- both have to be HIGH
			interrupts( 1 ) <= regData( 1 ) and ( not nINT_DAUGHTER_CARD );
			interrupts( 0 ) <= regData( 0 ) and INT_ENGINE;
		end if;
	end process;
	process ( LB_CLK )
	begin
		if rising_edge( LB_CLK ) then
			interrupts_status <= interrupts;
		end if;
	end process;

-- Connect output pins.
	-- If any of the flags will go LOW the interrupt will be requested.
	-- Be careful this pin is open drain pin.
	ORedInterrupts <= interrupts( 2 ) or interrupts( 1 ) or interrupts( 0 );
	nINT_OUTPUT_REQUEST <= '0' when ORedInterrupts = '1' else 'Z';
	
end opis;

