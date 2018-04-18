library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity trivium_module is
  port (clk, start, reset : in std_logic;
    K, IV : in std_logic_vector(79 downto 0);
    input : in std_logic;
    ready, output : out std_logic);
end entity trivium_module;

architecture bhv of trivium_module is
  signal clk_div, done, stream, s_output, s_reset : std_logic := '0';
  signal internal_state : std_logic_vector(287 downto 0) := (others => '0');
  signal out_count : integer range 0 to 3  := 0;
  
  component key_setup_module is
    port (clk, start, reset : in std_logic;
      key, vector : in std_logic_vector(79 downto 0);   
      out_state : out std_logic_vector(287 downto 0);
      done : out std_logic);
  end component;
  
  component cipher_module is
    port (clk, start, hold, reset : in std_logic;
      input_state : in std_logic_vector(287 downto 0);
      out_bit : out std_logic);
  end component;
 
begin
  key_setup_module_1 : key_setup_module port map (clk, start, s_reset, K, IV, internal_state, done);
  cipher_module_1 : cipher_module port map (clk, done, start, s_reset, internal_state, stream);
  process(clk)
    variable v_count : integer range 0 to 11 := 0;
    variable v_out : integer range 0 to 3 := 0;
  begin
    if(reset = '1') then
      v_count := 0;
      v_out := 0;
      s_reset <= '1';
    else
      if (rising_edge(clk)) then
        if (done =  '1' and v_out < 2) then
          v_out := v_out + 1;
        end if;
      end if;
      out_count <= v_out;
    end if;
  end process;
  ready <= done;
  output <= input xor stream when done = '1' and out_count > 0;
end architecture bhv;