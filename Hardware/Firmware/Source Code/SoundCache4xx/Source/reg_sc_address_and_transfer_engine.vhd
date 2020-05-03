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

-- Current address of audio engine (pointer to analog memory).
--
-- bit15 -  R - (0) \
-- bit14 -  R - (0) |
-- bit13 -  R - (0) |
-- bit12 -  R - (0) |
-- bit11 -  R - (0) |
-- bit10 -  R - (0) |
-- bit9  -  R - (0) |
-- bit8  -  R - (0) |->address of mono channel
-- bit7  -  R - (0) |
-- bit6  -  R - (0) |
-- bit5  -  R - (0) |
-- bit4  -  R - (0) |
-- bit3  -  R - (0) |
-- bit2  -  R - (0) |
-- bit1  -  R - (0) |
-- bit0  -  R - (0) /
--
-- Notes:
-- ( ) means value after reset.
--
--
-- As channel buffers are now separated (mono buffers) the biggest address is
-- 0x1000 (0x0000 to 0x0FFF) for 4xx cards and
-- 0x0800 (0x0000 to 0x07FF) for 824 card ->
-- this means that the upper bits are always 0.
-- NOTICE:
-- All addresses are in samples. This means that you need to multiply
-- with 4 to get the sample address.
--
-- Buffer layout:
-- 4xx cards:
-- offset    channel
-- 0x0000    input1left
-- 0x1000    input1right
-- 0x2000    input2left
-- 0x3000    input2right
-- 0x4000    output1left
-- 0x5000    output1right
-- 0x6000    output2left
-- 0x7000    output2right
--
-- 824 cards:
-- offset    channel
-- 0x0000    input1left
-- 0x0800    input1right
-- 0x1000    input2left
-- 0x1800    input2right
-- 0x2000    input3left
-- 0x2800    input3right
-- 0x3000    input4left
-- 0x3800    input5right
-- 0x4000    output1left
-- 0x4800    output1right
-- 0x5000    output2left
-- 0x5800    output2right
-- 0x6000    output3left
-- 0x6800    output3right
-- 0x7000    output4left
-- 0x7800    output4right
--
-- This layout is made with only one purpose in mind ->
-- as fast ASIO transfer as it can get.
--
LIBRARY IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;

entity reg_SC_SAMPLE_COUNTER_and_TRANSFER_ENGINE is
	port
	(
	-- Signals from main.
		LB_CLK: in std_logic;
		nLB_ENABLE_READ_ADDRESS: in std_logic;
		nLB_ENABLE_READ_COUNTER1: in std_logic;
		LB_DATA: out std_logic_vector ( 15 downto 0 );
	
	-- Pins dedicated to register.
		-- address counter
		nRESET_ENGINE: in std_logic;
		CLK_SAMPLE_RATE: in std_logic;
		
		-- memory controller
		LB_ADDRESS: out std_logic_vector ( 16 downto 0 ); -- full address ( channel + sample select )
		LB_HOLD: out std_logic; -- 45
		LB_HOLDA: in std_logic; -- 46
		LB_USER: out std_logic; -- 138
		
		nMEMORY_CE: out std_logic;
		nMEMORY_OE: out std_logic;
		nMEMORY_WE: out std_logic;

		-- transfer engine
		TE_CHANNEL_SELECT: out std_logic_vector( 2 downto 0 );
		nTE_READ_WRITE_ENABLE: out std_logic;
		nTE_STOP_REQUEST: in std_logic;
		TE_TRANSFER_REQUEST: in std_logic
	);
	
end reg_SC_SAMPLE_COUNTER_and_TRANSFER_ENGINE;

architecture opis of reg_SC_SAMPLE_COUNTER_and_TRANSFER_ENGINE is
	component sample_counter
		port
		(
			CLK_SAMPLE_RATE: in std_logic;
			nRESET_ADDRESS: in std_logic; -- asynchronous reset
			ADDRESS: out std_logic_vector ( 15 downto 0 )
		);
	end component;
	component neg_event_pulse_generator
		port
		(
			LB_CLK: in std_logic;
			TRIGGER: in std_logic;
			OUTPUT_PULSE: out std_logic
		);
	end component;

