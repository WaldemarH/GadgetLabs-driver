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

--Register contains audio engine parameters (sample rate, reset flags).
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
-- bit6  - WR - (0) - Reset clock detectors( it will reset bit 2 and 3 ).
--                    Usage:
--                    The bit flags should be set to 1, wait for 1us and set back to 0.
--                    Wait another 10us and test the clock present bits.
--
-- bit5  - WR - (0) - Release counters from reset state.
--
--                    This flag will allow counter to start running.
--                    Or if we look it at another way this is a
--                    negative going reset state
--                    ( 0 -> reset/freeze, 1 - counters can run ).
--
--                    Notice:
--                    This flag resets the audio engine interrupt flag also.
--
-- bit4  - WR - (0) - Start/Stop audio engine(clocks).
--
--                    This flag disables MasterClockOutput so that any
--                    gliches from PLL programming wont cause any problems.
--                    When set to 1 Altera circuit will start the clock glich free.
--
--                    This flag also disconnects PLL clocks from audio engine.
--                    (if set to 0 -> disconnect, 1 - connect.)
--
--                    Extra notes:
--                    The flag (Start/Stop meaning) has connection to
--                    daughter board so the daughter board is Started/Stopped
--                    synchronized to main board.
--                    But you have to reset slave cards.
--
-- bit3  -  R - (x) - syncIn clock present
-- bit2  -  R - (x) - daughter card clock present
-- bit1  - WR - (0) - \
-- bit0  - WR - (0) - /-> sample rate selector
--
-- Notes:
-- ( ) means value after reset.
--
-- Sample rate selector (code has automatic mux selector/deselector) for 4xx cards:
-- value (bit2,bit1,bit0)   sampleRate
-- 00                       PLL A(default setting)
-- 01                       PLL A(default setting)
-- 10                       daughter card clock (digital signal clock)
-- 11                       use syncIn clock
--
--
-- To change the sample rate do:
-- 1) Stop master card -> bit4 to 0.
--
--    This will cause the masterCard clock to stop glichFree (the clock
--    will stop in LOW position).
--    And because the master card clock will be stopped all the
--    slave cards will stop also.
--
--    Notice:
--    Wait 1us.
--
-- 2) Set new sampleRate.
--
-- 3) Reset all cards counters -> bit5 to 0 on all cards.
--    (To reset counters you need to clear the flag, wait a 1us and set it
--     back to 1. )
--
-- 4) Set the interrupt downcounter value to bufferSize+1.
--    Delay 1us.
--
-- 5) Start master card -> bit4 to 1.
--    This will start masterCard and slave cards glichFree.
--
-- NOTICE:
-- This start->stop->start period MUST be as short as it can be, so user
-- should rise priority as much as it can.
-- (CODECs don't like to be stopped for a long times.)
--
LIBRARY IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;

entity reg_SC_ENGINE is
	port
	(
	-- Signals from main.
		LB_CLK: in std_logic;
		nLB_ENABLE_READ: in std_logic;
		nLB_ENABLE_WRITE: in std_logic;
		nLB_RESET: in std_logic;
		LB_DATA: inout std_logic_vector ( 15 downto 0 );

	-- Pins dedicated to engine.
		CLOCK_SELECTION: out std_logic_vector( 1 downto 0 );
		
		nRESET_ENGINES: out std_logic;	-- connect also to pin81
		nENGINES_STOP_REQUEST: out std_logic; -- connect also to pin80
		
	-- Clocks for detection.
		CLOCK_DAUGHTER_CARD: in std_logic; -- 68
		CLOCK_SYNC_IN: in std_logic -- 70
	);
	
end reg_SC_ENGINE;

architecture opis of reg_SC_ENGINE is
	component clockTester is
		port
		(
			CLOCK: in std_logic;
			RESET_TESTER: in std_logic;
			
			CLK_PRESENT: out std_logic
		);
	end component;

-------------------------------------------------------------------------------
-- REGISTER -------------------------------------------------------------------
-------------------------------------------------------------------------------
-- bit4  - WR - (0) - Reset clockDetectors
-- bit3  - WR - (0) - Release counters from reset state.
-- bit2  - WR - (0) - Start/Stop audio engine(clocks).
-- bit1  - WR - (0) - \
-- bit0  - WR - (0) - /-> sample rate selector
	signal regData: std_logic_vector( 4 downto 0 );
	
	signal daughterCardClockPresent: std_logic;
	signal syncInClockPresent: std_logic;

begin
	CT1: clockTester port map
	(
		CLOCK_DAUGHTER_CARD,
		regData( 4 ),
		daughterCardClockPresent
	);
	CT2: clockTester port map
	(
		CLOCK_SYNC_IN,
		regData( 4 ),
		syncInClockPresent
	);

-------------------------------------------------------------------------------
-- REGISTER -------------------------------------------------------------------
-------------------------------------------------------------------------------
	read: process ( nLB_ENABLE_READ, regData, syncInClockPresent, daughterCardClockPresent )
	begin
		if ( nLB_ENABLE_READ = '0' ) then
			LB_DATA( 15 downto 7 ) <= "000000000";
			LB_DATA( 6 downto 4 ) <= regData( 4 downto 2 );
			LB_DATA( 3 ) <= syncInClockPresent;
			LB_DATA( 2 ) <= daughterCardClockPresent;
			LB_DATA( 1 downto 0 ) <= regData( 1 downto 0 );
		else
			LB_DATA <= ( LB_DATA'range => 'Z' );
		end if;
	end process;
	write: process
	(
		LB_CLK,
		nLB_ENABLE_WRITE,
		nLB_RESET
	)
	begin
		if rising_edge( LB_CLK ) then
			if ( nLB_RESET = '0' ) then
				regData <= ( regData'range => '0' );
			elsif ( nLB_ENABLE_WRITE = '0' ) then
				regData( 4 downto 2 ) <= LB_DATA( 6 downto 4 );
				regData( 1 downto 0 ) <= LB_DATA( 1 downto 0 );
			end if;
		end if;
	end process;

-- Connect output pins.
	CLOCK_SELECTION <= regData( 1 downto 0 );
	nENGINES_STOP_REQUEST <= regData( 2 );
	nRESET_ENGINES <= regData( 3 );

end opis;

-- Clock tester detects if there is a clock present.
--
-- Now this is a simple counter. It counts the clocks of the input signal.
-- If it detects more then 16 clocks then it will stop counting and signal that clock is present.
--
LIBRARY IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;

entity clockTester is
	port
	(
		CLOCK: in std_logic;
		RESET_TESTER: in std_logic;
			
		CLK_PRESENT: out std_logic
	);
end clockTester;

architecture opis of clockTester is
	signal counter: std_logic_vector( 4 downto 0 );
	
begin
	process( CLOCK, RESET_TESTER, counter )
	begin
		if ( RESET_TESTER = '1' ) then
			counter <= "00000";
		elsif rising_edge ( CLOCK ) then
			if ( counter = "10000" ) then
				counter <= "10000";
			else
				counter <= counter + "00001";
			end if;
		end if;
	end process;
	CLK_PRESENT <= counter( 4 );
	
end opis;