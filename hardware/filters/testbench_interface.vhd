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
  begin
    wait for 1 ns;
    t_clk <= '1';
    t_inp <= x"01010153";
    wait for 1 ns;
    t_clk <= '0';
    t_inp <= x"01010153";
    wait for 1 ns;
    t_clk <= '1';
    t_inp <= x"01010153";
    wait for 1 ns;
    t_clk <= '0'; 
    t_inp <= x"01010153";
    wait for 1 ns;
    t_clk <= '1';
    t_inp <= x"01010153";
    wait for 1 ns;
    t_clk <= '0';
    t_inp <= x"01010153";      
  end process;
end beh;




