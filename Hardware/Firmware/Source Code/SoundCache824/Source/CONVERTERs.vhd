--***************************************************************************
--***************************************************************************
--
--                          PROPRIETARY INFORMATION
--
-- This software is supplied under the terms of a license agreement or
-- nondisclosure agreement with Waldemar Haszlakiewicz and may not be
-- copied or disclosed except in accordance with the terms of that agreement.
--
--                  Copyright (C) 2005 Waldemar Haszlakiewicz
--                            All Rights Reserved.
--
--***************************************************************************
--***************************************************************************

-- Transfer engine 
LIBRARY IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;

entity CONVERTERs_engine is
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
		
		CONVERTER1_SERIAL_DATA_IN: in std_logic; -- 127
		CONVERTER2_SERIAL_DATA_IN: in std_logic; -- 126
		CONVERTER3_SERIAL_DATA_IN: in std_logic; -- 125
		CONVERTER4_SERIAL_DATA_IN: in std_logic; -- 124
			
		CONVERTER1_SERIAL_DATA_OUT: out std_logic; -- 140
		CONVERTER2_SERIAL_DATA_OUT: out std_logic; -- 139
		CONVERTER3_SERIAL_DATA_OUT: out std_logic; -- 138
		CONVERTER4_SERIAL_DATA_OUT: out std_logic -- 137
	);
end CONVERTERs_engine;

architecture opis of CONVERTERs_engine is
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
	component event_pulse_generator_rising_edge
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

-- Delayed Bick 24 signal.
	signal nBick24_2halfs: std_logic; -- 1 bick clock delay
	signal nBick24_3halfs: std_logic; -- 1.5 bick clock delay
	signal nBick24_4halfs: std_logic; -- 2 bick clock delay

-- Master/Slave communication.
	signal master_ch12_in: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal master_ch34_in: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal master_ch56_in: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal master_ch78_in: std_logic_vector ( 23 downto 0 );-- := x"000000";

	signal slave_ch12_in: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal slave_ch34_in: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal slave_ch56_in: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal slave_ch78_in: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal slave_ch12_in_gate: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal slave_ch34_in_gate: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal slave_ch56_in_gate: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal slave_ch78_in_gate: std_logic_vector ( 23 downto 0 );-- := x"000000";

	signal master_ch12_out: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal master_ch34_out: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal master_ch56_out: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal master_ch78_out: std_logic_vector ( 23 downto 0 );-- := x"000000";
	
	signal slave_ch12_out: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal slave_ch34_out: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal slave_ch56_out: std_logic_vector ( 23 downto 0 );-- := x"000000";
	signal slave_ch78_out: std_logic_vector ( 23 downto 0 );-- := x"000000";

-------------------------------------------------------------------------------
-- LOAD/READ SLAVES -----------------------------------------------------------
-------------------------------------------------------------------------------
	signal nLRS_channelSelect: std_logic_vector( 7 downto 0 );
	signal LRS_transferFromToSlave: std_logic;	-- this is a 2 BICK clk pulse

begin
-------------------------------------------------------------------------------
-- CODEC COMMUNICATION --------------------------------------------------------
-------------------------------------------------------------------------------
-- Left/Right.
--
-- Left/Right signal is really SAMPLE_RATE signal, but we'll use this
-- name so that the code will be easier to understand.
	leftRight <= SAMPLE_RATE;

-- Delayed Bick 24 signal.
--
-- This signals will show the engine when to transfer and when not to.
-- 
	-- 1 bick clock delay
	process ( BICK, nBICK_24 )
	begin
		if falling_edge( BICK ) then
			nBick24_2halfs <= nBICK_24;
		end if;
	end process;
	-- 1.5 bick clock delay
	process ( BICK, nBick24_2halfs )
	begin
		if rising_edge( BICK ) then
			nBick24_3halfs <= nBick24_2halfs;
		end if;
	end process;
	-- 2 bick clock delay
	process ( BICK, nBick24_2halfs )
	begin
		if falling_edge( BICK ) then
			nBick24_4halfs <= nBick24_2halfs;
		end if;
	end process;

