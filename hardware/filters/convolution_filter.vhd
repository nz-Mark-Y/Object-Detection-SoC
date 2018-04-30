library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity convolution_filter is
  port(clk, div_flag : in std_logic;
    input_pixels : in unsigned(71 downto 0);  
    kernel : in unsigned(71 downto 0);
    output_pixel : out unsigned(15 downto 0));
end entity convolution_filter;

architecture bhv of convolution_filter is
  signal s_output : unsigned(15 downto 0) := "0000000000000000";
begin
  process(clk)
    variable v_pixels: unsigned(71 downto 0);
    variable sum : unsigned (15 downto 0) := "0000000000000000";
  begin
    if (rising_edge(clk)) then
      v_pixels := input_pixels;

      for i in 0 to 8 loop
        sum := sum + kernel(((i+1)*8)-1 downto i*8) * v_pixels(((i+1)*8)-1 downto i*8);  
      end loop;

      if (div_flag = '1') then
        s_output <= "00" & sum(15 downto 2);
      else
        s_output <= sum;
      end if;
      
    end if;
  end process;
  output_pixel <= s_output;
end architecture bhv;
