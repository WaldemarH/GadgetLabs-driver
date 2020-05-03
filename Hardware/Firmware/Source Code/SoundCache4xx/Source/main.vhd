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
use IEEE.std_logic_signed.all;

entity main is
	port
	(
	-- Local BUS
		LB_ADDRESS: inout std_logic_vector( 16 downto 0 ) bus;
		LB_DATA: inout std_logic_vector( 31 downto 0 ) bus;
		
		LB_CLK: in std_logic; -- 20
		nLB_RD: in std_logic; -- 48
		nLB_WR: in std_logic; -- 49
		
		nLB_RESET: in std_logic; -- 140
	
	-- Chip enables
		nCS_FPGA: in std_logic; -- 42
		nCS_LM: in std_logic; -- 44
		
	-- Memory control
		nBANK_CE: out std_logic; -- 108
		nBANK_OE: out std_logic; -- 106
		nBANK_WE: out std_logic; -- 107
		
	-- Local bus control.
		LB_HOLD: out std_logic; -- 45
		LB_HOLDA: in std_logic; -- 46
		LB_USER0: out std_logic; -- 138
		LB_USER1: out std_logic; -- 139 (permanently tied to 0)
		
	-- PLL/master clock
		CLOCK_DAUGHTER_CARD: in std_logic; -- 68
		CLOCK_MASTER_OUT: out std_logic; --67
		CLOCK_PLL_A: in std_logic; -- 71
		CLOCK_SYNC_IN: in std_logic; -- 70
		CLOCK_UART: IN STD_LOGIC; -- 41
		CRYSTAL_OSCILATOR_IN: in std_logic; -- 75
		
		PLL_INTERFACE_CLK: out std_logic; -- 110
		PLL_INTERFACE_DATA: inout std_logic; -- 111
		PLL_REFERENCE_CLOCK: out std_logic; -- 74
		
	-- Daughter card pins.
		DC_DATA_BIT: out std_logic; -- 116
		DC_DATA_CLK: out std_logic; -- 115
		DC_CONF_DONE: in std_logic; -- 114
		DC_nCONFIG: out std_logic; -- 112
		DC_nSTATUS: in std_logic; -- 113
		nDC_PRSNT: in std_logic; -- 51 (it has pull-up resistor)
		
		DC_nFINISHED_TRANSFER: in std_logic; -- 52 (it has pull-down resistor)
		DC_nSTART_TRANSFER: out std_logic; -- 50

	-- CODEC pins.
		CODEC_BICK: out std_logic; -- 84
		CODEC_CS1: out std_logic; -- 99
		CODEC_CS2: out std_logic; -- 98
		CODEC_CONTROL_CLK: out std_logic; -- 96
		CODEC_CONTROL_DATA: out std_logic; -- 95	 
		CODEC_CONTROL_IF: out std_logic; -- 97
		CODEC_LEFT_RIGHT: out std_logic; -- 83
		CODEC_MASTER_SLAVE: out std_logic; -- 101
		CODEC1_SERIAL_DATA_IN: in std_logic; -- 88
		CODEC2_SERIAL_DATA_IN: in std_logic; -- 87
		CODEC1_SERIAL_DATA_OUT: out std_logic; -- 94
		CODEC2_SERIAL_DATA_OUT: out std_logic; -- 93
		CODEC_POWER_UP: out std_logic; -- 100

	-- Interrupt pins.
		INT_MIDI: in std_logic; -- 58
		nINT_DAUGHTER_CARD: in std_logic; -- 57 (it has pull-up resistor)
		nINT_OUTPUT_REQUEST: out std_logic; -- 43 openDrain pin
	
	-- Gain/Monitoring pins.
		GAIN_OUT: out std_logic; -- 59 (only for 496 cards)
		GAIN_IN_A: out std_logic; -- 66
		GAIN_IN_B: out std_logic; --65
		MONITOR_1L: out std_logic; -- 64
		MONITOR_1R: out std_logic; -- 63
		MONITOR_2L: out std_logic; -- 62
		MONITOR_2R: out std_logic; -- 61
		POWER_ON_MUTE: out std_logic -- 60 (permanently tied to 0)
	);
	
end main;

