library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity median_filter is
  port(clk : in std_logic;
    input_pixels : in unsigned(71 downto 0);  
    output_pixel : out unsigned(7 downto 0));
end entity median_filter;

architecture bhv of median_filter is
  signal s_output : unsigned(7 downto 0) := "00000000";
begin
  process(clk)
    variable v_pixels: unsigned(71 downto 0); -- Store the 9 pixels in a variable
    variable temp : unsigned(7 downto 0) := "00000000";
  begin
    if (rising_edge(clk)) then
      v_pixels := input_pixels;
      
      for i in 1 to 8 loop -- Bubble sort the 9 pixels
        for j in 1 to 8 loop
          if (v_pixels((j*8)-1 downto (j-1)*8) > v_pixels(((j+1)*8)-1 downto j*8)) then
            temp := v_pixels(((j+1)*8)-1 downto j*8);
            v_pixels(((j+1)*8)-1 downto j*8) := v_pixels((j*8)-1 downto (j-1)*8);
            v_pixels((j*8)-1 downto (j-1)*8) := temp;
          end if;
        end loop;
      end loop;
      
      s_output <= v_pixels(39 downto 32); -- Write middle pixel value to output pixel
    end if;
  end process;
  output_pixel <= s_output;
end architecture bhv;
