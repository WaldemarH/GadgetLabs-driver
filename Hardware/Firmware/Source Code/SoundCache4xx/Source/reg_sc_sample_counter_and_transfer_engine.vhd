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

--This variable is a common 16bit wide up counter.
--Now this value can be used for a channel address(this is how it's done in
--hardware.
--So the counter is reset when SC_RELEASE_COUNTERS (SC_AUDIO_ENGINE) goes low.
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
	
		-- Next one will signal in which state the transfer engine is.
		LB_USER0: out std_logic; -- 138
	
	-- Pins dedicated to register.
		-- address counter
		nRESET_ENGINE: in std_logic;
		CLK_SAMPLE_RATE: in std_logic;
		
		-- memory controller
		LB_ADDRESS: out std_logic_vector ( 16 downto 0 ); -- full address ( channel + sample select )
		LB_HOLD: out std_logic; -- 45
		LB_HOLDA: in std_logic; -- 46
		
		nMEMORY_OE: out std_logic;
		nMEMORY_WE: out std_logic;

		-- transfer engine
		TE_CHANNEL_SELECT: out std_logic_vector( 2 downto 0 ); -- 8 channels (4 in, 4 out)
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
	
-------------------------------------------------------------------------------
--  TRANSFER ENGINE  ----------------------------------------------------------
-------------------------------------------------------------------------------
	signal TE_allowAddressChange: std_logic;
	signal TE_channelSelect: std_logic_vector( 2 downto 0 );
	signal TE_fullAddress: std_logic_vector( 16 downto 0 );
	signal nTE_enableTransfer: std_logic;
	signal TE_LB_HOLD: std_logic;

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
		TE_read_ch4_r,
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
	process ( LB_CLK, SCR_counter_protector )
	begin
		if rising_edge( LB_CLK ) then
			if ( TE_allowAddressChange = '1' ) then
				SCR_counter_status <= SCR_counter_protector;
			end if;
		end if;
	end process;

-------------------------------------------------------------------------------
--  TRANSFER ENGINE  ----------------------------------------------------------
-------------------------------------------------------------------------------
-- memory address:
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
	TE_fullAddress <= TE_channelSelect & SCR_counter_status( 11 downto 0 ) & "00";
	LB_ADDRESS <= TE_fullAddress when ( nTE_enableTransfer = '0' ) else ( LB_ADDRESS'range => 'Z' );

-------------------------------------------------------------------------------
-- Tranfer request protector
--
-- As regSelector is used for transfer request we need to create a trigger out of it ->
-- but first we have to protect transfer engine agains gliches.
-- (Because event pulse generator containts a DFF at the input
-- we'll remove the 'Status' DFF from protection).
--
-- TE_TRANSFER_REQUEST transfer pulse:
-- Longest  pulse (for fs = 44100Hz) = 236.21ns
-- Shortest pulse (for fs = 96000Hz) = 108.51ns
-- PCI clock pulse                   =  30.30ns
--
-- Notice:
-- As we can see the TE_startTransfer must happen on falling edge of the TE_TRANSFER_REQUEST
-- or we'll run into trouble.
--
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
				nMEMORY_OE <= '1';
				nMEMORY_WE <= '1';
				TE_allowAddressChange <= '1';
				
				-- channel selector
				-- This variable defines which channel should receive/transmit
				-- it's data.
				nTE_enableTransfer <= '1';
				nTE_READ_WRITE_ENABLE <= '1';
				TE_channelSelect <= "000";
				
				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';
				
				--Signal the state we are in.
				LB_USER0 <= '1';
				
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
				TE_LB_HOLD <= '1';	-- request local bus
				nMEMORY_OE <= '1';
				nMEMORY_WE <= '1';
				TE_allowAddressChange <= '0';
				
				-- channel selector
				nTE_enableTransfer <= '1';
				nTE_READ_WRITE_ENABLE <= '1';
				TE_channelSelect <= "000";

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';

				--Signal the state we are in.
				LB_USER0 <= '0';

		-----------------------------------------------------------------------------
		-- Write channel 1 sample to memory.
			when TE_write_ch1 =>
			-- change state
				TE_nextState <= TE_write_ch1_w;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1';	-- hold request of the local bus
				nMEMORY_OE <= '1'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';

				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_channelSelect <= "000"; -- input1left => 0x0000 (in samples)

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';
				
				--Signal the state we are in.
				LB_USER0 <= '0';
				
			when TE_write_ch1_w =>
			-- change state
				TE_nextState <= TE_write_ch2;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1';	-- hold request of the local bus
				nMEMORY_OE <= '1'; -- output enable
				nMEMORY_WE <= '0'; -- write to memory
				TE_allowAddressChange <= '0';

				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_channelSelect <= "000"; -- input1left => 0x0000

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';

				--Signal the state we are in.
				LB_USER0 <= '0';

		-----------------------------------------------------------------------------				
		-- Write channel 2 sample to memory.
			when TE_write_ch2 =>
			-- change state
				TE_nextState <= TE_write_ch2_w;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1';	-- hold request of the local bus
				nMEMORY_OE <= '1'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';
				
				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_channelSelect <= "001"; -- input1right => 0x1000 (in samples)

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';
				
				--Signal the state we are in.
				LB_USER0 <= '0';
				
			when TE_write_ch2_w =>
			-- change state
				TE_nextState <= TE_write_ch3;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1';	-- hold request of the local bus
				nMEMORY_OE <= '1'; -- output enable
				nMEMORY_WE <= '0'; -- write to memory
				TE_allowAddressChange <= '0';
				
				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_channelSelect <= "001"; -- input1right => 0x1000

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';
				
				--Signal the state we are in.
				LB_USER0 <= '0';
				
		-----------------------------------------------------------------------------				
		-- Write channel 3 sample to memory.
			when TE_write_ch3 =>
			-- change state?
				TE_nextState <= TE_write_ch3_w;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1';	-- hold request of the local bus
				nMEMORY_OE <= '1'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';
				
				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_channelSelect <= "010"; -- input2left => 0x2000 (in samples)
				
				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';
				
				--Signal the state we are in.
				LB_USER0 <= '0';
				
			when TE_write_ch3_w =>
			-- change state?
				TE_nextState <= TE_write_ch4;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1';	-- hold request of the local bus
				nMEMORY_OE <= '1'; -- output enable
				nMEMORY_WE <= '0'; -- write to memory
				TE_allowAddressChange <= '0';
				
				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_channelSelect <= "010"; -- input2left => 0x2000

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';
				
				--Signal the state we are in.
				LB_USER0 <= '0';
				
		-----------------------------------------------------------------------------				
		-- Write channel 4 sample to memory.
			when TE_write_ch4 =>
			-- change state
				TE_nextState <= TE_write_ch4_w;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1';	-- hold request of the local bus
				nMEMORY_OE <= '1'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';
				
				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_channelSelect <= "011"; -- input2right => 0x3000 (in samples)
				
				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';

				--Signal the state we are in.
				LB_USER0 <= '0';

			when TE_write_ch4_w =>
			-- change state
				TE_nextState <= TE_read_ch1;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1';	-- hold request of the local bus
				nMEMORY_OE <= '1'; -- output enable
				nMEMORY_WE <= '0'; -- write to memory
				TE_allowAddressChange <= '0';
				
				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_channelSelect <= "011"; -- input2right => 0x3000
				
				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';

				--Signal the state we are in.
				LB_USER0 <= '0';

		-----------------------------------------------------------------------------				
		-- Read channel 1 sample from memory.
			when TE_read_ch1 =>
			-- change state
				TE_nextState <= TE_read_ch1_r;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1';	-- hold request of the local bus
				nMEMORY_OE <= '0'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';

				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_channelSelect <= "100"; -- output1left => 0x4000 (in samples)
				
				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';

				--Signal the state we are in.
				LB_USER0 <= '0';

			when TE_read_ch1_r =>
			-- change state
				TE_nextState <= TE_read_ch2;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1';	-- hold request of the local bus
				nMEMORY_OE <= '0'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';

				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_channelSelect <= "100"; -- output1left => 0x4000

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';

				--Signal the state we are in.
				LB_USER0 <= '0';

		-----------------------------------------------------------------------------				
		-- Read channel 2 sample from memory.
			when TE_read_ch2 =>
			-- change state
				TE_nextState <= TE_read_ch2_r;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1';	-- hold request of the local bus
				nMEMORY_OE <= '0'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';

				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_channelSelect <= "101"; -- output1right => 0x5000 (in samples)

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';
				
				--Signal the state we are in.
				LB_USER0 <= '0';
				
			when TE_read_ch2_r =>
			-- change state
				TE_nextState <= TE_read_ch3;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1';	-- hold request of the local bus
				nMEMORY_OE <= '0'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';

				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_channelSelect <= "101"; -- output1right => 0x5000

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';

				--Signal the state we are in.
				LB_USER0 <= '0';

		-----------------------------------------------------------------------------				
		-- Read channel 3 sample from memory.
			when TE_read_ch3 =>
			-- change state
				TE_nextState <= TE_read_ch3_r;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1';	-- hold request of the local bus
				nMEMORY_OE <= '0'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';

				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_channelSelect <= "110"; -- output2left => 0x6000 (in samples)

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';

				--Signal the state we are in.
				LB_USER0 <= '0';

			when TE_read_ch3_r =>
			-- change state
				TE_nextState <= TE_read_ch4;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1';	-- hold request of the local bus
				nMEMORY_OE <= '0'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';

				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_channelSelect <= "110"; -- output2left => 0x6000

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';

				--Signal the state we are in.
				LB_USER0 <= '0';

		-----------------------------------------------------------------------------				
		-- Read channel 4 sample from memory.
			when TE_read_ch4 =>
			-- change state
				TE_nextState <= TE_read_ch4_r;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1';	-- hold request of the local bus
				nMEMORY_OE <= '0'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';

				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_channelSelect <= "111"; -- output2right => 0x7000 (in samples)

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';
				
				--Signal the state we are in.
				LB_USER0 <= '0';
				
			when TE_read_ch4_r =>
			-- change state
				TE_nextState <= TE_DaughterCard_transferRequest;
					
			-- outputs
				-- memory control
				TE_LB_HOLD <= '1';	-- hold request of the local bus
				nMEMORY_OE <= '0'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';

				-- channel selector
				nTE_enableTransfer <= '0';
				nTE_READ_WRITE_ENABLE <= '0';
				TE_channelSelect <= "111"; -- output2right => 0x7000

				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '1';

				--Signal the state we are in.
				LB_USER0 <= '0';

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
				nMEMORY_OE <= '1'; -- output enable
				nMEMORY_WE <= '1'; -- write to memory
				TE_allowAddressChange <= '0';
				
				-- channel selector
				-- This variable defines which channel should receive/transmit
				-- it's data.
				nTE_enableTransfer <= '1';
				nTE_READ_WRITE_ENABLE <= '1';
				TE_channelSelect <= "111"; -- output2right => 0x7000
				
				--DaughterCard transfer
				DC_nSTART_TRANSFER <= '0';

				--Signal the state we are in.
				LB_USER0 <= '0';

		end case;
	end process;

-- Connectoutput pins.
	TE_CHANNEL_SELECT <= TE_channelSelect;
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
		CLK_SAMPLE_RATE: in std_logic;	-- rising edge clock
		nRESET_ADDRESS: in std_logic;   -- asynchronous reset
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
	
-- Pulse generator
	-- input from 1 to 0
	OUTPUT_PULSE <= ( not TRIGGER ) and DFF_2;

end opis;