library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity cipher_testbench is
end cipher_testbench;

architecture beh of cipher_testbench is
  signal t_clk, t_out_bit : std_logic;
  signal t_start : std_logic := '1';
  signal t_input_state : std_logic_vector(287 downto 0) := x"255890FC4CFEB0E3A74312DBEA2B5A5FD5082640815C80B1643671D8D255CC3AE926350A";
  
  component cipher_module is
    port (clk, start : in std_logic;
      input_state : in std_logic_vector(287 downto 0);
      out_bit : out std_logic);
  end component;

begin
    test_module : cipher_module port map(t_clk, t_start, t_input_state, t_out_bit);
    
    test : process
    begin
      wait for 1 ms;
      t_clk <= '1';
      wait for 1 ms;
      t_clk <= '0';
      
    end process;

end beh;