-- Master/Slave communication.
--
-- The communication engine uses I2S Audio Serial Interface Format.
-- Both input and output 24bits are MSB in 48bit word.
-- DATA_OUT: new data bit is transfered on falling edge of the BICK clock and
--           is sampled on rising edge by the CONVERTER
-- DATA_IN: new data bit is received on falling edge of the BICK clock and
--           is sampled on rising edge by the register.
--           This is how it would be in the perfect world, but in real one there
--           are delay. So we will sample data on the second falling edge.
--           Why?
--           Well bick clock is an internal clock and the line is transmited to global line
--           inside Altera (fast lines) -> now this means that at the falling edge the line
--           will change on registers and at CONVERTERS_96FS_ADC at approx. the same time.
--           And as delays inside Altera are shorter than the change of the converter data,
--           we can be sure that the data on the input is still valid.
-- First 1 and last 23 bits are not used.
--
-- Take notice that bick_24_clocks signals changes state on 2 or/and
-- 1 bick clocks after left_right signal has changed -> in that way
-- we can be sure that the right data will be transfered to right channel.
--


-- inputs: inputs are just gates
-- outputs: outputs are memory cells and gates
--WALDEMAR
--Vse od zgoraj napiši da bo veljalo za spodnje.

-------------------------------------------------------------------------------
-- Channel transfer window.
--
-- Define when the transfer from/to should occure.
--





