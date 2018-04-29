library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity median_filter is
  port( clk : in std_logic;
    input_pixels : in std_logic_vector(71 downto 0);  
    output_pixel : out std_logic_vector(7 downto 0));
end entity median_filter;

architecture bhv of median_filter is
  signal s_output : std_logic_vector(7 downto 0) := "00000000";
begin
  process(clk)
  begin
    if (rising_edge(clk)) then
      s_output <= "00000000";
    end if;
  end process;
  output_pixel <= s_output;
end architecture bhv;