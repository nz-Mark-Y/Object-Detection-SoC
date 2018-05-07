library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity convolution_filter is
  port(clk, div_flag : in std_logic;
    input_pixels : in unsigned(71 downto 0);  
    kernel : in unsigned(35 downto 0); -- vector is unsigned but represents array of 9 4-bit signed numbers
    output_pixel : out unsigned(7 downto 0));
end entity convolution_filter;

architecture bhv of convolution_filter is
  signal s_output : unsigned(17 downto 0) := "000000000000000000";
begin
  process(clk)
    variable v_pixels: unsigned(71 downto 0);
    variable sum : signed (17 downto 0) := "000000000000000000";
  begin
    if (rising_edge(clk)) then
      v_pixels := input_pixels;

      for i in 0 to 8 loop
        sum := sum + signed(kernel(((i+1)*4)-1 downto i*4)) * signed(v_pixels(((i+1)*8)-1 downto i*8));  
      end loop;

      if (sum < 0) then
        sum := "000000000000000000";
      end if;		

      if (div_flag = '1') then
        s_output <= unsigned("00" & sum(17 downto 2));
      else
        s_output <= unsigned(sum);
      end if;
      
    end if;
  end process;
  output_pixel <= s_output(7 downto 0);
end architecture bhv;
