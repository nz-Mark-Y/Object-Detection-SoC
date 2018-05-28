library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity testbench_interface is
end testbench_interface;

architecture beh of testbench_interface is 
  signal t_clk : std_logic;
  signal t_inp : std_logic_vector(31 downto 0) := x"00000000";
  signal t_outp : std_logic_vector(15 downto 0) := x"0000";  
  
  component filter_interface is
    port(clk : in std_logic;
    inp : in std_logic_vector(31 downto 0);
    outp  : out std_logic_vector(15 downto 0));
  end component;

begin
  test_filter : filter_interface port map(t_clk, t_inp, t_outp);
    
  test : process
    type input_values is array (0 to 8) of std_logic_vector(31 downto 0);
    variable input_vectors : input_values := (x"01010160", x"02020261", x"03030362", x"01020363", x"04050664", x"07080965", x"01010166", x"01010167", x"01010168");
    variable count : integer := 0;
  begin
    wait for 1 ns;
    t_clk <= '1';
    t_inp <= input_vectors(count);
    wait for 1 ns;
    t_clk <= '0';
    count := count + 1;
    if count = 9 then
      count := 0;
    end if;      
  end process;
end beh;