-------------------------------------------------------------------------------
--  SC_ADC_DAC_ADDRESS REGISTER  ----------------------------------------------
-------------------------------------------------------------------------------
-- Notice:
-- Address is in samples, as lower 2 bits are not used.
	signal AR_address: std_logic_vector ( 15 downto 0 );
	signal AR_address_protector: std_logic_vector ( 15 downto 0 );
	signal AR_address_status: std_logic_vector ( 15 downto 0 );
	
	signal LB_DATA_gate1: std_logic_vector ( 15 downto 0 );
	signal LB_DATA_gate2: std_logic_vector ( 15 downto 0 );
	
-------------------------------------------------------------------------------
--  TRANSFER ENGINE  ----------------------------------------------------------
-------------------------------------------------------------------------------
	signal TE_address: std_logic_vector( 11 downto 0 );
	signal TE_channelSelect: std_logic_vector( 2 downto 0 );
	signal nTE_enableTransfer: std_logic;

	signal TE_transferRequest_protector: std_logic;
	signal TE_startTransfer: std_logic;

	type TE_states is
	(
		TE_waitForTransferRequest,
		TE_requestLocalBus,
		TE_write_ch1,
		TE_write_ch1_w,
		TE_write_ch2,
		TE_write_ch2_w,
		TE_write_ch3,
		TE_write_ch3_w,
		TE_write_ch4,
		TE_write_ch4_w,
		TE_read_ch1,
		TE_read_ch1_r,
		TE_read_ch2,
		TE_read_ch2_r,
		TE_read_ch3,
		TE_read_ch3_r,
		TE_read_ch4,
		TE_read_ch4_r
	);
	signal TE_nextState, TE_currentState: TE_states;
	
