library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity testbench_median is
end testbench_median;

architecture beh of testbench_median is
  signal t_clk : std_logic;
  signal t_input_pixels : unsigned(71 downto 0) := x"090807060504030201";
  signal t_output_pixel : unsigned(7 downto 0);
  
  component median_filter is
    port(clk : in std_logic;
      input_pixels : in unsigned(71 downto 0);  
      output_pixel : out unsigned(7 downto 0));
  end component;

begin
  test_filter : median_filter port map(t_clk, t_input_pixels, t_output_pixel);
    
  test : process
  begin
    wait for 1 ns;
    t_clk <= '1';
    wait for 1 ns;
    t_clk <= '0';
      
  end process;
end beh;



