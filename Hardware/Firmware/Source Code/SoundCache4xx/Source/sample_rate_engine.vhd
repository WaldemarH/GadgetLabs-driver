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

LIBRARY IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;

entity clocks_engine is
	port
	(
	--Pins dedicated to engine.
		-- input/output clocks
		CLOCK_PLL_A: in std_logic; -- 71 (11.2896 MHz for 44.1 kHz sample rate)
		CLOCK_PLL_B: in std_logic; -- 72 (12.288 MHz for 48 kHz sample rate)
		CLOCK_PLL_C: in std_logic; -- 69 (C1: 8.192 MHz for 32 kHz sample rate)
		                           --    (C2: 22.5792 MHz for 88.2 kHz sample rate)
		CLOCK_PLL_D: in std_logic; -- 89 (24.576 MHz for 96 kHz sample rate)
		CLOCK_DAUGHTER_CARD: in std_logic; -- 68
		CLOCK_SYNC_IN: in std_logic; -- 70
		CLOCK_MASTER_OUT: out std_logic; -- 67 (256 x fs)

		-- other
		CLOCK_SELECTION: in std_logic_vector( 2 downto 0 );
		nSTOP_MASTER_CLOCK: in std_logic;
		
		
		-- clock generator
		CLOCK_BICK_OUT: out std_logic;
		BICK_24_3HALFS_CLOCK_DELAY: out std_logic;
		BICK_24_FULL_CLOCK_DELAY: out std_logic;
	
		CLOCK_SAMPLE_RATE_OUT: out std_logic
	);
end clocks_engine;

architecture opis of clocks_engine is
-------------------------------------------------------------------------------
-- SAMPLE RATE ENGINE ---------------------------------------------------------
-------------------------------------------------------------------------------
	signal SRE_masterClock: std_logic;
	signal SRE_masterClockGate: std_logic;
	signal SRE_masterClockGate_protector: std_logic;
	signal SRE_selectedClock: std_logic;

-------------------------------------------------------------------------------
-- CLOCKS GENERATOR -----------------------------------------------------------
-------------------------------------------------------------------------------
	signal CG_generator: std_logic_vector ( 7 downto 0 ); 
	
	signal CG_bick: std_logic;
	signal CG_bick_24: std_logic;
	signal CG_bick_24_half: std_logic;
	signal CG_bick_24_full: std_logic;
	signal CG_bick_24_3halfs: std_logic;
	signal CG_sampleRateClock: std_logic; -- true fs clock

begin
-------------------------------------------------------------------------------
-- SAMPLE RATE ENGINE ---------------------------------------------------------
-------------------------------------------------------------------------------
-- Stop/Start master clock ( glich free )
-- Take notice that at change the CODECs will play just
-- zero samples (no sound), so there shouldn't be any strange sound
-- comming out because strange clock behaviour.
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
	
	-- master clock mux
	process
	(
		CLOCK_SELECTION,
		CLOCK_PLL_A,
		CLOCK_PLL_B,
		CLOCK_PLL_C,
		CLOCK_PLL_D,
		CLOCK_DAUGHTER_CARD,
		CLOCK_SYNC_IN
	)
	begin
		case CLOCK_SELECTION is
			when b"000" => SRE_selectedClock <= CLOCK_PLL_A; -- 44.1kHz
			when b"001" => SRE_selectedClock <= CLOCK_PLL_A; -- 44.1kHz
			when b"010" => SRE_selectedClock <= CLOCK_PLL_C; -- 32kHz
			when b"011" => SRE_selectedClock <= CLOCK_PLL_C; -- 88.2kHz
			when b"100" => SRE_selectedClock <= CLOCK_PLL_B; -- 48kHz
			when b"101" => SRE_selectedClock <= CLOCK_PLL_D; -- 96kHz
			when b"110" => SRE_selectedClock <= CLOCK_DAUGHTER_CARD; -- digitalClock
			when others => SRE_selectedClock <= CLOCK_SYNC_IN; -- syncIn
		end case;
	end process;
	-- POPRAVI -> SEL_CD je bil prestavljen v reg0
	PLL_SELECT_CD <= '1' when ( CLOCK_SELECTION = b"011" ) else '0';

	SRE_masterClock <= SRE_masterClockGate and SRE_selectedClock;

-- Connect output pins.
	CLOCK_MASTER_OUT <= SRE_masterClock;

-------------------------------------------------------------------------------
-- CLOCKS GENERATOR -----------------------------------------------------------
-------------------------------------------------------------------------------
-- Code could use external masterClock_in, but I'll reserver that pin for
-- other needs.
--
-- The registers are initialized to 0 by default.
--
-- For transfer engine we need to count 24 bick clocks so that we'll know when
-- to and when not to fill the transfer registers.
	process ( SRE_masterClock, SRE_masterClockGate )
	begin
		-- As soon as sampleRate engine will stop the generator
		-- will get reset -> so everything will again start synhronized.
		if ( SRE_masterClockGate = '0' ) then
			CG_generator <= ( CG_generator'range => '0' );
		elsif rising_edge( SRE_masterClock ) then
			CG_generator <= CG_generator + b"00000001";
		end if;
	end process;
	CG_bick <= CG_generator( 1 );
	CG_bick_24 <= '0' when ( CG_generator( 6 downto 2 )< b"11000" ) else '1'; -- decimal 24
	
	-- Delay bick_24 for half clock cycle.
	process ( CG_bick )
	begin
		if rising_edge( CG_bick ) then
			CG_bick_24_half <= CG_bick_24;
		end if;
	end process;
	-- Delay bick_24 for full clock cycle.
	process ( CG_bick )
	begin
		if falling_edge( CG_bick ) then
			CG_bick_24_full <= CG_bick_24_half;
		end if;
	end process;
	-- Delay bick_24 for 3halfs clock cycles.
	process ( CG_bick )
	begin
		if rising_edge( CG_bick ) then
			CG_bick_24_3halfs <= CG_bick_24_full;
		end if;
	end process;
	
--	Connect output pins.
	CLOCK_BICK_OUT <= CG_bick;
	BICK_24_3HALFS_CLOCK_DELAY <= CG_bick_24_3halfs;
	BICK_24_FULL_CLOCK_DELAY <= CG_bick_24_full;
	CLOCK_SAMPLE_RATE_OUT <= CG_generator( 7 );

end opis;