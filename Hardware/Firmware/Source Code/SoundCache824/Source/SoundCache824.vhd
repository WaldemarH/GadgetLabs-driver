--***************************************************************************
--***************************************************************************
--
--                          PROPRIETARY INFORMATION
--
-- This software is supplied under the terms of a license agreement or
-- nondisclosure agreement with Waldemar Haszlakiewicz and may not be
-- copied or disclosed except in accordance with the terms of that agreement.
--
--               Copyright (C) 2002 - 2005 Waldemar Haszlakiewicz
--                            All Rights Reserved.
--
--***************************************************************************
--***************************************************************************

LIBRARY IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;

entity SoundCache824 is
	port
	(
	-- Local BUS
		LB_ADDRESS_16TO1: inout std_logic_vector( 15 downto 0 ) bus;
		LB_DATA: inout std_logic_vector( 23 downto 0 ) bus;
		LB_DATA_UPPER: inout std_logic_vector( 7 downto 0 ) bus;
		
		LB_CLK: in std_logic; -- 28
		nLB_RD: in std_logic; -- 68
		nLB_WR: in std_logic; -- 69
		
		nLB_RESET: in std_logic; -- 3
	
	-- Chip enables
		nCS_FPGA: in std_logic; -- 57
		nCS_FPGA_DAUGHTER: out std_logic; -- 119 (direct link from pin 57 to pin 119)
		nCS_LM: in std_logic; -- 59
		
	-- Memory control
		nBANK_CE: out std_logic; -- 156
		nBANK_OE: out std_logic; -- 154
		nBANK_WE: out std_logic; -- 155
		
	-- Local bus control.
		LB_HOLD: out std_logic; -- 60
		LB_HOLDA: in std_logic; -- 61

	-- clocks
		nLB_CLOCK_SELECT0: out std_logic; -- 82 (permanently tied to 1)
		nLB_CLOCK_SELECT1: out std_logic; -- 101 (permanently tied to 0)
		nLB_CLOCK_SELECT2: out std_logic; -- 103 (permanently tied to 1)
		
		CLOCK_16MHz: in std_logic; -- 105
		CLOCK_18MHz: in std_logic; -- 128
		CLOCK_DAUGHTER_CARD: in std_logic; -- 117
		CLOCK_MASTER_OUT: out std_logic; -- 104 (what comes out of 104 comes to pin 24 in)
		CLOCK_SYNC_IN: in std_logic; -- 106
		
	-- Daughter card pins.
		DC_nPRSNT: in std_logic; -- 73
		DC_DATA_BIT: out std_logic; -- 175
		DC_DATA_CLK: out std_logic; -- 174
		DC_CONF_DONE: in std_logic; -- 173
		DC_nCONFIG: out std_logic; -- 171
		DC_nSTATUS: in std_logic; -- 172
		DC_nFINISHED_TRANSFER: in std_logic; -- 74 (it has pull-down resistor)
		DC_nSTART_TRANSFER: out std_logic; -- 81

	-- Converters pins.
		DAC_DIF0: out std_logic; -- 136 (permanently tied to 0)
		
		nRESET_ADC_0: out std_logic; -- 121
		nRESET_ADC_1: out std_logic; -- 120
		nRESET_ADC_2: out std_logic; -- 108
		nRESET_ADC_3: out std_logic; -- 107
	
		CONVERTERS_96FS_ADC: out std_logic; -- 123
		CONVERTERS_96FS_DAC: out std_logic; -- 133
		CONVERTERS_LEFT_RIGHT_ADC: out std_logic; -- 122
		CONVERTERS_LEFT_RIGHT_DAC: out std_logic; -- 134

		CONVERTER1_SERIAL_DATA_IN: in std_logic; -- 127
		CONVERTER2_SERIAL_DATA_IN: in std_logic; -- 126
		CONVERTER3_SERIAL_DATA_IN: in std_logic; -- 125
		CONVERTER4_SERIAL_DATA_IN: in std_logic; -- 124
		
		CONVERTER1_SERIAL_DATA_OUT: out std_logic; -- 140
		CONVERTER2_SERIAL_DATA_OUT: out std_logic; -- 139
		CONVERTER3_SERIAL_DATA_OUT: out std_logic; -- 138
		CONVERTER4_SERIAL_DATA_OUT: out std_logic; -- 137
		
	-- Interrupt pins.
		INT_MIDI: in std_logic; -- 76
		nINT_DAUGHTER_CARD: in std_logic; -- 75
		nINT_OUTPUT_REQUEST: out std_logic; -- 58 openDrain pin
	
	-- Gain/Monitoring/Mute pins.
		GAIN_CH8_IN: out std_logic;	-- 83
		GAIN_CH7_IN: out std_logic;	-- 84
		GAIN_CH6_IN: out std_logic;	-- 85
		GAIN_CH5_IN: out std_logic;	-- 86
		GAIN_CH4_IN: out std_logic;	-- 87
		GAIN_CH3_IN: out std_logic;	-- 88
		GAIN_CH2_IN: out std_logic;	-- 89
		GAIN_CH1_IN: out std_logic;	-- 90

		GAIN_CH8_OUT: out std_logic; -- 91		
		GAIN_CH7_OUT: out std_logic; -- 92		
		GAIN_CH6_OUT: out std_logic; -- 93		
		GAIN_CH5_OUT: out std_logic; -- 94		
		GAIN_CH4_OUT: out std_logic; -- 97		
		GAIN_CH3_OUT: out std_logic; -- 98		
		GAIN_CH2_OUT: out std_logic; -- 99		
		GAIN_CH1_OUT: out std_logic; -- 100		

		MUTE_CH8: out std_logic; -- 145
		MUTE_CH7: out std_logic; -- 141
		MUTE_CH6: out std_logic; -- 146
		MUTE_CH5: out std_logic; -- 142
		MUTE_CH4: out std_logic; -- 151
		MUTE_CH3: out std_logic; -- 143
		MUTE_CH2: out std_logic; -- 152
		MUTE_CH1: out std_logic; -- 144

		MON_CH8: out std_logic; -- 161
		MON_CH7: out std_logic; -- 162
		MON_CH6: out std_logic; -- 163
		MON_CH5: out std_logic; -- 164
		MON_CH4: out std_logic; -- 167
		MON_CH3: out std_logic; -- 168
		MON_CH2: out std_logic; -- 169
		MON_CH1: out std_logic; -- 170

	-- Various.
--		SPARE1: out std_logic; -- 118 (directly connected to LB_ADDRESS(1))
		SPARE7: out std_logic; -- 109 (permanently tied to 0)
		TP_1: out std_logic; -- 160
		TP_2: out std_logic; -- 159
		TP_3: out std_logic; -- 158
		TP_4: out std_logic; -- 157
		TP_5: out std_logic; -- 205
		TP_6: out std_logic; -- 206
		TP_7: out std_logic; -- 207
		TP_8: out std_logic -- 208
	);
