library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity testbench_convolution is
end testbench_convolution;

architecture beh of testbench_convolution is
  signal t_clk : std_logic;
  signal t_div_flag : std_logic := '0';
  signal t_input_pixels : unsigned(71 downto 0) := x"000000010101010101";
  signal t_kernel : unsigned(35 downto 0) := x"111111111";
  signal t_output_pixel : unsigned(7 downto 0);
  
  component convolution_filter is
    port(clk, div_flag : in std_logic;
      input_pixels : in unsigned(71 downto 0);
      kernel : in unsigned(35 downto 0);  
      output_pixel : out unsigned(7 downto 0));
  end component;

begin
  test_filter : convolution_filter port map(t_clk, t_div_flag, t_input_pixels, t_kernel, t_output_pixel);
    
  test : process
  begin
    wait for 1 ns;
    t_clk <= '1';
    wait for 1 ns;
    t_clk <= '0';
      
  end process;
end beh;





