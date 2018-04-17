library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity HPS_Interface is
	port(clk, ready : in std_logic;
		dataANDcontrol : in std_logic_vector(23 downto 0);
		reset, input, start : out std_logic;
		IV, KEY : out std_logic_vector(79 downto 0));
end entity;

architecture beh of HPS_Interface is
--dataANDcontrol is structured as follows:
--Bits 20 to 23 are control bits used for instructions
--Bits 0 to 19 are storing the data to be processed
signal s_IV, s_KEY : std_logic_vector(79 downto 0) := x"00000000000000000000";

begin

process(clk)
variable control : std_logic_vector(3 downto 0) := x"0";
variable data : std_logic_vector(19 downto 0) := x"00000";
variable v_reset, v_start, v_input : std_logic;
variable v_shifter, v_addA, v_addB : std_logic_vector(79 downto 0);
begin
	if(rising_edge(clk)) then
		v_reset := '0';
		v_start := '0';
		v_input := '0';
		data := dataANDcontrol(19 downto 0);
		control := dataANDcontrol(23 downto 20);
		
		if(control(3 downto 0) = "0000") then
			s_IV <= x"00000000000000000000";
			s_KEY <= x"00000000000000000000";
			v_reset := '1';
		elsif (control(3 downto 0) = "0001") then
			v_start := '1';
		elsif (control(3 downto 0) = "0010") then
			--DLC
		elsif (control(3 downto 0) = "0011") then
			v_input := data(0) AND ready;		
		elsif(control(3) = '1') then
		
			if(control(2) = '0') then
				v_addA := s_KEY;
			else
				v_addA := s_IV;
			end if;
			
			case control(1 downto 0) is
				when "00" => v_shifter := x"000000000000000" & data;
				when "01" => v_shifter := x"0000000000" & data & x"00000";
				when "10" => v_shifter := x"00000" & data & x"0000000000";
				when "11" => v_shifter := data & x"000000000000000";
			  when others => null;
			end case;
			
			v_addB := v_addA + v_shifter;
			
			if(control(2) = '0') then
				s_KEY <= v_addB;
			else
				s_IV <= v_addB;
			end if;
			
			
		end if;
	
	end if;
	reset <= v_reset;
	input <= v_input;
	start <= v_start;
end process;
IV <= s_IV;
KEY <= s_KEY;
end architecture;