end SoundCache824;
			   
architecture opis of SoundCache824 is
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
			
			nOutputEnable_read: out std_logic_vector ( 6 downto 0 );
			nOutputEnable_write: out std_logic_vector ( 6 downto 0 )
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
			DC_nPRSNT: in std_logic; -- 73
			DC_CONF_DONE: in std_logic; -- 173
			DC_DATA_BIT: out std_logic; -- 175
			DC_DATA_CLK: out std_logic; -- 174
			DC_nCONFIG: out std_logic; -- 171
			DC_nSTATUS: in std_logic -- 172 
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
			
			nRESET_ENGINES: out std_logic;	-- connect also to pin??(for daughter card)
			nENGINES_STOP_REQUEST: out std_logic; -- connect also to pin?? (for daughter card)
			
			-- Clocks for detection.
			CLOCK_DAUGHTER_CARD: in std_logic; -- 117
			CLOCK_SYNC_IN: in std_logic -- 106
		);
	end component;
	-- 0x04, 0x0C
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
		
		-- Next one should be commente out on final release.
			nLB_ENABLE_READ_BUS_WAIT_TIMER: in std_logic;
			nLB_ENABLE_WRITE_BUS_WAIT_TIMER: in std_logic;
		
		-- Pins dedicated to register.
			-- address counter
			nRESET_ENGINE: in std_logic;
			CLK_SAMPLE_RATE: in std_logic;
			
			-- memory controller
			LB_ADDRESS_16TO2: out std_logic_vector ( 14 downto 0 ); -- full address ( channel + sample select )
			LB_HOLD: out std_logic; -- 45
			LB_HOLDA: in std_logic; -- 46
			
			nMEMORY_CE: out std_logic;
			nMEMORY_OE: out std_logic;
			nMEMORY_WE: out std_logic;

			-- transfer engine
			TE_CHANNEL_SELECT: out std_logic_vector( 2 downto 0 );
			nTE_READ_WRITE_ENABLE: out std_logic;
			nTE_STOP_REQUEST: in std_logic;
			TE_TRANSFER_REQUEST: in std_logic
		);
	end component;
	-- 0x06
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
	-- 0x08
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
	-- 0x0A
	component reg_SC8_GAIN
		port
		(
		-- Signals from main.
			LB_CLK: in std_logic;
			nLB_ENABLE_READ: in std_logic;
			nLB_ENABLE_WRITE: in std_logic;
			nLB_RESET: in std_logic;
			LB_DATA: inout std_logic_vector ( 15 downto 0 );
		
		-- Pins dedicated to register.
			GAIN_CH8_IN: out std_logic;	-- 83
			GAIN_CH7_IN: out std_logic;	-- 84
			GAIN_CH6_IN: out std_logic;	-- 85
			GAIN_CH5_IN: out std_logic;	-- 86
			GAIN_CH4_IN: out std_logic;	-- 87
			GAIN_CH3_IN: out std_logic;	-- 88
			GAIN_CH2_IN: out std_logic;	-- 89
			GAIN_CH1_IN: out std_logic;	-- 90

			GAIN_CH8_OUT: out std_logic; -- 91		
			GAIN_CH7_OUT: out std_logic; -- 92		
			GAIN_CH6_OUT: out std_logic; -- 93		
			GAIN_CH5_OUT: out std_logic; -- 94		
			GAIN_CH4_OUT: out std_logic; -- 97		
			GAIN_CH3_OUT: out std_logic; -- 98		
			GAIN_CH2_OUT: out std_logic; -- 99		
			GAIN_CH1_OUT: out std_logic -- 100		
		);
	end component;
	-- 0x0C ( SC_BUS_WAIT_TIMER -> in transfer engine )
	-- 0x0E
	component reg_SC8_MON_MUTE
		port
		(
		-- Signals from main.
			LB_CLK: in std_logic;
			nLB_ENABLE_READ: in std_logic;
			nLB_ENABLE_WRITE: in std_logic;
			nLB_RESET: in std_logic;
			LB_DATA: inout std_logic_vector ( 15 downto 0 );
		
		-- Pins dedicated to register.
			MUTE_CH8: out std_logic; -- 145
			MUTE_CH7: out std_logic; -- 141
			MUTE_CH6: out std_logic; -- 146
			MUTE_CH5: out std_logic; -- 142
			MUTE_CH4: out std_logic; -- 151
			MUTE_CH3: out std_logic; -- 143
			MUTE_CH2: out std_logic; -- 152
			MUTE_CH1: out std_logic; -- 144

			MON_CH8: out std_logic; -- 161
			MON_CH7: out std_logic; -- 162
			MON_CH6: out std_logic; -- 163
			MON_CH5: out std_logic; -- 164
			MON_CH4: out std_logic; -- 167
			MON_CH3: out std_logic; -- 168
			MON_CH2: out std_logic; -- 169
			MON_CH1: out std_logic -- 170
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
			CLOCK_16MHz: in std_logic; -- 105
			CLOCK_18MHz: in std_logic; -- 128
			CLOCK_DAUGHTER_CARD: in std_logic; -- 117
			CLOCK_SYNC_IN: in std_logic; -- 106
			
			-- output clocks
			CLOCK_BICK_OUT: out std_logic;
			nBICK_24_OUT: out std_logic;
			CLOCK_MASTER_OUT: out std_logic; -- 104 (384 x fs)
			CLOCK_SAMPLE_RATE_OUT: out std_logic
		);
	end component;

	component CONVERTERs_engine
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

	signal nRegisterEnable_read: std_logic_vector ( 6 downto 0 );
	signal nRegisterEnable_write: std_logic_vector ( 6 downto 0 );

	-- SC_ENGINE
	signal clockSelection: std_logic_vector( 1 downto 0 );
	signal nResetEngines: std_logic;
	signal nStopEngines: std_logic;

	-- SC_ADDRESS_and_TRANSFER_ENGINE
	signal nTE_CE: std_logic;
	signal nTE_OE: std_logic;
	signal nTE_WE: std_logic;

	signal TE_channelSelect: std_logic_vector( 2 downto 0 );
	signal nTE_readWriteEnable: std_logic;

	-- CONVERTERs engine
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
-- Set PCI clock to local bus clock.
--   LB_CLOCK_SELECT0#  -> set 18MHz clock to local bus
--   LB_CLOCK_SELECT1#  -> set PCI clock to local bus
--   LB_CLOCK_SELECT2#  -> set master clock to local bus
	nLB_CLOCK_SELECT0 <= '1';
	nLB_CLOCK_SELECT1 <= '0';
	nLB_CLOCK_SELECT2 <= '1';

