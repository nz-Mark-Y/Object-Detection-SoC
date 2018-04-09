library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity key_setup_testbench is
end key_setup_testbench;

architecture beh of key_setup_testbench is
  signal t_clk : std_logic;
  signal t_start : std_logic := '1';
  signal t_done : std_logic := '0';
  signal t_K  : std_logic_vector(79 downto 0) := (others => '0'); 
  signal t_IV : std_logic_vector(79 downto 0) := (others => '0');
  signal t_output : std_logic_vector(287 downto 0);
  
  component key_setup_module is
    port (clk, start : in std_logic;
      key, vector : in std_logic_vector(79 downto 0);   
      out_state : out std_logic_vector(287 downto 0);
      done : out std_logic);
  end component; 

begin
    test_module : key_setup_module port map(t_clk, t_start, t_K, t_IV, t_output, t_done);
    
    test : process
    begin
      wait for 10 ms;
      t_clk <= '1';
      wait for 10 ms;
      t_clk <= '0';
      
    end process test;

end beh;
