--***************************************************************************
--***************************************************************************
--
--                          PROPRIETARY INFORMATION
--
-- This software is supplied under the terms of a license agreement or
-- nondisclosure agreement with Waldemar Haszlakiewicz and may not be
-- copied or disclosed except in accordance with the terms of that agreement.
--
--               		Copyright (C) 2005 Waldemar Haszlakiewicz
--                            All Rights Reserved.
--
--***************************************************************************
--***************************************************************************

-- It's a downcounter which will signal audioEngine interrupt when
-- it commes to 0.
-- Because of the hardware design the new value should be:
-- bufferSize + 1 - (channel address & (bufferSize-1))
--
-- Ex.:
-- At start:
-- bufferSize = 0x100
-- channel address = 0
-- new value = bufferSize + 1 = 0x101
--
-- In between:
-- bufferSize = 0x100
-- channel address = 0x501
-- new value = bufferSize + 1 - (channel address & (bufferSize-1)) =
--           = 0x100 + 1 - (0x501 & 0xFF) = 0x100 + 1 - 1 = 0x100
--
-- bit15 - R  - (0)
-- bit14 - R  - (0)
-- bit13 - R  - (0)
-- bit12 - R  - (0)
-- bit11 - R  - (0)
-- bit10 - W  - (x) - \
-- bit9  - W  - (x) - |
-- bit8  - W  - (x) - |
-- bit7  - W  - (x) - |
-- bit6  - W  - (x) - |
-- bit5  - W  - (x) - |-> new value
-- bit4  - W  - (x) - |
-- bit3  - W  - (x) - |
-- bit2  - W  - (x) - |
-- bit1  - W  - (x) - |
-- bit0  - W  - (x) - /
--
-- Notes:
-- ( ) means value after reset.
--
LIBRARY IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;

entity reg_SC_INTERRUPT_DOWNCOUNTER is
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
end reg_SC_INTERRUPT_DOWNCOUNTER;

architecture opis of reg_SC_INTERRUPT_DOWNCOUNTER is
-- BufferSizes: 1024(max), 512, 256, 128, 64
--
-- Because I removed all the interrupt signal delays, I've added
-- the 10th bit which will enable us to insert a value bigger
-- than 512 so we'll set the 2 sample rate delays externally.
	signal IDC_downCounter: std_logic_vector( 10 downto 0 );
	signal IDC_downCounter_back: std_logic_vector( 10 downto 0 );
	
begin
-- Because the new value is set asynchronously there will be gliches.
	process ( CLOCK_SAMPLE_RATE, nLB_ENABLE_WRITE, LB_DATA )
	begin
		if ( nLB_ENABLE_WRITE = '0' ) then
			IDC_downCounter <= LB_DATA( 10 downto 0 );
		else
			if falling_edge( CLOCK_SAMPLE_RATE ) then
				IDC_downCounter <= IDC_downCounter_back;
			end if;
		end if;
	end process;
	
-- Substract the value.
	nextValue: process ( IDC_downCounter )
	begin
		if ( IDC_downCounter = b"00000000000" ) then
			IDC_downCounter_back <= b"00000000000";
		else
			IDC_downCounter_back <= IDC_downCounter - b"00000000001";
		end if;
	end process;
	
-- Fire interrupt.
--
-- When counter commes to 0 the interrupt should be fired.
-- So as soon we'll set a new value into the register the interrupt will be
-- cleared.
	ENGINE_INT_REQUEST <= '1' when ( IDC_downCounter = b"00000000000" ) else '0';

end opis;