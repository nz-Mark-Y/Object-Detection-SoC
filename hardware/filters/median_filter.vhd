library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity median_filter is
  port(clk : in std_logic;
    input_pixels : in std_logic_vector(71 downto 0);  
    output_pixel : out std_logic_vector(7 downto 0));
end entity median_filter;

architecture bhv of median_filter is
  type vector_array is array (8 downto 0) of unsigned(7 downto 0);
  signal s_output : std_logic_vector(7 downto 0) := "00000000";
begin
  process(clk)
    variable v_pixels : vector_array;
    variable temp : unsigned(7 downto 0) := "00000000";
  begin
    if (rising_edge(clk)) then
      v_pixels(0) := unsigned(input_pixels(7 downto 0));
      v_pixels(1) := unsigned(input_pixels(15 downto 8));
      v_pixels(2) := unsigned(input_pixels(23 downto 16));
      v_pixels(3) := unsigned(input_pixels(31 downto 24));
      v_pixels(4) := unsigned(input_pixels(39 downto 32));
      v_pixels(5) := unsigned(input_pixels(47 downto 40));
      v_pixels(6) := unsigned(input_pixels(55 downto 48));
      v_pixels(7) := unsigned(input_pixels(63 downto 56));
      v_pixels(8) := unsigned(input_pixels(71 downto 64));
      
      for i in 1 to 8 loop
        for j in 1 to 8 loop
          if (v_pixels(j-1) > v_pixels(j)) then
            temp := v_pixels(j);
            v_pixels(j) := v_pixels(j-1);
            v_pixels(j-1) := temp;
          end if;
        end loop;
      end loop;
      
      s_output <= std_logic_vector(v_pixels(4));
    end if;
  end process;
  output_pixel <= s_output;
end architecture bhv;