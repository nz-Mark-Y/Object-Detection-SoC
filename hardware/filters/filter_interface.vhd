LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE ieee.numeric_std.all;

ENTITY filter_interface IS
    PORT(clk : IN STD_LOGIC;
    inp : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
    outp  : OUT STD_LOGIC_VECTOR(15 DOWNTO 0));
END ENTITY filter_interface;

ARCHITECTURE BEH OF filter_interface IS

    SIGNAL outpet : STD_LOGIC_VECTOR(15 DOWNTO 0) := x"0000";
    SIGNAL bufferA, bufferB, bufferC, bufferD : STD_LOGIC_VECTOR(23 DOWNTO 0) := x"000000";
    SIGNAL in_pixels : UNSIGNED(71 DOWNTO 0);
    SIGNAL kernale : UNSIGNED(35 DOWNTO 0) := x"0101C1010";
    SIGNAL kernalg : UNSIGNED(35 DOWNTO 0) := x"121242121";
    SIGNAL kernali : UNSIGNED(35 DOWNTO 0) := x"000010000";
    SIGNAL out_pixels_edge : UNSIGNED(7 DOWNTO 0);
    SIGNAL out_pixels_gaussian : UNSIGNED(7 DOWNTO 0);
    SIGNAL out_pixels_identity : UNSIGNED(7 DOWNTO 0);
    SIGNAL out_pixels_median : UNSIGNED(7 DOWNTO 0);
    SIGNAL out_pixels_sobel : UNSIGNED(7 DOWNTO 0);
    SIGNAL divi : STD_LOGIC := '0';
    SIGNAL divis : STD_LOGIC := '1';

    COMPONENT convolution_filter IS
        PORT(clk, div_flag : IN STD_LOGIC;
        input_pixels : IN UNSIGNED(71 DOWNTO 0);  
        kernel : IN UNSIGNED(35 DOWNTO 0);
        output_pixel : OUT UNSIGNED(7 DOWNTO 0));
    END COMPONENT;

    COMPONENT median_filter IS
        PORT(clk : IN STD_LOGIC;
        input_pixels : IN UNSIGNED(71 DOWNTO 0);  
        output_pixel : OUT UNSIGNED(7 DOWNTO 0));
    END COMPONENT;

    COMPONENT sobel_filter IS
        PORT(clk : IN STD_LOGIC;
        input_pixels : IN UNSIGNED(71 DOWNTO 0);  
        output_pixel : OUT UNSIGNED(7 DOWNTO 0));
    END COMPONENT;

BEGIN

    edge_filter : convolution_filter PORT MAP (clk, divi, in_pixels, kernale, out_pixels_edge);
    gaussian_filter : convolution_filter PORT MAP (clk, divis, in_pixels, kernalg, out_pixels_gaussian);
    identity_filter : convolution_filter PORT MAP (clk, divi, in_pixels, kernali, out_pixels_identity);
    median_filter_1 : median_filter PORT MAP (clk, in_pixels, out_pixels_median);
    sobel_filter_1 : sobel_filter PORT MAP (clk, in_pixels, out_pixels_sobel);

    PROCESS(clk)
        VARIABLE mux_out : UNSIGNED (7 DOWNTO 0);
        VARIABLE mux_sel : STD_LOGIC_VECTOR(2 DOWNTO 0);
    BEGIN
        IF (rising_edge(clk)) THEN
            -- load incoming data stream into series of shift registers
            bufferA <= inp(31 DOWNTO 24) & bufferA(23 DOWNTO 8);
            bufferB <= inp(23 DOWNTO 16) & bufferB(23 DOWNTO 8);
            bufferC <= inp(15 DOWNTO 8) & bufferC(23 DOWNTO 8);
            bufferD <= inp(7 DOWNTO 0) & bufferD(23 DOWNTO 8);

            -- send register data to the filter blocks
            in_pixels <= UNSIGNED(bufferA & bufferB & bufferC);
            
            -- choose filter output based on control signal
            mux_sel := bufferD(7 DOWNTO 5);
            CASE mux_sel IS
                WHEN "000" => mux_out := out_pixels_edge;
                WHEN "001" => mux_out := out_pixels_gaussian;
                WHEN "010" => mux_out := out_pixels_identity;
                WHEN "011" => mux_out := out_pixels_median;
                WHEN others => mux_out := out_pixels_sobel;
            END CASE;
            outpet <= bufferD(7 DOWNTO 0) & STD_LOGIC_VECTOR(mux_out);
        END IF;
    END PROCESS;
    outp <= outpet;
END ARCHITECTURE;

