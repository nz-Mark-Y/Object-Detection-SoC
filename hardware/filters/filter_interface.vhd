LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE ieee.numeric_std.all;

ENTITY filter_interface IS
    PORT(clk : IN STD_LOGIC;
    inputA, inputB, inputC : IN STD_LOGIC_VECTOR(23 DOWNTO 0);
    inputD : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
    outp  : OUT STD_LOGIC_VECTOR(23 DOWNTO 0));
END ENTITY filter_interface;

ARCHITECTURE BEH OF filter_interface IS

    SIGNAL outpet : STD_LOGIC_VECTOR(23 DOWNTO 0) := x"000000";
    SIGNAL ida, idb : STD_LOGIC_VECTOR(15 DOWNTO 0) := x"0000";
    SIGNAL in_pixels : UNSIGNED(71 DOWNTO 0);
    SIGNAL kernal : UNSIGNED(35 DOWNTO 0) := x"0101C1010";
    SIGNAL out_pixels : UNSIGNED(7 DOWNTO 0);
    SIGNAL divi : STD_LOGIC := '0';

    COMPONENT convolution_filter IS
        PORT(clk, div_flag : IN STD_LOGIC;
          input_pixels : IN UNSIGNED(71 DOWNTO 0);  
          kernel : IN UNSIGNED(35 DOWNTO 0);
          output_pixel : OUT UNSIGNED(7 DOWNTO 0));
    END COMPONENT;

BEGIN

    convo_filter_1 : convolution_filter PORT MAP ( clk, divi, in_pixels, kernal, out_pixels); -- Just using convolution filter for now

    PROCESS(clk)
    BEGIN
        IF (rising_edge(clk)) THEN 
            in_pixels <= UNSIGNED(inputA & inputB & inputC); -- Input comes in three 24-bit sections. AND them together for 72 bit vector
            ida <= inputD;
            idb <= ida;
            outpet <= idb & STD_LOGIC_VECTOR(out_pixels); -- Ensure id is returned with output so the C code recognises it
        END IF;
    END PROCESS;
    outp <= outpet;
END ARCHITECTURE;