-- Set DAC sample transfer format type -> DIF2 = 1, DIF1 = 1, DIF0 = 0
-- (notice: first two are hardwired).
--
-- And enable ADCs -> both ADCs and DACs will shut down automatically if
-- you stop the master clock and if system reset happens the audio engine
-- will stop and with that the master clock will be stopped too.
	DAC_DIF0 <= '0';
	nRESET_ADC_0 <= '1';
	nRESET_ADC_1 <= '1';
	nRESET_ADC_2 <= '1';
	nRESET_ADC_3 <= '1';

-- As daughter card connectors pin names differ slightly from the 4xx cards
-- (some pins are not connected or have different meaning than with 4xx cards),
-- we need to restore the proper order.
	nCS_FPGA_DAUGHTER <= nCS_FPGA;

-- Daughter card connector.
--	SPARE1 <= LB_ADDRESS(1);

-- Some pins are left in the 'air' so we'll tie them to ground.
	SPARE7 <= '0';
	TP_1 <= '0';
	TP_2 <= '0';
	TP_3 <= '0';
	TP_4 <= '0';
	TP_5 <= '0';
	TP_6 <= '0';
	TP_7 <= '0';
	TP_8 <= '0';

-------------------------------------------------------------------------------
-- Memory access. -------------------------------------------------------------
-------------------------------------------------------------------------------
-- Logic for memory access(the code needs to create signals to enable memory)
--
-- nBANK_CE: out std_logic; -- 156
-- nBANK_OE: out std_logic; -- 154
-- nBANK_WE: out std_logic; -- 155
--
-- nCS_LM: in std_logic; -- 59
-- nLB_RD: in std_logic; -- 68
-- nLB_WR: in std_logic; -- 69
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
	nBANK_CE <= nBank_OE_s and nBank_WE_s and nTE_CE;

	-- Set upper 8bits.
	upperByte <= x"00" when ( LB_DATA( 23 ) = '0' ) else x"FF";
	process ( nMemory_readEnable, upperByte )
	begin		
		if ( nMemory_readEnable = '0' ) then
			LB_DATA_UPPER( 7 downto 0 ) <= upperByte;
		else
			LB_DATA_UPPER( 7 downto 0 ) <= ( LB_DATA_UPPER'range => 'Z' );
		end if;
	end process;

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
--WALDEMAR
-- DaughterCard is disabled.
	nFPGA_readEnable <= nCS_FPGA or nLB_RD; -- only when all are low
	nFPGA_writeEnable <= nCS_FPGA or nLB_WR;

	LD: lineDecoder_3to8 port map
	(
		LB_ADDRESS_16TO1( 2 downto 0 ),
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
		DC_nPRSNT,
		DC_CONF_DONE,
		DC_DATA_BIT,
		DC_DATA_CLK,
		DC_nCONFIG,
		DC_nSTATUS
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
		
		--Daughter card.
		DC_nFINISHED_TRANSFER,
		DC_nSTART_TRANSFER,
		
		-- Next one should be commented out on final release.
		-- This one is localBus wait time counter.
		nRegisterEnable_read( 5 ),
		nRegisterEnable_write( 5 ),
		
	-- Pins dedicated to register.
		-- address counter
		nResetEngines,
		clock_sampleRate,
		
		-- memory controller
		LB_ADDRESS_16TO1( 15 downto 1 ),
		LB_HOLD,
		LB_HOLDA,
		
		nTE_CE,
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
	SC8_GAIN: reg_SC8_GAIN port map
	(
	-- Signals from main.
		LB_CLK,
		nRegisterEnable_read( 4 ),
		nRegisterEnable_write( 4 ),
		nLB_RESET,
		LB_DATA( 15 downto 0 ),

	-- Pins dedicated to register.
		GAIN_CH8_IN,
		GAIN_CH7_IN,
		GAIN_CH6_IN,
		GAIN_CH5_IN,
		GAIN_CH4_IN,
		GAIN_CH3_IN,
		GAIN_CH2_IN,
		GAIN_CH1_IN,

		GAIN_CH8_OUT,
		GAIN_CH7_OUT,
		GAIN_CH6_OUT,
		GAIN_CH5_OUT,
		GAIN_CH4_OUT,
		GAIN_CH3_OUT,
		GAIN_CH2_OUT,
		GAIN_CH1_OUT
	);
	-- 0x0E
	SC8_MON_MUTE: reg_SC8_MON_MUTE port map
	(
	-- Signals from main.
		LB_CLK,
		nRegisterEnable_read( 6 ),
		nRegisterEnable_write( 6 ),
		nLB_RESET,
		LB_DATA( 15 downto 0 ),
		
	-- Pins dedicated to register.
		MUTE_CH8,
		MUTE_CH7,
		MUTE_CH6,
		MUTE_CH5,
		MUTE_CH4,
		MUTE_CH3,
		MUTE_CH2,
		MUTE_CH1,
		
		MON_CH8,
		MON_CH7,
		MON_CH6,
		MON_CH5,
		MON_CH4,
		MON_CH3,
		MON_CH2,
		MON_CH1
	);

-- Engines ------------------------------------------------------------------
	CLKE: clocks_engine port map
	(
	--Pins dedicated to engine.
		clockSelection,
		nStopEngines,

		-- input clocks
		CLOCK_16MHz,
		CLOCK_18MHz,
		CLOCK_DAUGHTER_CARD,
		CLOCK_SYNC_IN,

		-- output clocks
		clock_bick,
		nBick_24,
		CLOCK_MASTER_OUT,
		clock_sampleRate
	);
	CONVERTERS_96FS_ADC <= clock_bick;
	CONVERTERS_96FS_DAC <= clock_bick;
	
	CONVERTERS_LEFT_RIGHT_ADC <= clock_sampleRate;
	CONVERTERS_LEFT_RIGHT_DAC <= clock_sampleRate;
	
	CE: CONVERTERs_engine port map
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
		
		CONVERTER1_SERIAL_DATA_IN,
		CONVERTER2_SERIAL_DATA_IN,
		CONVERTER3_SERIAL_DATA_IN,
		CONVERTER4_SERIAL_DATA_IN,
			
		CONVERTER1_SERIAL_DATA_OUT,
		CONVERTER2_SERIAL_DATA_OUT,
		CONVERTER3_SERIAL_DATA_OUT,
		CONVERTER4_SERIAL_DATA_OUT
	);

end architecture opis;

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
		
		nOutputEnable_read: out std_logic_vector ( 6 downto 0 );
		nOutputEnable_write: out std_logic_vector ( 6 downto 0 )
	);
