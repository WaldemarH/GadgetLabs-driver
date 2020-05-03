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

-- Transfer engine 
LIBRARY IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;

entity CODEC_engine is
	port
	(
	-- Signals from main.
		LB_CLK: in std_logic;
		LB_DATA: inout std_logic_vector ( 23 downto 0 );
	
	-- Pins dedicated to register.
		nRESET_ENGINE: in std_logic;
		
		-- transfer engine
		CHANNEL_SELECT: in std_logic_vector( 2 downto 0 );
		nREAD_WRITE_ENABLE: in std_logic;
		TRANSFER_REQUEST: out std_logic;
	
		-- CODEC data transfer
		BICK: in std_logic;
		nBICK_24: in std_logic;
		SAMPLE_RATE: in std_logic;
		
		CODEC1_SERIAL_DATA_IN: in std_logic; -- 88
		CODEC2_SERIAL_DATA_IN: in std_logic; -- 87
		CODEC1_SERIAL_DATA_OUT: out std_logic; -- 94
		CODEC2_SERIAL_DATA_OUT: out std_logic -- 93
	);
end CODEC_engine;

architecture opis of CODEC_engine is
-------------------------------------------------------------------------------
-- COMPONENTS -----------------------------------------------------------------
-------------------------------------------------------------------------------
	component lineDecoder_3to8b
		port
		(
			CHANNEL_SELECT: in std_logic_vector ( 2 downto 0 );
			nREAD_WRITE_ENABLE: in std_logic;
			nOUTPUT_ENABLE: out std_logic_vector ( 7 downto 0 )
		);
	end component;
	component event_pulse_generator
		port
		(
			BICK: in std_logic;
			TRIGGER: in std_logic;
			OUTPUT_PULSE: out std_logic
		);
	end component;
	
-------------------------------------------------------------------------------
-- CODEC COMMUNICATION --------------------------------------------------------
-------------------------------------------------------------------------------
-- Left/Right.
	signal leftRight: std_logic;

-- Delayed Bick 24 signals.
	signal nBick24_2Halfs: std_logic; -- 1.0 bick clock delay
	signal nBick24_3Halfs: std_logic; -- 1.5 bick clock delay

-- Master/Slave communication.
	signal master_ch1_in: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal slave_ch1_in: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal master_ch2_in: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal slave_ch2_in: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal master_ch3_in: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal slave_ch3_in: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal master_ch4_in: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal slave_ch4_in: std_logic_vector ( 23 downto 0 );-- := x"000000";

	signal master_ch1_out: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal slave_ch1_out: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal master_ch2_out: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal slave_ch2_out: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal master_ch3_out: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal slave_ch3_out: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal master_ch4_out: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal slave_ch4_out: std_logic_vector ( 23 downto 0 );-- := x"000000";

	signal codec1_serial_out: std_logic;
	signal codec2_serial_out: std_logic;

-- Transfer window flags shows register when to copy data from/to CODEC.
	signal nTransferWindow_ch13_in: std_logic;
	signal nTransferWindow_ch24_in: std_logic;
	signal nTransferWindow_ch13_out: std_logic;
	signal nTransferWindow_ch24_out: std_logic;

-------------------------------------------------------------------------------
-- LOAD/READ SLAVES -----------------------------------------------------------
-------------------------------------------------------------------------------
	signal nLRS_channelSelect: std_logic_vector( 7 downto 0 );
	signal LRS_transferFromToSlave: std_logic;
	signal LRS_counter: std_logic_vector( 1 downto 0 );

	signal slave_ch1_in_gate: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal slave_ch2_in_gate: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal slave_ch3_in_gate: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal slave_ch4_in_gate: std_logic_vector ( 23 downto 0 );-- := x"000000";

begin
-------------------------------------------------------------------------------
-- CODEC COMMUNICATION --------------------------------------------------------
-------------------------------------------------------------------------------
-- Left/Right.
--
-- Left/Right signal is really SAMPLE_RATE signal, but we'll use this
-- name so that the code will be easier to understand.
--
-- Left = 1, Right = 0
--
-- Notice:
-- SAMPLE_RATE is rising edge clock (BICK is falling edge).
	leftRight <= SAMPLE_RATE;