architecture opis of main is
-------------------------------------------------------------------------------
-- COMPONENTS -----------------------------------------------------------------
-------------------------------------------------------------------------------
-- FPGA registers access ------------------------------------------------------
	component lineDecoder_3to8
		port
		(
			address: in std_logic_vector ( 2 downto 0 );
			nEnable_read: in std_logic;
			nEnable_write: in std_logic;
			
			nOutputEnable_read: out std_logic_vector ( 4 downto 0 );
			nOutputEnable_write: out std_logic_vector ( 4 downto 0 )
		);
	end component;
	
-- registers ------------------------------------------------------------------
	-- 0x00
	component reg_SC_DEVICES
		port
		(
		-- Signals from main.
			LB_CLK: in std_logic;
			nLB_ENABLE_READ: in std_logic;
			nLB_ENABLE_WRITE: in std_logic;
			nLB_RESET: in std_logic;
			LB_DATA: inout std_logic_vector ( 15 downto 0 );
		
		-- Pins dedicated to register.
			-- Daughter card.
			nDC_PRSNT: in std_logic; -- 51
			DC_CONF_DONE: in std_logic; -- 114
			DC_DATA_BIT: out std_logic; -- 116
			DC_DATA_CLK: out std_logic; -- 115
			DC_nCONFIG: out std_logic; -- 112
			DC_nSTATUS: in std_logic; -- 113
			
			-- PLL.
			PLL_INTERFACE_DATA: inout std_logic; --111
			PLL_INTERFACE_CLK: out std_logic; -- 110
			
			-- CODECs
			CODEC_POWER_UP: out std_logic; --100
			CODEC_CS1: out std_logic; --99
			CODEC_CS2: out std_logic; --98
			CODEC_CONTROL_IF: out std_logic; --97
			CODEC_CONTROL_CLK: out std_logic; --96
			CODEC_CONTROL_DATA: out std_logic --95
		);
	end component;
	-- 0x02
	component reg_SC_ENGINE
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
	end component;
	-- 0x04
	component reg_SC_SAMPLE_COUNTER_and_TRANSFER_ENGINE
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
			TE_CHANNEL_SELECT: out std_logic_vector( 2 downto 0 );
			nTE_READ_WRITE_ENABLE: out std_logic;
			nTE_STOP_REQUEST: in std_logic;
			TE_TRANSFER_REQUEST: in std_logic
		);
	end component;
	-- 0x08
	component reg_SC_INTERRUPT_DOWNCOUNTER
		port
		(
		-- Signals from main.
			LB_CLK: in std_logic;
			nLB_ENABLE_WRITE: in std_logic;
			LB_DATA: in std_logic_vector ( 10 downto 0 );
		
		-- Pins dedicated to engine.
			CLOCK_SAMPLE_RATE: in std_logic;
			ENGINE_INT_REQUEST: out std_logic
		);
	end component;
	-- 0x0A
	component reg_SC_INTERRUPT
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
	end component;
	-- 0x0C
	component reg_SC4_GAIN_MON
		port
		(
		-- Signals from main.
			LB_CLK: in std_logic;
			nLB_ENABLE_READ: in std_logic;
			nLB_ENABLE_WRITE: in std_logic;
			LB_DATA: inout std_logic_vector ( 15 downto 0 );
		
		-- Pins dedicated to register.
			-- Gain, monitoring.	
			GAIN_OUT: out std_logic; -- 59
			GAIN_IN_A: out std_logic; -- 66
			GAIN_IN_B: out std_logic; -- 65
			MONITOR_1L: out std_logic; -- 64
			MONITOR_1R: out std_logic; -- 63
			MONITOR_2L: out std_logic; -- 62
			MONITOR_2R: out std_logic -- 61
		);
	end component;
	
-- engines --------------------------------------------------------------------
	component clocks_engine
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
			CLOCK_MASTER_OUT: out std_logic; -- 67 (256 x fs)
			CLOCK_SAMPLE_RATE_OUT: out std_logic
		);
	end component;
	
	component CODEC_engine
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
	end component;
	