end lineDecoder_3to8;

architecture opis of lineDecoder_3to8 is
	signal nAddressEnable: std_logic_vector ( 7 downto 0 );
	
begin
	nAddressEnable( 0 ) <= '0' when ( address = b"000" ) else '1'; -- 0x00
	nAddressEnable( 1 ) <= '0' when ( address = b"001" ) else '1'; -- 0x02
	nAddressEnable( 2 ) <= '0' when ( address = b"010" ) else '1'; -- 0x04
	nAddressEnable( 3 ) <= '0' when ( address = b"011" ) else '1'; -- 0x06
	nAddressEnable( 4 ) <= '0' when ( address = b"100" ) else '1'; -- 0x08
	nAddressEnable( 5 ) <= '0' when ( address = b"101" ) else '1'; -- 0x0A
	nAddressEnable( 6 ) <= '0' when ( address = b"110" ) else '1'; -- 0x0C
	nAddressEnable( 7 ) <= '0' when ( address = b"111" ) else '1'; -- 0x0E
	
-- Read registers: 0, 1, 2, 4, 5, 6
	-- SC_DEVICES (read/write)
	nOutputEnable_read( 0 ) <= nEnable_read or nAddressEnable( 0 ); -- 0x00
	-- SC_ENGINE (read/write)
	nOutputEnable_read( 1 ) <= nEnable_read or nAddressEnable( 1 ); -- 0x02
	-- SC_SAMPLE_COUNTER (read)
	nOutputEnable_read( 2 ) <= nEnable_read or nAddressEnable( 2 ); -- 0x04
	-- SC_INTERRUPT_DOWNCOUNTER (write)
	-- no code
	-- SC_INTERRUPT (read/write)
	nOutputEnable_read( 3 ) <= nEnable_read or nAddressEnable( 4 ); -- 0x08
	-- SC8_GAIN (read/write)	
	nOutputEnable_read( 4 ) <= nEnable_read or nAddressEnable( 5 ); -- 0x0A
	-- SC_BUS_WAIT_TIMER (read/write)
	nOutputEnable_read( 5 ) <= nEnable_read or nAddressEnable( 6 ); -- 0x0C
	-- SC8_MON_MUTE (read/write)
	nOutputEnable_read( 6 ) <= nEnable_read or nAddressEnable( 7 ); -- 0x0E