-- Delayed Bick 24 signal.
--
-- This signal will show the engine when to transfer and when not to.
-- 
-- DA starts transfer at bick_24 signal (data should be transfered at falling edge and
-- it's lateched by the DA converter at rising edge).
-- AD starts transfer bick_24 signal (data should be inputed at falling edge and
-- it's latched by Altera at rising edge).
--
	-- 2/2 bick clock delay
	process ( BICK, nBICK_24 )
	begin
		if falling_edge( BICK ) then
			nBick24_2Halfs <= nBICK_24;
		end if;
	end process;
	-- 3/2 bick clock delay
	process ( BICK, nBick24_2Halfs )
	begin
		if rising_edge( BICK ) then
			nBick24_3Halfs <= nBick24_2Halfs;
		end if;
	end process;

-- Master/Slave communication.
--
-- The communication engine uses MODE 3 Audio Serial Interface Format (AK4524).
-- Both input and output 24bits are MSB in 48bit word.
-- DATA_OUT: new data bit is transfered on falling edge of the BICK clock and
--           is sampled on rising edge by the CODEC
-- DATA_IN: new data bit is received on falling edge of the BICK clock and
--           is sampled on rising edge by the register.
-- Notice: First and last 23 bits are not used.
--
-- Notice2:
-- I've tried to use the MODE 2 audio interface, but the codecs didn't want to produce
-- any sound, so MODE 3 was used.
--
-- Extra notice:
-- As 4xx cards invert the phase of the input and output channel
-- (hardware bug). So we'll invert the phase in here,
-- so everything will be aligned properly to 824 cards.
--
-- 
-------------------------------------------------------------------------------
-- channel 1, 3 input
	nTransferWindow_ch13_in <= nBick24_2Halfs or leftRight;
	--nTransferWindow -> window is created at 24bit data present and it's latched on rising edge
	--
 	process
	(
		BICK,
		CODEC1_SERIAL_DATA_IN,
		CODEC2_SERIAL_DATA_IN,
		master_ch1_in,
		master_ch3_in,
		nTransferWindow_ch13_in,
		LRS_transferFromToSlave
	)
	begin
		if rising_edge( BICK ) then
			if ( nTransferWindow_ch13_in = '0' ) then
				master_ch1_in( 23 downto 0 ) <= master_ch1_in( 22 downto 0 ) & not( CODEC1_SERIAL_DATA_IN );
				master_ch3_in( 23 downto 0 ) <= master_ch3_in( 22 downto 0 ) & not( CODEC2_SERIAL_DATA_IN );
			end if;
		elsif falling_edge( BICK ) then
			if ( LRS_transferFromToSlave = '1' ) then
				slave_ch1_in <= master_ch1_in;
				slave_ch3_in <= master_ch3_in;
			end if;
	
		end if;
	end process;
	
-- channel 2, 4 input
	nTransferWindow_ch24_in <= nBick24_2Halfs or ( not leftRight );
    --nTransferWindow -> window is created at 24bit data present and it's latched on rising edge
	--
 	process
	(
		BICK,
		CODEC1_SERIAL_DATA_IN,
		CODEC2_SERIAL_DATA_IN,
		master_ch2_in,
		master_ch4_in,
		nTransferWindow_ch24_in,
		LRS_transferFromToSlave
	)
	begin
		if rising_edge( BICK ) then
			if ( nTransferWindow_ch24_in = '0' ) then
				master_ch2_in( 23 downto 0 ) <= master_ch2_in( 22 downto 0 ) & not( CODEC1_SERIAL_DATA_IN );
				master_ch4_in( 23 downto 0 ) <= master_ch4_in( 22 downto 0 ) & not( CODEC2_SERIAL_DATA_IN );
			end if;
		elsif falling_edge( BICK ) then
			if ( LRS_transferFromToSlave = '1' ) then
				slave_ch2_in <= master_ch2_in;
				slave_ch4_in <= master_ch4_in;
			end if;
		end if;
	end process;
	
-------------------------------------------------------------------------------
-- channel 1, 3 output
--
-- Notice:
-- nBICK_24 will eliminate last UNNEEDED falling edge.
	nTransferWindow_ch13_out <= nBick24_3Halfs or leftRight or nBick24_2Halfs;
	-- master/slave
 	process
	(
		BICK,
		slave_ch1_out,
		slave_ch3_out,
		LRS_transferFromToSlave,
		nTransferWindow_ch13_out
	)
	begin
		if falling_edge( BICK ) then
			if ( nTransferWindow_ch13_out = '0' ) then
				master_ch1_out( 23 downto 1 ) <= master_ch1_out( 22 downto 0 );
				master_ch3_out( 23 downto 1 ) <= master_ch3_out( 22 downto 0 );
			elsif ( LRS_transferFromToSlave = '1' ) then
				master_ch1_out <= slave_ch1_out;
				master_ch3_out <= slave_ch3_out;				
			end if;
		end if;
	end process;
-- channel 2, 4 output
	nTransferWindow_ch24_out <= nBick24_3Halfs or ( not leftRight ) or nBick24_2Halfs;
	-- master/slave
 	process
	(
		BICK,
		slave_ch2_out,
		slave_ch4_out,
		LRS_transferFromToSlave,
		nTransferWindow_ch24_out
	)
	begin
		if falling_edge( BICK ) then
			if ( nTransferWindow_ch24_out = '0' ) then
				master_ch2_out( 23 downto 1 ) <= master_ch2_out( 22 downto 0 );
				master_ch4_out( 23 downto 1 ) <= master_ch4_out( 22 downto 0 );
			elsif ( LRS_transferFromToSlave = '1' ) then
				master_ch2_out <= slave_ch2_out;
				master_ch4_out <= slave_ch4_out;
			end if;
		end if;
	end process;
	
	codec1_serial_out <= master_ch1_out( 23 ) when ( leftRight = '0' ) else master_ch2_out( 23 );
	codec2_serial_out <= master_ch3_out( 23 ) when ( leftRight = '0' ) else master_ch4_out( 23 );

	CODEC1_SERIAL_DATA_OUT <= not( codec1_serial_out );
	CODEC2_SERIAL_DATA_OUT <= not( codec2_serial_out );

-------------------------------------------------------------------------------
-- LOAD/READ SLAVES -----------------------------------------------------------
-------------------------------------------------------------------------------
	LD_CH: lineDecoder_3to8b port map
	(
		CHANNEL_SELECT,
		nREAD_WRITE_ENABLE,
		nLRS_channelSelect
	);

-------------------------------------------------------------------------------
-- inputs: inputs are just gates
-- outputs: outputs are memory cells and gates
--
-- channel 1 input
	process ( nLRS_channelSelect( 0 ), slave_ch1_in )
	begin
		if ( nLRS_channelSelect( 0 ) = '0' ) then
			slave_ch1_in_gate <= slave_ch1_in;
		else
			slave_ch1_in_gate <= ( slave_ch1_in_gate'range => 'Z' );
		end if;
	end process;
	LB_DATA <= slave_ch1_in_gate;
-- channel 2 input
	process ( nLRS_channelSelect( 1 ), slave_ch2_in )
	begin
		if ( nLRS_channelSelect( 1 ) = '0' ) then
			slave_ch2_in_gate <= slave_ch2_in;
		else
			slave_ch2_in_gate <= ( slave_ch2_in_gate'range => 'Z' );
		end if;
	end process;
	LB_DATA <= slave_ch2_in_gate;
-- channel 3 input
	process ( nLRS_channelSelect( 2 ), slave_ch3_in )
	begin
		if ( nLRS_channelSelect( 2 ) = '0' ) then
			slave_ch3_in_gate <= slave_ch3_in;
		else
			slave_ch3_in_gate <= ( slave_ch3_in_gate'range => 'Z' );
		end if;
	end process;
	LB_DATA <= slave_ch3_in_gate;
-- channel 4 input
	process ( nLRS_channelSelect( 3 ), slave_ch4_in )
	begin
		if ( nLRS_channelSelect( 3 ) = '0' ) then
			slave_ch4_in_gate <= slave_ch4_in;
		else
			slave_ch4_in_gate <= ( slave_ch4_in_gate'range => 'Z' );
		end if;
	end process;
	LB_DATA <= slave_ch4_in_gate;
	
-------------------------------------------------------------------------------
-- transfer engine reads the data in the next order:
-- 1. chSelect goes to 0
--    at the same time the read request is send to the memory
--    the memory trasnfer takes 2 LB_CLK (66ns) which is more
--    than enough for data to come to appropriate register
-- 2. after 2 LB_CLKs the chSelect of current channel goes to 1
--    and chSelect of next channel goes to 0
--
-- So the data should be latched on rising edge of the chSelect.
--
-- channel 1 output
	process ( LB_CLK, nLRS_channelSelect( 4 ), LB_DATA )
	begin
		if rising_edge( LB_CLK ) then
			if ( nLRS_channelSelect( 4 ) = '0' ) then
				slave_ch1_out <= LB_DATA;
			end if;
		end if;
	end process;
-- channel 2 output
	process ( LB_CLK, nLRS_channelSelect( 5 ), LB_DATA )
	begin
		if rising_edge( LB_CLK ) then
			if ( nLRS_channelSelect( 5 ) = '0' ) then
				slave_ch2_out <= LB_DATA;
			end if;
		end if;
	end process;
-- channel 3 output
	process ( LB_CLK, nLRS_channelSelect( 6 ), LB_DATA )
	begin
		if rising_edge( LB_CLK ) then
			if ( nLRS_channelSelect( 6 ) = '0' ) then
				slave_ch3_out <= LB_DATA;
			end if;
		end if;
	end process;
-- channel 4 output
	process ( LB_CLK, nLRS_channelSelect( 7 ), LB_DATA )
	begin
		if rising_edge( LB_CLK ) then
			if ( nLRS_channelSelect( 7 ) = '0' ) then
				slave_ch4_out <= LB_DATA;
			end if;
		end if;
	end process;

-------------------------------------------------------------------------------
-- Transfer requests (be careful transfer data part is different for 4xx and 824 cards).
--
-- ntransferFromToSlave -> transfer slave data to master (or around)
-- TRANSFER_REQUEST -> signal transfer engine that it should start
--                     transfering data from/to slaves to/from memory
--
--
-- Second BICK_24 signals the slave to master communication -> the pulse takes 2 bick clocks.
-- After this pulse there will be a clk pulse that transfer should start.
--
-- TRANSFER PROCESS:
-- input: current CODEC data is shifted into MASTER register, previous sample data is in SLAVE
-- output: previous sample is in MASTER and is transfered to CODECs, current one is in SLAVE
--
-- LR          --|__________|----------|__________|----------|__________|----------|__  ( leftRight clock )
--
-- transfer:        1,3        2,4        1,3        2,4        1,3        2,4
-- nBICK24     ---|_____|----|_____|----|_____|----|_____|----|_____|----|_____|----|_  ( nBick24_2Halfs )
--
--                (at reset)
-- LRS_C       ---(00)-->01-------->10-------->11-------->00-------->01-------->10----  ( LRS_counter )
--
-- LRS_C(1)    ____________________|---------------------|_____________________|------  ( LRS_counter(1) )
--
-- T_TFS       |___________________|-|___________________|-|___________________|-|____  ( LRS_transferFromToSlave )
--
-- DT          _xxxxxxxxxxxxxxxxxxx___xxxxxxxxxxxxxxxxxxx___xxxxxxxxxxxxxxxxxxx___xxxx  ( data transfer )
--             
-- Data transfer can happen at any x position (DT is transfer of data toAndFrom slave registers).
-- So this process enables us that transferEngine has 1 sample of time to transfer data toAndFrom memory.
-- Now if Altera gets local bus at the end of this 1 sample period it can happen that some register
-- or sample will get corrupted, so this must NOT happen, so DON'T transfer too much data to the card
-- in one pass.
	process ( nBick24_2Halfs, nRESET_ENGINE )
	begin
		if ( nRESET_ENGINE = '0' ) then
			LRS_counter <= ( LRS_counter'range => '0' );
		elsif rising_edge( nBick24_2Halfs ) then
			LRS_counter <= LRS_counter + "01";
		end if;
	end process;
	
	-- Create transfer pulse (2 BICK clks):
	-- Longest  pulse (for fs = 44100Hz) = 472.42ns
	-- Shortest pulse (for fs = 96000Hz) = 217.02ns
	-- PCI clock pulse                   =  30.30ns
	EPG: event_pulse_generator port map
	(
		BICK,
		LRS_counter( 1 ),
		LRS_transferFromToSlave
	);
	
-- Connect output pins.
	TRANSFER_REQUEST <= LRS_transferFromToSlave;

end opis;

-------------------------------------------------------------------------------
-- 3TO8 LINE DECODER -> SELECTOR ----------------------------------------------
-------------------------------------------------------------------------------
LIBRARY IEEE;
use IEEE.std_logic_1164.all;

entity lineDecoder_3to8b is
	port
	(
		CHANNEL_SELECT: in std_logic_vector ( 2 downto 0 );
		nREAD_WRITE_ENABLE: in std_logic;
		nOUTPUT_ENABLE: out std_logic_vector ( 7 downto 0 )
	);
end lineDecoder_3to8b;

architecture opis of lineDecoder_3to8b is
	signal nAddressEnable: std_logic_vector ( 7 downto 0 );
	
begin
	nAddressEnable( 0 ) <= '0' when ( CHANNEL_SELECT = b"000" ) else '1';
	nAddressEnable( 1 ) <= '0' when ( CHANNEL_SELECT = b"001" ) else '1';
	nAddressEnable( 2 ) <= '0' when ( CHANNEL_SELECT = b"010" ) else '1';
	nAddressEnable( 3 ) <= '0' when ( CHANNEL_SELECT = b"011" ) else '1';
	nAddressEnable( 4 ) <= '0' when ( CHANNEL_SELECT = b"100" ) else '1';
	nAddressEnable( 5 ) <= '0' when ( CHANNEL_SELECT = b"101" ) else '1';
	nAddressEnable( 6 ) <= '0' when ( CHANNEL_SELECT = b"110" ) else '1';
	nAddressEnable( 7 ) <= '0' when ( CHANNEL_SELECT = b"111" ) else '1';
	
	nOUTPUT_ENABLE( 0 ) <= nREAD_WRITE_ENABLE or nAddressEnable( 0 );
	nOUTPUT_ENABLE( 1 ) <= nREAD_WRITE_ENABLE or nAddressEnable( 1 );
	nOUTPUT_ENABLE( 2 ) <= nREAD_WRITE_ENABLE or nAddressEnable( 2 );
	nOUTPUT_ENABLE( 3 ) <= nREAD_WRITE_ENABLE or nAddressEnable( 3 );
	nOUTPUT_ENABLE( 4 ) <= nREAD_WRITE_ENABLE or nAddressEnable( 4 );
	nOUTPUT_ENABLE( 5 ) <= nREAD_WRITE_ENABLE or nAddressEnable( 5 );
	nOUTPUT_ENABLE( 6 ) <= nREAD_WRITE_ENABLE or nAddressEnable( 6 );
	nOUTPUT_ENABLE( 7 ) <= nREAD_WRITE_ENABLE or nAddressEnable( 7 );

end opis;

-- Event pulse generator:
--
-- For every event (rising edge and falling edge) the pulse generator
-- creates 2 BICK clocks long pulze.
--
LIBRARY IEEE;
use IEEE.std_logic_1164.all;

entity event_pulse_generator is
	port
	(
		BICK: in std_logic;
		TRIGGER: in std_logic;
		OUTPUT_PULSE: out std_logic
	);
end event_pulse_generator;

architecture opis of event_pulse_generator is
	signal DFF_1: std_logic;
	signal DFF_2: std_logic;
	
	signal from0To1: std_logic;
	signal from1To0: std_logic;
	
begin
-- Signal delays
	process ( BICK )
	begin
		if falling_edge( BICK ) then
			DFF_1 <= TRIGGER;
		end if;
	end process;
	process ( BICK )
	begin
		if falling_edge( BICK ) then
			DFF_2 <= DFF_1;
		end if;
	end process;
	
-- Slave to master pulses generator
	-- input from 0 to 1
	from0To1 <= TRIGGER and ( not DFF_2 );
	-- input from 1 to 0
	from1To0 <= ( not TRIGGER ) and DFF_2;
	OUTPUT_PULSE <= from0To1 or from1To0;
	
end opis;