-------------------------------------------------------------------------------
-- SIGNALS  -------------------------------------------------------------------
-------------------------------------------------------------------------------
	-- Memory access.
	signal nMemory_readEnable: std_logic;
	signal nMemory_writeEnable: std_logic;

	signal nBank_OE_s: std_logic;
	signal nBank_WE_s: std_logic;
	
	signal upperByte: std_logic_vector( 7 downto 0 );
	
	-- FPGA registers access.
	signal nFPGA_readEnable: std_logic;
	signal nFPGA_writeEnable: std_logic;

	signal nRegisterEnable_read: std_logic_vector ( 4 downto 0 );
	signal nRegisterEnable_write: std_logic_vector ( 4 downto 0 );

	-- SC_ENGINE
	signal clockSelection: std_logic_vector( 1 downto 0 );
	signal nResetEngines: std_logic;
	signal nStopEngines: std_logic;

	-- SC_ADDRESS_and_TRANSFER_ENGINE
	signal nTE_OE: std_logic;
	signal nTE_WE: std_logic;

	signal TE_channelSelect: std_logic_vector( 2 downto 0 );
	signal nTE_readWriteEnable: std_logic;
	
	-- CODECs engine
	signal CDC_transferRequest: std_logic;
	
	-- Interrupt downcounter.
	signal intAudioEngine: std_logic;
	
	-- Clocks engine
	signal nBick_24: std_logic;
	signal clock_bick: std_logic;
	signal clock_sampleRate: std_logic;
	
