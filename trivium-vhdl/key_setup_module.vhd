library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity key_setup_module is
  port (clk, start, reset : in std_logic;
    key, vector : in std_logic_vector(79 downto 0);   
    out_state : out std_logic_vector(287 downto 0);
    done : out std_logic);
end entity key_setup_module;

architecture bhv of key_setup_module is
  signal s_done : integer range 0 to 37 := 0;
begin
  process(clk)
    variable internal_state : std_logic_vector(287 downto 0) := (others => '0');
    variable v1, t1, t2, t3 : std_logic := '0';
  begin
    if(reset = '1') then
      s_done <= 0;
      internal_state := (others => '0');
      v1 := '0';
      t1 := '0';
      t2 := '0';
      t3 := '0';
    else
      if (rising_edge(clk)) then
        if (start = '1') then
          if (s_done = 0) then
            internal_state(79 downto 0) := key(79 downto 0);
            internal_state(92 downto 80) := (others => '0');
            internal_state(172 downto 93) := vector(79 downto 0);
            internal_state(176 downto 173) := (others => '0');
            internal_state(284 downto 177) := (others => '0');
            internal_state(287 downto 285) := (others => '1');
          end if; 
          if (s_done < 36) then
      for i in 1 to 32 loop
        t1 := internal_state(65) xor (internal_state(90) and internal_state(91)) xor internal_state(92) xor internal_state(170);
        t2 := internal_state(161) xor (internal_state(174) and internal_state(175)) xor internal_state(176) xor internal_state(263);
        t3 := internal_state(242) xor (internal_state(285) and internal_state(286)) xor internal_state(287) xor internal_state(68);
        internal_state(92 downto 1) := internal_state(91 downto 0);
        internal_state(0) := t3;
        internal_state(176 downto 94) := internal_state(175 downto 93);
        internal_state(93) := t1;
        internal_state(287 downto 178) := internal_state(286 downto 177);
        internal_state(177) := t2;
      end loop;
            s_done <= s_done + 1;          
          end if;
        end if;
      end if;
      out_state <= internal_state;
    end if;
  end process;
  done <= '1' when s_done > 35 else '0';
end architecture bhv;