begin
-------------------------------------------------------------------------------
--  SC_ADC_DAC_ADDRESS REGISTER  ----------------------------------------------
-------------------------------------------------------------------------------
	read_1: process ( nLB_ENABLE_READ_ADDRESS, AR_address_status )
	begin
		if ( nLB_ENABLE_READ_ADDRESS = '0' ) then
			LB_DATA_gate1 <= b"0000" & AR_address_status( 11 downto 0 );
		else
			LB_DATA_gate1 <= ( LB_DATA_gate1'range => 'Z' );
		end if;
	end process;
	read_2: process ( nLB_ENABLE_READ_COUNTER, AR_address_status )
	begin
		if ( nLB_ENABLE_READ_COUNTER = '0' ) then
			LB_DATA_gate2 <= AR_address_status;
		else
			LB_DATA_gate2 <= ( LB_DATA_gate2'range => 'Z' );
		end if;
	end process;
	-- This is here because if it isn't the read counter returns
	-- wrong value -> I don't know why??
	read_3: process ( nLB_ENABLE_READ_COUNTER2, AR_address_status )
	begin
		if ( nLB_ENABLE_READ_COUNTER2 = '0' ) then
			LB_DATA_gate3 <= AR_address_status;
		else
			LB_DATA_gate3 <= ( LB_DATA_gate3'range => 'Z' );
		end if;
	end process;
	LB_DATA <= LB_DATA_gate1;
	LB_DATA <= LB_DATA_gate2;
	LB_DATA <= LB_DATA_gate3;

-- counter
	AC: sample_counter port map
	(
		CLK_SAMPLE_RATE,
		nRESET_ENGINE,
		AR_address
	);
	-- glich protect
	process ( LB_CLK )
	begin
		if rising_edge( LB_CLK ) then
			AR_address_protector <= AR_address;
		end if;
	end process;
	process ( LB_CLK )
	begin
		if rising_edge( LB_CLK ) then
			AR_address_status <= AR_address_protector;
		end if;
	end process;

-------------------------------------------------------------------------------
--  TRANSFER ENGINE  ----------------------------------------------------------
-------------------------------------------------------------------------------
-- memory address
-- As soon as we get TE_startTransfer we need to lock down the current address.
-- That is because we are almost at the end of the current addres cycle so in
-- just a few bick clock pulses the address will change.
--
-- Notice:
-- TE_startTransfer is short(2 LB_CLK) long pulze.
	process( AR_address_status( 11 downto 0 ), TE_startTransfer )
	begin
		if rising_edge( TE_startTransfer ) then
			TE_address <= AR_address_status( 11 downto 0 );
		end if;
	end process;
	process ( TE_address, TE_channelSelect, nTE_enableTransfer )
	begin
		if ( nTE_enableTransfer = '0' ) then
			LB_ADDRESS <= TE_channelSelect & TE_address & "00";
		else
			LB_ADDRESS <= ( LB_ADDRESS'range => 'Z' );
		end if;
	end process;

-------------------------------------------------------------------------------
-- Tranfer request protector
-- As regSelector is used for transfer request we need to
-- create a trigger out of it -> but first we have to protect
-- transfer engine agains gliches.
-- Because event pulse generator containts a DFF at the input
-- we'll remove the 'Status' DFF from protection.
	process ( LB_CLK )
	begin
		if rising_edge( LB_CLK ) then
			TE_transferRequest_protector <= TE_TRANSFER_REQUEST;
		end if;
	end process;
	EPG: neg_event_pulse_generator port map
	(
		LB_CLK,
		TE_transferRequest_protector,
		TE_startTransfer
	);

-------------------------------------------------------------------------------
-- This engine will transfer data from register to local memory or back.
	TE_L: process ( LB_CLK, nTE_STOP_REQUEST )
	begin
		if rising_edge( LB_CLK ) then
			if ( nTE_STOP_REQUEST = '0' ) then
				TE_currentState <= TE_waitForTransferRequest;
			else
				TE_currentState <= TE_nextState; 
			end if;
		end if;
	end process;
	TE_changeState: process
	(
		TE_currentState,
		TE_startTransfer,
		LB_HOLDA
	)
	begin
		case TE_currentState is
		-----------------------------------------------------------------------------
		-- Starting state -> wait for change request.
			when TE_waitForTransferRequest =>
			-- change state?
				if ( TE_startTransfer = '1' ) then
				-- yes, go to next state
					TE_nextState <= TE_requestLocalBus;
				else
				-- no, don't change the state
					TE_nextState <= TE_waitForTransferRequest;
				end if;
						
			-- outputs
				-- memory control
				LB_HOLD <= '0';
				LB_USER <= '1';
				nMEMORY_CE <= '1';
				nMEMORY_OE <= '1';
				nMEMORY_WE <= '1';
				
				-- channel selector
				-- This variable defines which channel should receive/transmit
				-- it's data.
				nTE_enableTransfer <= '1';
				TE_channelSelect <= "000";
				
		-----------------------------------------------------------------------------
		-- Request local bus -> wait until LB_HOLDA goes high.
			when TE_requestLocalBus =>
			-- change state?
				if ( LB_HOLDA = '1' ) then
				-- yes, the bus access was granted -> go to next state
					TE_nextState <= TE_write_ch1;
				else
				-- no, don't change the state
					TE_nextState <= TE_requestLocalBus;
				end if;
					
			-- outputs
				-- memory control
				LB_HOLD <= '1';	-- request local bus
				LB_USER <= '0';
				nMEMORY_CE <= '1';
				nMEMORY_OE <= '1';
				nMEMORY_WE <= '1';
				
				-- channel selector
				nTE_enableTransfer <= '1';
				TE_channelSelect <= "000";

		-----------------------------------------------------------------------------
		-- Write channel 1 sample to memory.
			when TE_write_ch1 =>
			-- change state
				TE_nextState <= TE_write_ch1_w;
					
			-- outputs
				-- memory control
				LB_HOLD <= '1';	-- hold request of the local bus
				LB_USER <= '0';				
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '1'; 
				nMEMORY_WE <= '1'; -- write to memory

				-- channel selector
				nTE_enableTransfer <= '0';
				TE_channelSelect <= "000"; -- input1left => 0x0000 (in samples)
				
			when TE_write_ch1_w =>
			-- change state
				TE_nextState <= TE_write_ch2;
					
			-- outputs
				-- memory control
				LB_HOLD <= '1';	-- hold request of the local bus
				LB_USER <= '0';				
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '1'; 
				nMEMORY_WE <= '0'; -- write to memory

				-- channel selector
				nTE_enableTransfer <= '0';
				TE_channelSelect <= "000"; -- input1left => 0x0000

		-----------------------------------------------------------------------------				
		-- Write channel 2 sample to memory.
			when TE_write_ch2 =>
			-- change state
				TE_nextState <= TE_write_ch2_w;
					
			-- outputs
				-- memory control
				LB_HOLD <= '1';	-- hold request of the local bus
				LB_USER <= '0';				
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '1'; 
				nMEMORY_WE <= '1'; -- write to memory
				
				-- channel selector
				nTE_enableTransfer <= '0';
				TE_channelSelect <= "001"; -- input1right => 0x1000 (in samples)
				
			when TE_write_ch2_w =>
			-- change state
				TE_nextState <= TE_write_ch3;
					
			-- outputs
				-- memory control
				LB_HOLD <= '1';	-- hold request of the local bus
				LB_USER <= '0';				
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '1'; 
				nMEMORY_WE <= '0'; -- write to memory
				
				-- channel selector
				nTE_enableTransfer <= '0';
				TE_channelSelect <= "001"; -- input1right => 0x1000
				
		-----------------------------------------------------------------------------				
		-- Write channel 3 sample to memory.
			when TE_write_ch3 =>
			-- change state?
				TE_nextState <= TE_write_ch3_w;
					
			-- outputs
				-- memory control
				LB_HOLD <= '1';	-- hold request of the local bus
				LB_USER <= '0';				
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '1'; 
				nMEMORY_WE <= '1'; -- write to memory
				
				-- channel selector
				nTE_enableTransfer <= '0';
				TE_channelSelect <= "010"; -- input2left => 0x2000 (in samples)
				
			when TE_write_ch3_w =>
			-- change state?
				TE_nextState <= TE_write_ch4;
					
			-- outputs
				-- memory control
				LB_HOLD <= '1';	-- hold request of the local bus
				LB_USER <= '0';				
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '1'; 
				nMEMORY_WE <= '0'; -- write to memory
				
				-- channel selector
				nTE_enableTransfer <= '0';
				TE_channelSelect <= "010"; -- input2left => 0x2000
				
			
		-----------------------------------------------------------------------------				
		-- Write channel 4 sample to memory.
			when TE_write_ch4 =>
			-- change state
				TE_nextState <= TE_write_ch4_w;
					
			-- outputs
				-- memory control
				LB_HOLD <= '1';	-- hold request of the local bus
				LB_USER <= '0';				
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '1'; 
				nMEMORY_WE <= '1'; -- write to memory
				
				-- channel selector
				nTE_enableTransfer <= '0';
				TE_channelSelect <= "011"; -- input2right => 0x3000 (in samples)
				
			when TE_write_ch4_w =>
			-- change state
				TE_nextState <= TE_read_ch1;
					
			-- outputs
				-- memory control
				LB_HOLD <= '1';	-- hold request of the local bus
				LB_USER <= '0';				
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '1'; 
				nMEMORY_WE <= '0'; -- write to memory
				
				-- channel selector
				nTE_enableTransfer <= '0';
				TE_channelSelect <= "011"; -- input2right => 0x3000
				
		-----------------------------------------------------------------------------				
		-- Read channel 1 sample from memory.
			when TE_read_ch1 =>
			-- change state
				TE_nextState <= TE_read_ch1_r;
					
			-- outputs
				-- memory control
				LB_HOLD <= '1';	-- hold request of the local bus
				LB_USER <= '0';				
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '0'; 
				nMEMORY_WE <= '1'; -- write to memory

				-- channel selector
				nTE_enableTransfer <= '0';
				TE_channelSelect <= "100"; -- output1left => 0x4000 (in samples)
				
			when TE_read_ch1_r =>
			-- change state
				TE_nextState <= TE_read_ch2;
					
			-- outputs
				-- memory control
				LB_HOLD <= '1';	-- hold request of the local bus
				LB_USER <= '0';				
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '0'; 
				nMEMORY_WE <= '1'; -- write to memory

				-- channel selector
				nTE_enableTransfer <= '0';
				TE_channelSelect <= "100"; -- output1left => 0x4000

		-----------------------------------------------------------------------------				
		-- Read channel 2 sample from memory.
			when TE_read_ch2 =>
			-- change state
				TE_nextState <= TE_read_ch2_r;
					
			-- outputs
				-- memory control
				LB_HOLD <= '1';	-- hold request of the local bus
				LB_USER <= '0';				
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '0'; 
				nMEMORY_WE <= '1'; -- write to memory

				-- channel selector
				nTE_enableTransfer <= '0';
				TE_channelSelect <= "101"; -- output1right => 0x5000 (in samples)
				
			when TE_read_ch2_r =>
			-- change state
				TE_nextState <= TE_read_ch3;
					
			-- outputs
				-- memory control
				LB_HOLD <= '1';	-- hold request of the local bus
				LB_USER <= '0';				
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '0'; 
				nMEMORY_WE <= '1'; -- write to memory

				-- channel selector
				nTE_enableTransfer <= '0';
				TE_channelSelect <= "101"; -- output1right => 0x5000

		-----------------------------------------------------------------------------				
		-- Read channel 3 sample from memory.
			when TE_read_ch3 =>
			-- change state
				TE_nextState <= TE_read_ch3_r;
					
			-- outputs
				-- memory control
				LB_HOLD <= '1';	-- hold request of the local bus
				LB_USER <= '0';				
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '0'; 
				nMEMORY_WE <= '1'; -- write to memory

				-- channel selector
				nTE_enableTransfer <= '0';
				TE_channelSelect <= "110"; -- output2left => 0x6000 (in samples)

			when TE_read_ch3_r =>
			-- change state
				TE_nextState <= TE_read_ch4;
					
			-- outputs
				-- memory control
				LB_HOLD <= '1';	-- hold request of the local bus
				LB_USER <= '0';				
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '0'; 
				nMEMORY_WE <= '1'; -- write to memory

				-- channel selector
				nTE_enableTransfer <= '0';
				TE_channelSelect <= "110"; -- output2left => 0x6000

		-----------------------------------------------------------------------------				
		-- Read channel 4 sample from memory.
			when TE_read_ch4 =>
			-- change state
				TE_nextState <= TE_read_ch4_r;
					
			-- outputs
				-- memory control
				LB_HOLD <= '1';	-- hold request of the local bus
				LB_USER <= '0';				
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '0'; 
				nMEMORY_WE <= '1'; -- write to memory

				-- channel selector
				nTE_enableTransfer <= '0';
				TE_channelSelect <= "111"; -- output2right => 0x7000 (in samples)

			-- TE_read_ch4_r
			when others =>
			-- change state
				TE_nextState <= TE_waitForTransferRequest;
					
			-- outputs
				-- memory control
				LB_HOLD <= '1';	-- hold request of the local bus
				LB_USER <= '0';				
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '0'; 
				nMEMORY_WE <= '1'; -- write to memory

				-- channel selector
				nTE_enableTransfer <= '0';
				TE_channelSelect <= "111"; -- output2right => 0x7000

		end case;
	end process;

-- Connectoutput pins.
	TE_CHANNEL_SELECT <= TE_channelSelect;
	nTE_READ_WRITE_ENABLE <= nTE_enableTransfer;

end opis;

LIBRARY IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;

entity sample_counter is
	port
	(
		CLK_SAMPLE_RATE: in std_logic;
		nRESET_ADDRESS: in std_logic; -- asynchronous reset
		ADDRESS: out std_logic_vector( 15 downto 0 )
	);
end sample_counter;

architecture opis of sample_counter is
	signal counter: std_logic_vector( 15 downto 0 );
	
begin
	process ( CLK_SAMPLE_RATE, nRESET_ADDRESS )
	begin
		if ( nRESET_ADDRESS = '0' ) then
			counter <= ( counter'range => '0' );
		elsif falling_edge( CLK_SAMPLE_RATE ) then
			counter <= counter + x"0001";
		end if;
	end process;
	
-- Connect output pins.
	ADDRESS <= counter;
	
end opis;

-------------------------------------------------------------------------------
--  PULSE GENERATOR  ----------------------------------------------------------
-------------------------------------------------------------------------------
LIBRARY IEEE;
use IEEE.std_logic_1164.all;

entity neg_event_pulse_generator is
	port
	(
		LB_CLK: in std_logic;
		TRIGGER: in std_logic;
		OUTPUT_PULSE: out std_logic
	);
end neg_event_pulse_generator;

architecture opis of neg_event_pulse_generator is
	signal DFF_1: std_logic;
	signal DFF_2: std_logic;
	signal DFF_3: std_logic;
	
begin
-- Signal delays
	process ( LB_CLK )
	begin
		if rising_edge( LB_CLK ) then
			DFF_1 <= TRIGGER;
		end if;
	end process;
	process ( LB_CLK )
	begin
		if rising_edge( LB_CLK ) then
			DFF_2 <= DFF_1;
		end if;
	end process;
	process ( LB_CLK )
	begin
		if rising_edge( LB_CLK ) then
			DFF_3 <= DFF_2;
		end if;
	end process;
	
-- Pulse generator
	-- input from 1 to 0
	OUTPUT_PULSE <= ( not DFF_1 ) and DFF_3;

end opis;