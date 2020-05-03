--***************************************************************************
--***************************************************************************
--
--                          PROPRIETARY INFORMATION
--
-- This software is supplied under the terms of a license agreement or
-- nondisclosure agreement with Waldemar Haszlakiewicz and may not be
-- copied or disclosed except in accordance with the terms of that agreement.
--
--               Copyright (C) 2004 Waldemar Haszlakiewicz
--                            All Rights Reserved.
--
--***************************************************************************
--***************************************************************************

-- Next code will create clocks that will be used in the Altera.
-- Clocks that are used on the card:
--
-- 1. LB_CLK (local bus clock -> used for register access -> PCI clock ->
--            connected in main.vhd to global signal pin 20)
--
-- 2. MASTER_CLOCK (clock used for CODECs engine -> 384*fs ->
--                  the code will select the master clock and output it
--                  to MASTER_CLOCK_OUT pin -> the CODECs are connected externally
--                  to it/it will be used for BICK clock generator also)
--
-- 3. BICK clock (clock is used for CODECs data transfer and for
--                transfer engine sinhronization (well bick_24) -> 96*fs ->
--                clock is internally connected to global clock)
--                Notice:
--                Falling edge clock.
--
-- 4. SAMPLE_RATE clock (clock is used for interrupt downcounter,
--                       memory address offset and for sampleRate clock used
--                       in driver -> 1*fs -> clock is internally connected to
--                       global clock)
--                       Notice:
--                       Faling edge clock.
--
LIBRARY IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;

entity clocks_engine is
	port
	(
	--Pins dedicated to engine.
		CLOCK_SELECTION: in std_logic_vector( 1 downto 0 );
		nSTOP_MASTER_CLOCK: in std_logic;

		-- input clocks
		CLOCK_DAUGHTER_CARD: in std_logic; -- 68
		CLOCK_PLL_A: in std_logic; -- 71
		CLOCK_SYNC_IN: in std_logic; -- 70
		
		-- output clocks
		CLOCK_BICK_OUT: out std_logic;
		nBICK_24_OUT: out std_logic;
		CLOCK_MASTER_OUT: out std_logic; -- 67 (384 x fs)
		CLOCK_SAMPLE_RATE_OUT: out std_logic
	);
end clocks_engine;

architecture opis of clocks_engine is
-------------------------------------------------------------------------------
-- MASTER CLOCK ENGINE --------------------------------------------------------
-------------------------------------------------------------------------------
	signal SRE_masterClock: std_logic;
	signal SRE_masterClockGate: std_logic;
	signal SRE_masterClockGate_protector: std_logic;
	signal SRE_selectedClock: std_logic;

-------------------------------------------------------------------------------
-- CLOCK GENERATORS -----------------------------------------------------------
-------------------------------------------------------------------------------
	signal CG_generator_bick: std_logic_vector ( 1 downto 0 );
	signal CG_generator_sampleRate: std_logic_vector ( 6 downto 0 ); 
	
	signal CG_bick: std_logic;

begin
-------------------------------------------------------------------------------
-- MASTER CLOCK ENGINE --------------------------------------------------------
-------------------------------------------------------------------------------
-- Select master clock.
	process
	(
		CLOCK_SELECTION,
		CLOCK_PLL_A,
		CLOCK_DAUGHTER_CARD,
		CLOCK_SYNC_IN
	)
	begin
		case CLOCK_SELECTION is
			when b"00" => SRE_selectedClock <= CLOCK_PLL_A; -- pll clock
			when b"01" => SRE_selectedClock <= CLOCK_PLL_A; -- pll clock
			when b"10" => SRE_selectedClock <= CLOCK_DAUGHTER_CARD; -- digitalClock
			when others => SRE_selectedClock <= CLOCK_SYNC_IN; -- syncIn
		end case;
	end process;

-- Stop/Start master clock ( glich free ).
--
-- Take notice that at change the CODECs will play just zero samples (no sound),
-- so there shouldn't be any strange sound comming out
-- (because strange clock behaviour).
--
	process ( SRE_selectedClock, nSTOP_MASTER_CLOCK )
	begin
		if falling_edge( SRE_selectedClock ) then
			SRE_masterClockGate_protector <= nSTOP_MASTER_CLOCK;
		end if;
	end process;
	process ( SRE_selectedClock, SRE_masterClockGate_protector )
	begin
		if falling_edge( SRE_selectedClock ) then
		-- Gate: 0 -> no output clock
		--       1 -> clock enabled
			SRE_masterClockGate <= SRE_masterClockGate_protector;
		end if;
	end process;
	SRE_masterClock <= SRE_masterClockGate and SRE_selectedClock;

	-- Connect output pins.
	CLOCK_MASTER_OUT <= SRE_masterClock;

-------------------------------------------------------------------------------
-- CLOCK GENERATORS -----------------------------------------------------------
-------------------------------------------------------------------------------
-- Notice:
-- If nSTOP_MASTER_CLOCK goes LOW clocks must be reset, so that when started
-- they will start synchronized.
--
--
-- Create BICK clock and use it as a global clock
-- (that's because the whole transfer engine is synchronized to BICK clock).
--
-- BICK(96fs) = MASTER_CLOCK(384fs)/4
--
	process ( SRE_masterClock, SRE_masterClockGate )
	begin
		if ( SRE_masterClockGate = '0' ) then
		-- Reset counter.
			CG_generator_bick <= ( CG_generator_bick'range => '0' );
		elsif falling_edge( SRE_masterClock ) then
			CG_generator_bick <= CG_generator_bick + b"01";
		end if;
	end process;
	CG_bick <= CG_generator_bick( 1 );
	CLOCK_BICK_OUT <= CG_bick;

-- Create SAMPLE_RATE clock and use it as a global clock.
--
-- SAMPLE_RATE = BICK/96 = MASTER_CLOCK/384
--
-- Notice:
-- The counter will be organized in a way that we'll be able to get the bick_24 out of it also.
--
	process ( CG_bick, SRE_masterClockGate )
	begin
		if ( SRE_masterClockGate = '0' ) then
		-- Reset counter.
			CG_generator_sampleRate <= ( CG_generator_sampleRate'range => '0' );
		elsif falling_edge( CG_bick ) then
		-- Because bick_24 needs simetric clock we'll divide 96fs to 2 parts ->
		-- first will count 48 clocks and the second 48 clock -> and in
		-- between we'll jump up and down.
			if ( CG_generator_sampleRate = b"0101111" ) then -- decimal 47 (from 0 to 47 are 48 steps)
				CG_generator_sampleRate <= b"1000000";
			elsif ( CG_generator_sampleRate = b"1101111" ) then	-- decimal 111 (from 64 to 111 are 48 steps)
				CG_generator_sampleRate <= b"0000000";
			else
				CG_generator_sampleRate <= CG_generator_sampleRate + b"0000001";
			end if;
		end if;
	end process;
	
	--Convert negative edge to rising edge clock -> 'not' in the signal.
	CLOCK_SAMPLE_RATE_OUT <= CG_generator_sampleRate( 6 );

-- Create bick_24 signal.
--
-- In every SAMPLE_RATE clock we need to transfer 2 samples to/from CODECs.
-- Because SAMPLE_RATE = BICK/96 we have 96 bick clocks to do this.
-- But because we need to transfer only 24 bits per sample we'll create a signal
-- that will show us which bits to transfer and which not.
-- 
	nBICK_24_OUT <= '0' when ( CG_generator_sampleRate( 5 downto 0 ) < b"011000" ) else '1';

end opis;