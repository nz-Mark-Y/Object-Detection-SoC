library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity trivium_module is
  port (clk, start : in std_logic;
    K, IV : in std_logic_vector(79 downto 0);
    input : in std_logic;
    ready, output, stream_out : out std_logic);
end entity trivium_module;

architecture bhv of trivium_module is
  signal clk_div, done, stream, s_output : std_logic := '0';
  signal internal_state : std_logic_vector(287 downto 0) := (others => '0');
  
  component key_setup_module is
    port (clk, start : in std_logic;
      key, vector : in std_logic_vector(79 downto 0);   
      out_state : out std_logic_vector(287 downto 0);
      done : out std_logic);
  end component;
  
  component cipher_module is
    port (clk, start : in std_logic;
      input_state : in std_logic_vector(287 downto 0);
      out_bit : out std_logic);
  end component;
 
begin
  key_setup_module_1 : key_setup_module port map (clk_div, start, K, IV, internal_state, done);
  cipher_module_1 : cipher_module port map (clk, done, internal_state, stream);
  process(clk)
    variable v_count : integer range 0 to 11 := 0;
  begin
    if (rising_edge(clk)) then
      
      v_count := v_count + 1;
      if (v_count > 10) then
        clk_div <= '1';
        v_count := 0;
      else
        clk_div <= '0';
      end if;
      
    end if;
  end process;
  ready <= done;
  output <= input xor stream when done = '1';
  stream_out <= stream when done = '1';
end architecture bhv;



