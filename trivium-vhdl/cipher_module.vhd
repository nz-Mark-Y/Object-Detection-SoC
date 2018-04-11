library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity cipher_module is
  port (clk, start : in std_logic;
    input_state : in std_logic_vector(287 downto 0);
    out_bit : out std_logic);
end entity cipher_module;

architecture bhv of cipher_module is
  signal started, z1 : std_logic := '0';
begin
  process(clk)
    variable internal_state : std_logic_vector(287 downto 0) := (others => '0');
    variable t1, t2, t3 : std_logic := '0';
  begin
    if (falling_edge(clk)) then
      if (started = '0' and start = '1') then
        started <= '1';
        internal_state(287 downto 0) := input_state(287 downto 0);
      end if;
      
      t1 := internal_state(65) xor internal_state(92);
      t2 := internal_state(161) xor internal_state(176);
      t3 := internal_state(242) xor internal_state(287);
      z1 <= t1 xor t2 xor t3;
      
      t1 := t1 xor (internal_state(90) and internal_state(91)) xor internal_state(170);
      t2 := t2 xor (internal_state(174) and internal_state(175)) xor internal_state(263);
      t3 := t3 xor (internal_state(285) and internal_state(286)) xor internal_state(68);
      internal_state(92 downto 1) := internal_state(91 downto 0);
      internal_state(0) := t3;
      internal_state(176 downto 94) := internal_state(175 downto 93);
      internal_state(93) := t1;
      internal_state(287 downto 178) := internal_state(286 downto 177);
      internal_state(177) := t2;
    end if; 
    out_bit <= z1; 
  end process;
end architecture bhv;