begin
-------------------------------------------------------------------------------
-- Permanently tied to GND or VCC. --------------------------------------------
-------------------------------------------------------------------------------
-- Notice:
-- PLL power down pin has external pullUp -> power unused parts
--	
	CODEC_MASTER_SLAVE <= '0'; -- CODECs are always in slave mode
	--LB_USER0 <= '0'; -- that the line wont fluctuate(it's not connected anywhere)
	LB_USER1 <= '0'; -- that the line wont fluctuate(it's not connected anywhere)
	POWER_ON_MUTE <= '0'; -- powerOnMute is causing clicks so it's turned off
	
-------------------------------------------------------------------------------
-- Reference clock for PLL. ---------------------------------------------------
-------------------------------------------------------------------------------
	PLL_REFERENCE_CLOCK <= CRYSTAL_OSCILATOR_IN;
	
-------------------------------------------------------------------------------
-- Memory access. -------------------------------------------------------------
-------------------------------------------------------------------------------
-- Logic for memory access(the code needs to create signals to enable memory)
--
-- nBANK_CE: out std_logic; -- 108
-- nBANK_OE: out std_logic; -- 106
-- nBANK_WE: out std_logic; -- 107
--
-- nCS_LM: in std_logic; -- 44
-- nLB_RD: in std_logic; -- 48
-- nLB_WR: in std_logic; -- 49
--
-- Because every group(pins) are far from each other we should connect
-- as few lines as we can get cross the chip. So we will transport
-- only 2 lines nMemory_readEnable and nMemory_writeEnable.
--
-- Notice:
-- On memory read(from the system) we'll also set the upper 8bits.
-- So that all 32bits will be correct.
-- (If bit 23 is 1 then all upper 8bits should be 1 too, else 0.)
--
	nMemory_readEnable <= nCS_LM or nLB_RD; -- only when both are low
	nMemory_writeEnable <= nCS_LM or nLB_WR;

	nBank_OE_s <= nMemory_readEnable and nTE_OE; -- if at least one is low
	nBank_WE_s <= nMemory_writeEnable and nTE_WE;
	
	nBANK_OE <= nBank_OE_s;
	nBANK_WE <= nBank_WE_s;
	nBANK_CE <= '0';	-- memory is always on... needed for 1 PCI cycle R/W operation

	-- Set upper 8bits.
	upperByte <= x"00" when ( LB_DATA( 23 ) = '0' ) else x"FF";
	LB_DATA( 31 downto 24 ) <= upperByte when ( nMemory_readEnable = '0' ) else "ZZZZZZZZ";

-------------------------------------------------------------------------------
-- FPGA registers access. -----------------------------------------------------
-------------------------------------------------------------------------------
-- Local Address: 0xC0 = 1100 0000b
-- FPGA addresses: from 0xC0 to 0xD0-1 ( 1100 0000b to 1101 0000b )
-- DaughterCard addresses: from 0xD0 to 0xE0-1
--
-- Altera should not enable registers if bit 5 is set
-- (reserved for daughter card).
--
	nFPGA_readEnable <= nCS_FPGA or LB_ADDRESS( 4 ) or nLB_RD; -- only when all are low
	nFPGA_writeEnable <= nCS_FPGA or LB_ADDRESS( 4 ) or nLB_WR;

	LD: lineDecoder_3to8 port map
	(
		LB_ADDRESS( 3 downto 1 ),
		nFPGA_readEnable,
		nFPGA_writeEnable,
		nRegisterEnable_read,
		nRegisterEnable_write
	);

-------------------------------------------------------------------------------
-- Registers  -----------------------------------------------------------------
-------------------------------------------------------------------------------
-- registers
	-- 0x00
	SC_DEVICES: reg_SC_DEVICES port map
	(
	-- Signals from main.
		LB_CLK,
		nRegisterEnable_read( 0 ),
		nRegisterEnable_write( 0 ),
		nLB_RESET,
		LB_DATA( 15 downto 0 ),
	
	-- Pins dedicated to register.
		-- Daughter card.
		nDC_PRSNT,
		DC_CONF_DONE,
		DC_DATA_BIT,
		DC_DATA_CLK,
		DC_nCONFIG,
		DC_nSTATUS,
		
		-- PLL.
		PLL_INTERFACE_DATA,
		PLL_INTERFACE_CLK,
		
		-- CODECs
		CODEC_POWER_UP,
		CODEC_CS1,
		CODEC_CS2,
		CODEC_CONTROL_IF,
		CODEC_CONTROL_CLK,
		CODEC_CONTROL_DATA
	);
	-- 0x02
	SC_ENGINE: reg_SC_ENGINE port map
	(
	-- Signals from main.
		LB_CLK,
		nRegisterEnable_read( 1 ),
		nRegisterEnable_write( 1 ),
		nLB_RESET,
		LB_DATA( 15 downto 0 ),

	-- Pins dedicated to engine.
		clockSelection,
		
		nResetEngines,
		nStopEngines,
		
		-- Clocks for detection.
		CLOCK_DAUGHTER_CARD,
		CLOCK_SYNC_IN
	);
	-- 0x04
	SC_SAMPLE_COUNTER_and_TRANSFER_ENGINE: reg_SC_SAMPLE_COUNTER_and_TRANSFER_ENGINE port map
	(
	-- Signals from main.
		LB_CLK,
		nRegisterEnable_read( 2 ),
		LB_DATA( 15 downto 0 ),
		
		-- Daughter card.
		DC_nFINISHED_TRANSFER,
		DC_nSTART_TRANSFER,
		
		-- Next one will signal in which state the transfer engine is.
		LB_USER0,

	-- Pins dedicated to register.
		-- address counter
		nResetEngines,
		clock_sampleRate,
		
		-- memory controller
		LB_ADDRESS,
		LB_HOLD,
		LB_HOLDA,
		
		nTE_OE,
		nTE_WE,
		
		-- transfer engine
		TE_channelSelect,
		nTE_readWriteEnable,
		nStopEngines,
		CDC_transferRequest
	);
	-- 0x06
	SC_INTERRUPT_DOWNCOUNTER: reg_SC_INTERRUPT_DOWNCOUNTER port map
	(
	-- Signals from main.
		LB_CLK,
		nRegisterEnable_write( 2 ),
		LB_DATA( 10 downto 0 ),
	
	-- Pins dedicated to engine.
		clock_sampleRate,
		intAudioEngine
	);
	-- 0x08
	SC_INTERRUPT: reg_SC_INTERRUPT port map
	(
	-- Signals from main.
		LB_CLK,
		nRegisterEnable_read( 3 ),
		nRegisterEnable_write( 3 ),
		nLB_RESET,
		LB_DATA( 15 downto 0 ),
		
	-- Pins dedicated to register.
		INT_MIDI,
		nINT_DAUGHTER_CARD,
		intAudioEngine,
		nINT_OUTPUT_REQUEST
	);
	-- 0x0A
	SC4_GAIN_MON: reg_SC4_GAIN_MON port map
	(
	-- Signals from main.
		LB_CLK,
		nRegisterEnable_read( 4 ),
		nRegisterEnable_write( 4 ),
		LB_DATA( 15 downto 0 ),
		
	-- Pins dedicated to register.
		-- Gain, monitoring.	
		GAIN_OUT,
		GAIN_IN_A,
		GAIN_IN_B,
		MONITOR_1L,
		MONITOR_1R,
		MONITOR_2L,
		MONITOR_2R
	);

-- Engines ------------------------------------------------------------------
	CLKE: clocks_engine port map
	(
	--Pins dedicated to engine.
		clockSelection,
		nStopEngines,

		-- input clocks
		CLOCK_DAUGHTER_CARD,
		CLOCK_PLL_A,
		CLOCK_SYNC_IN,
		
		-- output clocks
		clock_bick,
		nBick_24,
		CLOCK_MASTER_OUT,
		clock_sampleRate
	);
	CODEC_BICK <= clock_bick;
	CODEC_LEFT_RIGHT <= clock_sampleRate;

	CE: CODEC_engine port map
	(
	-- Signals from main.
		LB_CLK,
		LB_DATA( 23 downto 0 ),
		
	-- Pins dedicated to register.
		nResetEngines,
		
		-- transfer engine
		TE_channelSelect,
		nTE_readWriteEnable,
		CDC_transferRequest,
		
		-- CODEC data transfer
		clock_bick,
		nBick_24,
		clock_sampleRate,
		
		CODEC1_SERIAL_DATA_IN,
		CODEC2_SERIAL_DATA_IN,
		CODEC1_SERIAL_DATA_OUT,
		CODEC2_SERIAL_DATA_OUT
	);

end opis;

-------------------------------------------------------------------------------
-- 3TO8 LINE DECODER -> SELECTOR ----------------------------------------------
-------------------------------------------------------------------------------
LIBRARY IEEE;
use IEEE.std_logic_1164.all;

entity lineDecoder_3to8 is
	port
	(
		address: in std_logic_vector ( 2 downto 0 );
		nEnable_read: in std_logic;
		nEnable_write: in std_logic;
		
		nOutputEnable_read: out std_logic_vector ( 4 downto 0 );
		nOutputEnable_write: out std_logic_vector ( 4 downto 0 )
	);
end lineDecoder_3to8;

architecture opis of lineDecoder_3to8 is
	signal nAddressEnable: std_logic_vector ( 5 downto 0 );
	
begin
	nAddressEnable( 0 ) <= '0' when ( address = b"000" ) else '1'; -- 0x00
	nAddressEnable( 1 ) <= '0' when ( address = b"001" ) else '1'; -- 0x02
	nAddressEnable( 2 ) <= '0' when ( address = b"010" ) else '1'; -- 0x04
	nAddressEnable( 3 ) <= '0' when ( address = b"011" ) else '1'; -- 0x06
	nAddressEnable( 4 ) <= '0' when ( address = b"100" ) else '1'; -- 0x08
	nAddressEnable( 5 ) <= '0' when ( address = b"101" ) else '1'; -- 0x0A
	
-- Read registers:
	-- 0x00 - SC_DEVICES (read/write)
	nOutputEnable_read( 0 ) <= nEnable_read or nAddressEnable( 0 ); -- 0x00
	
	-- 0x02 - SC_ENGINE (read/write)
	nOutputEnable_read( 1 ) <= nEnable_read or nAddressEnable( 1 ); -- 0x02
	
	-- 0x04 - SC_SAMPLE_COUNTER (read)
	nOutputEnable_read( 2 ) <= nEnable_read or nAddressEnable( 2 ); -- 0x04
	
	-- 0x06 - SC_INTERRUPT_DOWNCOUNTER (write)
	-- no code                                                      -- 0x06
	
	-- 0x08 - SC_INTERRUPT (read/write)
	nOutputEnable_read( 3 ) <= nEnable_read or nAddressEnable( 4 ); -- 0x08
	
	-- 0x0A - SC4_GAIN_MON (read/write)	
	nOutputEnable_read( 4 ) <= nEnable_read or nAddressEnable( 5 ); -- 0x0A
	
	
-- Write registers:
	-- 0x00 - SC_DEVICES (read/write)
	nOutputEnable_write( 0 ) <= nEnable_write or nAddressEnable( 0 ); -- 0x00
	
	-- 0x02 - SC_ENGINE (read/write)
	nOutputEnable_write( 1 ) <= nEnable_write or nAddressEnable( 1 ); -- 0x02
	
	-- 0x04 - SC_SAMPLE_COUNTER (read)
	-- no code                                                        -- 0x04
	
	-- 0x06 - SC_INTERRUPT_DOWNCOUNTER (write)
	nOutputEnable_write( 2 ) <= nEnable_write or nAddressEnable( 3 ); -- 0x06
	
	-- 0x08 - SC_INTERRUPT (read/write)
	nOutputEnable_write( 3 ) <= nEnable_write or nAddressEnable( 4 ); -- 0x08
	
	-- 0x0A - SC4_GAIN_MON (read/write)
	nOutputEnable_write( 4 ) <= nEnable_write or nAddressEnable( 5 ); -- 0x0A

end opis;