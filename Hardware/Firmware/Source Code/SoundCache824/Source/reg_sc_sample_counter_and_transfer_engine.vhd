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

--This variable is a common 16bit wide up counter.
--Now this value can be used for a channel address(this is how it's done in
--hardware.
--Now the counter is reset when SC_RELEASE_COUNTERS (SC_AUDIO_ENGINE) goes low.
--
--Now to get the channel address all we need to do is mask out the upper
--4bits(4xx cards) or upper 5bits(824 cards).
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
		nLB_ENABLE_READ_COUNTER: in std_logic;
		LB_DATA: out std_logic_vector ( 15 downto 0 );
		
		-- Daughter card.
		DC_nFINISHED_TRANSFER: in std_logic;
		DC_nSTART_TRANSFER: out std_logic;
	
		-- Next one should be commented out on final release.
		--(well it looks it will stay as it's a good way to detect
		-- transfer problems)
		nLB_ENABLE_READ_BUS_WAIT_TIMER: in std_logic;
		nLB_ENABLE_WRITE_BUS_WAIT_TIMER: in std_logic;
	
	-- Pins dedicated to register.
		-- address counter
		nRESET_ENGINE: in std_logic;
		CLK_SAMPLE_RATE: in std_logic;
		
		-- memory controller
		LB_ADDRESS_16TO2: out std_logic_vector ( 14 downto 0 ); -- full address ( channel + sample select )
		LB_HOLD: out std_logic; -- 60
		LB_HOLDA: in std_logic; -- 61
		
		nMEMORY_CE: out std_logic;
		nMEMORY_OE: out std_logic;
		nMEMORY_WE: out std_logic;

		-- transfer engine
		TE_CHANNEL_SELECT: out std_logic_vector( 2 downto 0 );	-- 8 channels (8 in, 8 out)
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
			COUNTER: out std_logic_vector( 15 downto 0 )
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
--  SC_SAMPLE_COUNTER REGISTER  ----------------------------------------------
-------------------------------------------------------------------------------
-- Notice:
-- Address is in samples, as lower 2 bits are not used.
	signal SCR_counter: std_logic_vector ( 15 downto 0 );
	signal SCR_counter_protector: std_logic_vector ( 15 downto 0 );
	signal SCR_counter_status: std_logic_vector ( 15 downto 0 );
	signal SCR_gate: std_logic_vector( 15 downto 0 );
	
	signal SCR_channel_protector: std_logic;
	signal SCR_channel_status: std_logic;
	
-------------------------------------------------------------------------------
--  BUS WAIT TIMER  -----------------------------------------------------------
------------------------------------------------------------------------------
	signal BWT_busTimer: std_logic_vector( 8 downto 0 );
	signal BWT_register: std_logic_vector( 8 downto 0 );
	signal BWT_register_gate: std_logic_vector( 15 downto 0 );
	
-------------------------------------------------------------------------------
--  TRANSFER ENGINE  ----------------------------------------------------------
-------------------------------------------------------------------------------
	signal TE_allowAddressChange: std_logic;
	signal TE_allowAddressChange_gated: std_logic;
	signal TE_upperAddress: std_logic_vector( 2 downto 0 );
	signal TE_fullAddress_16TO2: std_logic_vector( 14 downto 0 );
	signal nTE_enableTransfer: std_logic; -- This line enables the address
										  -- transfer to local bus address lines.
	signal TE_LB_HOLD: std_logic;

	signal TE_transferRequest_protector: std_logic;
	signal TE_startTransfer: std_logic;

	type TE_states is
	(
		TE_waitForTransferRequest,
		TE_requestLocalBus,
		TE_write_ch12,
		TE_write_ch12_w,
		TE_write_ch34,
		TE_write_ch34_w,
		TE_write_ch56,
		TE_write_ch56_w,
		TE_write_ch78,
		TE_write_ch78_w,
		TE_read_ch12,
		TE_read_ch12_r,
		TE_read_ch34,
		TE_read_ch34_r,
		TE_read_ch56,
		TE_read_ch56_r,
		TE_read_ch78,
		TE_read_ch78_r,
		TE_DaughterCard_transferRequest
	);
	signal TE_nextState, TE_currentState: TE_states;
	
begin
-------------------------------------------------------------------------------
--  SC_SAMPLE_COUNTER REGISTER  -----------------------------------------------
-------------------------------------------------------------------------------
	readCounterValue: process ( nLB_ENABLE_READ_COUNTER, SCR_counter_status )
	begin
		if ( nLB_ENABLE_READ_COUNTER = '0' ) then
			SCR_gate <= SCR_counter_status;
		else
			SCR_gate <= ( SCR_gate'range => 'Z' );
		end if;
	end process;
	LB_DATA <= SCR_gate;

-- counter
--
-- Counter defines current sample. This value is used for sample counter -> for driver
-- and for defining the current local bus address.
-- Notice:
-- This value is allowed to change only if TE_allowAddressChange is 1 AND if 
-- SCR_channel_status is 0.
-- This means that the clock status will only be allowed to change on the left
-- part of the sample clock.
	AC: sample_counter port map
	(
		CLK_SAMPLE_RATE,
		nRESET_ENGINE,
		SCR_counter
	);
	-- glich protect
	process ( LB_CLK, SCR_counter )
	begin
		if rising_edge( LB_CLK ) then
			SCR_counter_protector <= SCR_counter;
		end if;
	end process;
	process ( LB_CLK, SCR_counter_protector, TE_allowAddressChange_gated )
	begin
		if rising_edge( LB_CLK ) then
			if ( TE_allowAddressChange_gated = '1' ) then
				SCR_counter_status <= SCR_counter_protector;
			end if;
		end if;
	end process;

-- channel selector (left/right signal)
--
-- As soon as transfer request will come the SCR_channel_status will contain
-- the left/right definition. This is because the transfer request commes at a few
-- bick clocks after the left/right line changes.
--
	-- glich protector
	process ( LB_CLK, CLK_SAMPLE_RATE )
	begin
		if rising_edge( LB_CLK ) then
			SCR_channel_protector <= CLK_SAMPLE_RATE; -- this is also left/right signal
		end if;
	end process;
	process ( LB_CLK, SCR_channel_protector, TE_allowAddressChange )
	begin
		if rising_edge( LB_CLK ) then
			if ( TE_allowAddressChange = '1' ) then
				SCR_channel_status <= SCR_channel_protector;
			end if;
		end if;
	end process;
	TE_allowAddressChange_gated <= TE_allowAddressChange and ( not SCR_channel_status );
	
-------------------------------------------------------------------------------
--  BUS WAIT TIMER  -----------------------------------------------------------
-------------------------------------------------------------------------------
-- This part should be commented out on final release.
--
-- Wait for bus timer.
	process( LB_CLK, TE_LB_HOLD, LB_HOLDA )
	begin
		if rising_edge( LB_CLK ) then
			if ( TE_LB_HOLD = '0' ) then
			-- reset the counter
				BWT_busTimer <= "000000000";
			else
			-- LB_HOLD = '1'
				if ( LB_HOLDA = '0' ) then
				-- count time from LB_HOLD = '1' to LB_HOLDA = '1'
					BWT_busTimer <= BWT_busTimer + "000000001";
				end if;
			end if;
		end if;
	end process;
-- Transfer current timer value to register.
	process ( LB_CLK, LB_HOLDA, BWT_busTimer, nLB_ENABLE_WRITE_BUS_WAIT_TIMER )
	begin
		if rising_edge( LB_CLK ) then
			if ( nLB_ENABLE_WRITE_BUS_WAIT_TIMER = '0' ) then
				BWT_register <= "000000000";
			elsif ( LB_HOLDA = '1' ) then
				if ( BWT_busTimer > BWT_register ) then
					BWT_register <= BWT_busTimer;
				end if;
			end if;
		end if;
	end process;
-- Read register
	process ( nLB_ENABLE_READ_BUS_WAIT_TIMER, BWT_register )
	begin
		if ( nLB_ENABLE_READ_BUS_WAIT_TIMER = '0' ) then
			BWT_register_gate <= "0000000" & BWT_register;
		else
			BWT_register_gate <= ( BWT_register_gate'range => 'Z' );
		end if;
	end process;
	LB_DATA <= BWT_register_gate;
	
-------------------------------------------------------------------------------
--  TRANSFER ENGINE  ----------------------------------------------------------
-------------------------------------------------------------------------------
-- memory address:
--
-- Notice2:
-- 4xx cards address is 12bits wide
-- 824 cards address is 11bits wide
--	
-- Because lower 2 bits are not used we'll just set them
-- to "00" (remember we are working with samples and not bytes).
--
-- As soon as we get TE_startTransfer we need to lock down the current address.
-- That is because we are almost at the end of the current addres cycle so in
-- just a few bick clock pulses the address will change.
-- So we will stop the updating of the SCR_counter_status and with this
-- the address will be locked.
-- Now in this time no register read can happen as Altera owns the local bus.
--
--NOTICE:
--At one test this register was removed and for stop refreshing just
--TE_allowAddressChange was used -> the card started sounding strangely ->
--it looks the time wasn't long enough for adress to be updated.
	TE_fullAddress_16TO2 <= TE_upperAddress & SCR_channel_status & SCR_counter_status( 10 downto 0 );
	LB_ADDRESS_16TO2 <= TE_fullAddress_16TO2 when ( nTE_enableTransfer = '0' ) else ( LB_ADDRESS_16TO2'range => 'Z' );

-------------------------------------------------------------------------------
-- Tranfer request protector
--
-- As regSelector is used for transfer request we need to
-- create a trigger out of it -> but first we have to protect
-- transfer engine agains gliches.
-- (Because event pulse generator containts a DFF at the input
-- we'll remove the 'Status' DFF from protection).
	process ( LB_CLK, TE_TRANSFER_REQUEST )
	begin
		if rising_edge( LB_CLK ) then
			TE_transferRequest_protector <= TE_TRANSFER_REQUEST;
		end if;
	end process;
	EPG_LEFT: neg_event_pulse_generator port map
	(
		LB_CLK,
		TE_transferRequest_protector,
		TE_startTransfer
	);
	
-------------------------------------------------------------------------------
-- This engine will transfer data from register to local memory or back.
--
-- LR          --|__________|----------|__________|----------|__________|----------|__  ( leftRight clock )
--
-- nB3/2       ---|_____|----|_____|----|_____|----|_____|----|_____|----|_____|----|_  ( nBick24_3Halfs )
--
-- T_TFS       _________|-|________|-|________|-|________|-|________|-|________|-|____  ( LRS_transferFromToSlave )
--
-- serialPort:     IN L      IN R       IN L       IN R       IN L       IN R           ( data currently shifted to master )
-- serialPort:     OUT L     OUT R      OUT L      OUT L      OUT L      OUT R          ( data currently shifted from master )
--
-- dataInSlave:         IN L       IN R       IN L       IN R       IN L       IN R     ( master contains -> slave will get )
-- dataInSlave:         OUT R      OUT L      OUT R      OUT L      OUT R      OUT L    ( slave contains -> master will get )
--
-- T_IL_OR     _________|-|___________________|-|___________________|-|_______________  ( transfer left input and right output )
--
-- DT_L        ____________xxxxxxxx______________xxxxxxxx______________xxxxxxxx_______  ( transfer left input from slave and set left output to slave )
--
-- T_IR_OL     ____________________|-|___________________|-|___________________|-|____  ( transfer right input and left output )
--
-- DT_R        xxxxxxxxx______________xxxxxxxx______________xxxxxxxx______________xxxx  ( transfer right input from slave and set right output to slave )
--
--
-- Data transfer can happen at any x position. DT is transfer of data toAndFrom slave registers.
-- So this process enables us that transferEngine has 1/2 sample of time to transfer data toAndFrom memory.
-- Now if Altera gets local bus at the end of this 1/2 sample period it can happen that some register
-- or sample will get corrupted, so this must NOT happen.
--
--
-- Local bus address change:
-- as we see from the time diagram the address should be allowed to change at every time
-- except in the DT_R part where the address can really change (the address changes on
-- falling edge of LR ).
-- Now just to be programatically correct we'll allow the address change only at
-- TE_waitForTransferRequest_left, so as soon the right channel data will be read/saved
-- the adrress will be allowed to be read again.
--
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
		LB_HOLDA,
		DC_nFINISHED_TRANSFER
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
				TE_LB_HOLD <= '0';
				nMEMORY_CE <= '1';
				nMEMORY_OE <= '1';
				nMEMORY_WE <= '1';
				TE_allowAddressChange <= '1';
				
				-- channel selector
				-- This variable defines which channel should receive/transmit
				-- it's data.
				nTE_enableTransfer <= '1';
				nTE_READ_WRITE_ENABLE <= '1';
				TE_upperAddress <= "000";
				TE_CHANNEL_SELECT <= "000";

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';
				
		-----------------------------------------------------------------------------
		-- Request local bus -> wait until LB_HOLDA goes high.
			when TE_requestLocalBus =>
			-- change state?
				if ( LB_HOLDA = '1' ) then
				-- yes, the bus access was granted -> go to next state
					TE_nextState <= TE_write_ch12;
				else
				-- no, don't change the state
					TE_nextState <= TE_requestLocalBus;
				end if;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1'; -- request local bus
				nMEMORY_CE <= '1';
				nMEMORY_OE <= '1';
				nMEMORY_WE <= '1';
				TE_allowAddressChange <= '0';
				
				-- channel selector
				nTE_enableTransfer <= '1';
				nTE_READ_WRITE_ENABLE <= '1';
				TE_upperAddress <= "000";
				TE_CHANNEL_SELECT <= "000";

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';

		-----------------------------------------------------------------------------
		-- WRITE CHANNELS.
		-----------------------------------------------------------------------------
		-- Write channel 1/2 sample to memory.
			when TE_write_ch12 =>
			-- change state
				TE_nextState <= TE_write_ch12_w;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1'; -- hold request of the local bus
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '1'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';

				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_upperAddress <= "000"; -- input1/2 -> 0x0000/0x0800 (in samples)
				TE_CHANNEL_SELECT <= "000";

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';
				
			when TE_write_ch12_w =>
			-- change state
				TE_nextState <= TE_write_ch34;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1'; -- hold request of the local bus
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '1'; -- output enable
				nMEMORY_WE <= '0'; -- write to memory
				TE_allowAddressChange <= '0';

				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_upperAddress <= "000"; -- input1/2 => 0x0000/0x0800 (in samples)
				TE_CHANNEL_SELECT <= "000";

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';

		-----------------------------------------------------------------------------				
		-- Write channel 3/4 sample to memory.
			when TE_write_ch34 =>
			-- change state?
				TE_nextState <= TE_write_ch34_w;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1'; -- hold request of the local bus
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '1'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';
				
				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_upperAddress <= "001"; -- input3/4 => 0x1000/0x1800 (in samples)
				TE_CHANNEL_SELECT <= "001";

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';
				
			when TE_write_ch34_w =>
			-- change state?
				TE_nextState <= TE_write_ch56;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1'; -- hold request of the local bus
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '1'; -- output enable
				nMEMORY_WE <= '0'; -- write to memory
				TE_allowAddressChange <= '0';
				
				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_upperAddress <= "001"; -- input3/4 => 0x1000/0x1800 (in samples)
				TE_CHANNEL_SELECT <= "001";

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';
				
		-----------------------------------------------------------------------------
		-- Write channel 5/6 sample to memory.
			when TE_write_ch56 =>
			-- change state
				TE_nextState <= TE_write_ch56_w;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1'; -- hold request of the local bus
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '1'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';

				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_upperAddress <= "010"; -- input5/6 => 0x2000/0x2800 (in samples)
				TE_CHANNEL_SELECT <= "010";

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';
				
			when TE_write_ch56_w =>
			-- change state
				TE_nextState <= TE_write_ch78;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1'; -- hold request of the local bus
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '1'; -- output enable
				nMEMORY_WE <= '0'; -- write to memory
				TE_allowAddressChange <= '0';

				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_upperAddress <= "010"; -- input5/6 => 0x2000/0x2800 (in samples)
				TE_CHANNEL_SELECT <= "010";

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';
				
		-----------------------------------------------------------------------------				
		-- Write channel 7/8 sample to memory.
			when TE_write_ch78 =>
			-- change state?
				TE_nextState <= TE_write_ch78_w;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1'; -- hold request of the local bus
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '1'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';
				
				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_upperAddress <= "011"; -- input7/8 => 0x3000/0x3800 (in samples)
				TE_CHANNEL_SELECT <= "011";

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';
				
			when TE_write_ch78_w =>
			-- change state?
				TE_nextState <= TE_read_ch12;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1'; -- hold request of the local bus
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '1'; -- output enable
				nMEMORY_WE <= '0'; -- write to memory
				TE_allowAddressChange <= '0';
				
				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_upperAddress <= "011"; -- input7/8 => 0x3000/0x3800 (in samples)
				TE_CHANNEL_SELECT <= "011";

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';

		-----------------------------------------------------------------------------
		-- READ CHANNELS.
		-----------------------------------------------------------------------------				
		-- Read channel 1/2 sample from memory.
			when TE_read_ch12 =>
			-- change state
				TE_nextState <= TE_read_ch12_r;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1'; -- hold request of the local bus
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '0'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';

				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_upperAddress <= "100"; -- output1/2 => 0x4000/0x4800 (in samples)
				TE_CHANNEL_SELECT <= "100";

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';

			when TE_read_ch12_r =>
			-- change state
				TE_nextState <= TE_read_ch34;
				
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1'; -- hold request of the local bus
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '0'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';

				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_upperAddress <= "100"; -- output1/2 => 0x4000/0x4800 (in samples)
				TE_CHANNEL_SELECT <= "100";

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';
				
		-----------------------------------------------------------------------------				
		-- Read channel 3/4 sample from memory.
			when TE_read_ch34 =>
			-- change state
				TE_nextState <= TE_read_ch34_r;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1'; -- hold request of the local bus
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '0'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';

				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_upperAddress <= "101"; -- output3/4 => 0x5000/0x5800 (in samples)
				TE_CHANNEL_SELECT <= "101";

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';

			when TE_read_ch34_r =>
			-- change state
				TE_nextState <= TE_read_ch56;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1'; -- hold request of the local bus
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '0'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';

				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_upperAddress <= "101"; -- output3/4 => 0x5000/0x5800 (in samples)
				TE_CHANNEL_SELECT <= "101";
				
				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';
				
		-----------------------------------------------------------------------------				
		-- Read channel 5/6 sample from memory.
			when TE_read_ch56 =>
			-- change state
				TE_nextState <= TE_read_ch56_r;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1'; -- hold request of the local bus
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '0'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';

				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_upperAddress <= "110"; -- output5/6 => 0x6000/0x6800 (in samples)
				TE_CHANNEL_SELECT <= "110";

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';
				
			when TE_read_ch56_r =>
			-- change state
				TE_nextState <= TE_read_ch78;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1'; -- hold request of the local bus
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '0'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';

				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_upperAddress <= "110"; -- output5/6 => 0x6000/0x6800 (in samples)
				TE_CHANNEL_SELECT <= "110";
				
				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';
				
		-----------------------------------------------------------------------------				
		-- Read channel 7/8 sample from memory.
			when TE_read_ch78 =>
			-- change state
				TE_nextState <= TE_read_ch78_r;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1'; -- hold request of the local bus
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '0'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';

				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_upperAddress <= "111"; -- output7/8 => 0x7000/0x7800 (in samples)
				TE_CHANNEL_SELECT <= "111";

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';

			when TE_read_ch78_r =>
			-- change state
				TE_nextState <= TE_DaughterCard_transferRequest;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1'; -- hold request of the local bus
				nMEMORY_CE <= '0'; -- enable memory
				nMEMORY_OE <= '0'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';

				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_upperAddress <= "111"; -- output7/8 => 0x7000/0x7800 (in samples)
				TE_CHANNEL_SELECT <= "111";

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';

		-----------------------------------------------------------------------------				
		-- Daughter Card data transfer.
		--
		-- The daughter card will do everything on it's own but we have to define
		-- at which time it can do it.
		--
		-- DC_nSTART_TRANSFER - signals daughterCard that it's time to do the honky tonk
		-- nDC_FINISHED_TRANSFER - daughterCard signals to main Altera chip that it had
		--                         finished with it's work
		--                         Notice:
		--                         This line has a pull-down resistor, so the line will be
		--                         at logic 0 if there is no daughterCard present.
		--                         In other case the daughterCard will hold this line at
		--                         logic 1 for most of the time.
		--
--			TE_DaughterCard_transferRequest
			when others =>
			-- change state
				if ( DC_nFINISHED_TRANSFER = '0' ) then
				-- yes, go to next state
					TE_nextState <= TE_waitForTransferRequest;
				else
				-- no, don't change the state
					TE_nextState <= TE_DaughterCard_transferRequest;
				end if;
			
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1'; -- hold request of the local bus
				nMEMORY_CE <= '1'; -- enable memory
				nMEMORY_OE <= '1'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';
				
				-- channel selector
				-- This variable defines which channel should receive/transmit
				-- it's data.
				nTE_enableTransfer <= '1';
				nTE_READ_WRITE_ENABLE <= '1';
				TE_upperAddress <= "111";
				TE_CHANNEL_SELECT <= "111";
				
				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '0';

		end case;
	end process;

-- Connectoutput pins.
	LB_HOLD <= TE_LB_HOLD;

end opis;

-------------------------------------------------------------------------------
--  SAMPLE_RATE_COUNTER  ------------------------------------------------------
-------------------------------------------------------------------------------
LIBRARY IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;

entity sample_counter is
	port
	(
		CLK_SAMPLE_RATE: in std_logic;
		nRESET_ADDRESS: in std_logic; -- asynchronous reset
		COUNTER: out std_logic_vector( 15 downto 0 )
	);
end sample_counter;

architecture opis of sample_counter is
	signal counter_L: std_logic_vector( 15 downto 0 );
	
begin
	process ( CLK_SAMPLE_RATE, counter_L, nRESET_ADDRESS )
	begin
		if ( nRESET_ADDRESS = '0' ) then
			counter_L <= ( counter_L'range => '0' );
		elsif falling_edge( CLK_SAMPLE_RATE ) then
			counter_L <= counter_L + x"0001";
		end if;
	end process;

-- Connect output pins.
	COUNTER <= counter_L;
	
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