-- Write registers: 0, 1, 3, 4, 5, 6
	-- SC_DEVICES (read/write)
	nOutputEnable_write( 0 ) <= nEnable_write or nAddressEnable( 0 ); -- 0x00
	-- SC_ENGINE (read/write)
	nOutputEnable_write( 1 ) <= nEnable_write or nAddressEnable( 1 ); -- 0x02
	-- SC_SAMPLE_COUNTER (read)
	-- no code
	-- SC_INTERRUPT_DOWNCOUNTER (write)
	nOutputEnable_write( 2 ) <= nEnable_write or nAddressEnable( 3 ); -- 0x06
	-- SC_INTERRUPT (read/write)
	nOutputEnable_write( 3 ) <= nEnable_write or nAddressEnable( 4 ); -- 0x08
	-- SC8_GAIN (read/write)
	nOutputEnable_write( 4 ) <= nEnable_write or nAddressEnable( 5 ); -- 0x0A
	-- SC_BUS_WAIT_TIMER (read/write)
	nOutputEnable_write( 5 ) <= nEnable_write or nAddressEnable( 6 ); -- 0x0C
	-- SC8_MON_MUTE (read/write)
	nOutputEnable_write( 6 ) <= nEnable_write or nAddressEnable( 7 ); -- 0x0E

end opis;