-------------------------------------------------------------------------------
-- INPUTS ---------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Channel in 1,2
 	process
	(
		BICK,
		CONVERTER1_SERIAL_DATA_IN,
		nBick24_2halfs
	)
	begin
		if rising_edge( BICK ) then
			if ( nBick24_2halfs = '0' ) then
				master_ch12_in( 23 downto 0 ) <= master_ch12_in( 22 downto 0 ) & CONVERTER1_SERIAL_DATA_IN;
			end if;
		end if;
	end process;
	process
	(
		BICK,
		LRS_transferFromToSlave,
		master_ch12_in
	)
	begin
		if falling_edge( BICK ) then
			if ( LRS_transferFromToSlave = '1' ) then
				slave_ch12_in <= master_ch12_in;
			end if;
		end if;
	end process;
	process ( nLRS_channelSelect( 0 ), slave_ch12_in )
	begin
		if ( nLRS_channelSelect( 0 ) = '0' ) then
			slave_ch12_in_gate <= slave_ch12_in;
		else
			slave_ch12_in_gate <= ( slave_ch12_in_gate'range => 'Z' );
		end if;
	end process;
	LB_DATA <= slave_ch12_in_gate;
	
-- Channel in 3, 4
 	process
	(
		BICK,
		CONVERTER2_SERIAL_DATA_IN,
		nBick24_2halfs
	)
	begin
		if rising_edge( BICK ) then
			if ( nBick24_2halfs = '0' ) then
				master_ch34_in( 23 downto 0 ) <= master_ch34_in( 22 downto 0 ) & CONVERTER2_SERIAL_DATA_IN;
			end if;
		end if;
	end process;
	process
	(
		BICK,
		LRS_transferFromToSlave,
		master_ch34_in
	)
	begin
		if falling_edge( BICK ) then
			if ( LRS_transferFromToSlave = '1' ) then
				slave_ch34_in <= master_ch34_in;
			end if;
		end if;
	end process;
	process ( nLRS_channelSelect( 1 ), slave_ch34_in )
	begin
		if ( nLRS_channelSelect( 1 ) = '0' ) then
			slave_ch34_in_gate <= slave_ch34_in;
		else
			slave_ch34_in_gate <= ( slave_ch34_in_gate'range => 'Z' );
		end if;
	end process;
	LB_DATA <= slave_ch34_in_gate;
	
-- Channel in 5, 6
 	process
	(
		BICK,
		CONVERTER3_SERIAL_DATA_IN,
		nBick24_2halfs
	)
	begin
		if rising_edge( BICK ) then
			if ( nBick24_2halfs = '0' ) then
				master_ch56_in( 23 downto 0 ) <= master_ch56_in( 22 downto 0 ) & CONVERTER3_SERIAL_DATA_IN;
			end if;
		end if;
	end process;
	process
	(
		BICK,
		LRS_transferFromToSlave,
		master_ch56_in
	)
	begin
		if falling_edge( BICK ) then
			if ( LRS_transferFromToSlave = '1' ) then
				slave_ch56_in <= master_ch56_in;
			end if;
		end if;
	end process;
	process ( nLRS_channelSelect( 2 ), slave_ch56_in )
	begin
		if ( nLRS_channelSelect( 2 ) = '0' ) then
			slave_ch56_in_gate <= slave_ch56_in;
		else
			slave_ch56_in_gate <= ( slave_ch56_in_gate'range => 'Z' );
		end if;
	end process;
	LB_DATA <= slave_ch56_in_gate;
	
-- Channel in 7, 8
 	process
	(
		BICK,
		CONVERTER4_SERIAL_DATA_IN,
		nBick24_2halfs
	)
	begin
		if rising_edge( BICK ) then
			if ( nBick24_2halfs = '0' ) then
				master_ch78_in( 23 downto 0 ) <= master_ch78_in( 22 downto 0 ) & CONVERTER4_SERIAL_DATA_IN;
			end if;
		end if;
	end process;
	process
	(
		BICK,
		LRS_transferFromToSlave,
		master_ch78_in
	)
	begin
		if falling_edge( BICK ) then
			if ( LRS_transferFromToSlave = '1' ) then
				slave_ch78_in <= master_ch78_in;
			end if;
		end if;
	end process;
	process ( nLRS_channelSelect( 3 ), slave_ch78_in )
	begin
		if ( nLRS_channelSelect( 3 ) = '0' ) then
			slave_ch78_in_gate <= slave_ch78_in;
		else
			slave_ch78_in_gate <= ( slave_ch78_in_gate'range => 'Z' );
		end if;
	end process;
	LB_DATA <= slave_ch78_in_gate;
	
-------------------------------------------------------------------------------
-- OUTPUTS --------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Channel out 1,2
	process ( LB_CLK, nLRS_channelSelect( 4 ), LB_DATA )
	begin
		if rising_edge( LB_CLK ) then
			if ( nLRS_channelSelect( 4 ) = '0' ) then
				slave_ch12_out <= LB_DATA;
			end if;
		end if;
	end process;
 	process
	(
		BICK,
		LRS_transferFromToSlave,
		nBick24_3halfs,
		slave_ch12_out
	)
	begin
		if falling_edge( BICK ) then
			if ( nBick24_3halfs = '0' ) then
				master_ch12_out( 23 downto 1 ) <= master_ch12_out( 22 downto 0 );
			elsif ( LRS_transferFromToSlave = '1' ) then
				master_ch12_out <= slave_ch12_out;
			end if;
		end if;
	end process;
	CONVERTER1_SERIAL_DATA_OUT <= not master_ch12_out( 23 );

-- Channel out 3, 4
	process ( LB_CLK, nLRS_channelSelect( 5 ), LB_DATA )
	begin
		if rising_edge( LB_CLK ) then
			if ( nLRS_channelSelect( 5 ) = '0' ) then
				slave_ch34_out <= LB_DATA;
			end if;
		end if;
	end process;
 	process
	(
		BICK,
		LRS_transferFromToSlave,
		nBick24_3halfs,
		slave_ch34_out
	)
	begin
		if falling_edge( BICK ) then
			if ( nBick24_3halfs = '0' ) then
				master_ch34_out( 23 downto 1 ) <= master_ch34_out( 22 downto 0 );
			elsif ( LRS_transferFromToSlave = '1' ) then
				master_ch34_out <= slave_ch34_out;
			end if;
		end if;
	end process;
	CONVERTER2_SERIAL_DATA_OUT <= not master_ch34_out( 23 );
	
-- Channel out 5, 6
	process ( LB_CLK, nLRS_channelSelect( 6 ), LB_DATA )
	begin
		if rising_edge( LB_CLK ) then
			if ( nLRS_channelSelect( 6 ) = '0' ) then
				slave_ch56_out <= LB_DATA;
			end if;
		end if;
	end process;
 	process
	(
		BICK,
		LRS_transferFromToSlave,
		nBick24_3halfs,
		slave_ch56_out
	)
	begin
		if falling_edge( BICK ) then
			if ( nBick24_3halfs = '0' ) then
				master_ch56_out( 23 downto 1 ) <= master_ch56_out( 22 downto 0 );
			elsif ( LRS_transferFromToSlave = '1' ) then
				master_ch56_out <= slave_ch56_out;
			end if;
		end if;
	end process;
	CONVERTER3_SERIAL_DATA_OUT <= not master_ch56_out( 23 );
	
-- Channel out 7, 8
	process ( LB_CLK, nLRS_channelSelect( 7 ), LB_DATA )
	begin
		if rising_edge( LB_CLK ) then
			if ( nLRS_channelSelect( 7 ) = '0' ) then
				slave_ch78_out <= LB_DATA;
			end if;
		end if;
	end process;
	process
	(
		BICK,
		LRS_transferFromToSlave,
		nBick24_3halfs,
		slave_ch78_out
	)
	begin
		if falling_edge( BICK ) then
			if ( nBick24_3halfs = '0' ) then
				master_ch78_out( 23 downto 1 ) <= master_ch78_out( 22 downto 0 );
			elsif ( LRS_transferFromToSlave = '1' ) then
				master_ch78_out <= slave_ch78_out;
			end if;
		end if;
	end process;
	CONVERTER4_SERIAL_DATA_OUT <= not master_ch78_out( 23 );
	
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
-------------------------------------------------------------------------------
-- transfer engine reads the data in the next order:
-- 1. chSelect goes to 0
--    at the same time the read request is send to the memory
--    the memory transfer takes 2 LB_CLK (66ns) which is more
--    than enough for data to come to appropriate register
-- 2. after 2 LB_CLKs the chSelect of current channel goes to 1
--    and chSelect of next channel goes to 0
--
-- So the data should be latched on rising edge of the chSelect.
--
-- channel 1,2 output
-- channel 3,4 output
-- channel 5,6 output
-- channel 7,8 output

-------------------------------------------------------------------------------
-- Transfer requests (be careful transfer data part is different for 4xx and 824 cards).
--
-- ntransferFromToSlave -> transfer slave data to master (or around)
-- TRANSFER_REQUEST -> signal transfer engine that it should start
--                     transfering data from/to slaves to/from memory
--
--
-- Second bick_24_3halfs signals the slave to master communication -> the pulse takes 1 bick clock.
-- After this pulse there will be a clk pulse that transfer should start.
--
-- TRANSFER PROCESS:
-- input: current CODEC data is shifted into MASTER register, previous sample data is in SLAVE
-- output: previous sample is in MASTER and is transfered to CODECs, current one is in SLAVE
--
-- LR          --|__________|----------|__________|----------|__________|----------|__  ( leftRight clock )
--
-- nB3/2       ---|___|------|___|------|___|------|___|------|___|------|___|------|_  ( nBick24_3Halfs )
--
-- T_TFS       _______|-|________|-|________|-|________|-|________|-|________|-|____  ( LRS_transferFromToSlave )
--
-- serialPort:     IN L      IN R       IN L       IN R       IN L       IN R           ( data currently shifted to master )
-- serialPort:     OUT L     OUT R      OUT L      OUT L      OUT L      OUT R          ( data currently shifted from master )
--
-- dataToSlave:         IN L       IN R       IN L       IN R       IN L       IN R     ( master contains -> slave will get )
-- dataInSlave:         OUT R      OUT L      OUT R      OUT L      OUT R      OUT L    ( slave contains -> master will get )
--
-- T_IL_OR     _______|-|___________________|-|___________________|-|_______________  ( transfer left input and right output )
--                      IN LEFT MASTER -> SLAVE
--                      OUT RIGHT SLAVE -> MASTER
--
-- DT_L        __________xxxxxxxx______________xxxxxxxx______________xxxxxxxx_______  ( transfer left input from slave and set left output to slave )
--                         IN LEFT               IN LEFT               IN LEFT
--                         OUT LEFT              OUT LEFT              OUT LEFT
--
-- T_IR_OL     __________________|-|___________________|-|___________________|-|____  ( transfer right input and left output )
--                                 IN RIGHT MASTER -> SLAVE
--                                 OUT LEFT SLAVE -> MASTER
--
-- DT_R        xxxxxxxxx____________xxxxxxxx______________xxxxxxxx______________xxxx  ( transfer right input from slave and set right output to slave )
--             IN RIGHT               IN RIGHT              IN RIGHT
--             OUT RIGHT              OUT RIGHT             OUT RIGHT
--
-- Data transfer can happen at any x position. DT is transfer of data toAndFrom slave registers.
-- So this process enables us that transferEngine has 1/2 sample of time to transfer data toAndFrom memory.
-- Now if Altera gets local bus at the end of this 1/2 sample period it can happen that some register
-- or sample will get corrupted, so this must NOT happen.
--
	EPG: event_pulse_generator_rising_edge port map
	(
		BICK,
		nBick24_4halfs,
		LRS_transferFromToSlave
	);
	TRANSFER_REQUEST <= LRS_transferFromToSlave;
	
	
-- Connect output pins.
-- TE engine has a falling edge detection pulse generator, so when slave
-- to master transfer will stop, the transfer request will be started.
-- We'll delay this signal for one bick clock just to be sure everything
-- has been finished, before any ports are opened.

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
	
	-- read from slave12
	nOUTPUT_ENABLE( 0 ) <= nREAD_WRITE_ENABLE or nAddressEnable( 0 );
	-- read from slave34
	nOUTPUT_ENABLE( 1 ) <= nREAD_WRITE_ENABLE or nAddressEnable( 1 );
	-- read from slave56
	nOUTPUT_ENABLE( 2 ) <= nREAD_WRITE_ENABLE or nAddressEnable( 2 );
	-- read from slave78
	nOUTPUT_ENABLE( 3 ) <= nREAD_WRITE_ENABLE or nAddressEnable( 3 );
	
	-- write to slave12
	nOUTPUT_ENABLE( 4 ) <= nREAD_WRITE_ENABLE or nAddressEnable( 4 );
	-- write to slave34
	nOUTPUT_ENABLE( 5 ) <= nREAD_WRITE_ENABLE or nAddressEnable( 5 );
	-- write to slave56
	nOUTPUT_ENABLE( 6 ) <= nREAD_WRITE_ENABLE or nAddressEnable( 6 );
	-- write to slave78
	nOUTPUT_ENABLE( 7 ) <= nREAD_WRITE_ENABLE or nAddressEnable( 7 );

end opis;

-- Event pulse generator (rising edge):
--
LIBRARY IEEE;
use IEEE.std_logic_1164.all;

entity event_pulse_generator_rising_edge is
	port
	(
		BICK: in std_logic;
		TRIGGER: in std_logic;
		OUTPUT_PULSE: out std_logic
	);
end event_pulse_generator_rising_edge;

architecture opis of event_pulse_generator_rising_edge is
	signal DFF_1: std_logic;
	signal DFF_2: std_logic;

begin
-- Signal delays
	process ( BICK, TRIGGER )
	begin
		if falling_edge( BICK ) then
			DFF_1 <= TRIGGER;
		end if;
	end process;
	process ( BICK, DFF_1 )
	begin
		if falling_edge( BICK ) then
			DFF_2 <= DFF_1;
		end if;
	end process;
	
-- Slave to master pulse generator
	-- input from 0 to 1
	OUTPUT_PULSE <= TRIGGER and ( not DFF_2 );
	
end opis;