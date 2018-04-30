library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity sobel_filter is
  port(clk : in std_logic;
    input_pixels : in unsigned(71 downto 0);  
    output_pixel : out unsigned(7 downto 0));
end entity sobel_filter;

architecture bhv of sobel_filter is
  signal s_output : unsigned(7 downto 0) := "00000000";
  
  function sqrt (Arg: unsigned) return unsigned is -- http://computer-programming-forum.com/42-vhdl/6d9bdb4a76dd7da8.htm
    constant AMSB: integer:= Arg'length-1; 
    constant RMSB: integer:= (Arg'length/2) - 1; 
    variable Root: unsigned(RMSB downto 0); 
    variable Test: unsigned(RMSB+1 downto 0); 
    variable Rest: unsigned(AMSB+1 downto 0); 
  begin 
    Root := (others => '0'); 
    Rest := '0' & Arg; 
    for i in RMSB downto 0 loop 
      Test := Root(RMSB-1 downto 0 ) & "01";   
      if Test(RMSB-i+1 downto 0) > Rest(AMSB+1 downto 2*i) then 
        Root := Root(RMSB-1 downto 0) & '0'; 
      else 
        Root := Root(RMSB-1 downto 0) & '1'; 
        Rest(AMSB downto i*2) := Rest(AMSB downto i*2) - Test(RMSB-i+1 downto 0); 
      end if; 
    end loop; 
    return Root; 
  end; 
  
begin
  process(clk)
    variable v_pixels: unsigned(71 downto 0);
    variable sum_x : unsigned(15 downto 0) := "0000000000000000";
    variable sum_y : unsigned(15 downto 0) := "0000000000000000";
  begin
    if (rising_edge(clk)) then
      v_pixels := input_pixels;
      
      sum_x := sum_x + "00000001" * v_pixels(7 downto 0);
      sum_y := sum_y + "00000001" * v_pixels(7 downto 0);
      sum_x := sum_x + "00000000" * v_pixels(15 downto 8);
      sum_y := sum_y + "00000010" * v_pixels(15 downto 8);
      sum_x := sum_x - "00000001" * v_pixels(23 downto 16);
      sum_y := sum_y + "00000001" * v_pixels(23 downto 16);
      sum_x := sum_x + "00000010" * v_pixels(31 downto 24);
      sum_y := sum_y + "00000000" * v_pixels(31 downto 24);
      sum_x := sum_x + "00000000" * v_pixels(39 downto 32);
      sum_y := sum_y + "00000000" * v_pixels(39 downto 32);
      sum_x := sum_x - "00000010" * v_pixels(47 downto 40);
      sum_y := sum_y + "00000000" * v_pixels(47 downto 40);
      sum_x := sum_x + "00000001" * v_pixels(55 downto 48);
      sum_y := sum_y - "00000001" * v_pixels(55 downto 48);
      sum_x := sum_x + "00000000" * v_pixels(63 downto 56);
      sum_y := sum_y - "00000010" * v_pixels(63 downto 56);
      sum_x := sum_x - "00000001" * v_pixels(71 downto 64);
      sum_y := sum_y - "00000001" * v_pixels(71 downto 64);
      
      s_output <= sqrt(sum_x*sum_x + sum_y*sum_y);
    end if;
  end process;
  output_pixel <= s_output;
end architecture bhv;


