# Object-Detection-SoC
Hardware/software co-designed system to compute MJPEG frame processing and object detection algorithms.

## Requirements ##
* Quartus Prime 16.1 or higher
* Cyclone V Device Support for Quartus Prime
* Intel SoC EDS 16.1 or higher
* Intel FPGA UPDS
* PuTTY
* Terasic DE1-SoC Board (with Linux ARM Micro SD Card)
* USB 2.0 Storage Device
* VGA Monitor

## Setup ##
### Hardware Setup ###
1. Open the project file in Quartus Prime from the hardware folder.
2. Start Qsys and generate VHDL for the Computer System component.
3. Compile project into .sof file

### Software Setup ###
1. Open SoC EDS Command Shell and browse to the software folder.
2. Build the solution using "make".
3. Copy the solution binary file ('memdjpeg_VGA') to a USB 2.0 device.

### DE1-SoC Setup ###
1. Use Quartus Prime Programmer to program the DE1-SoC with the compiled .sof file.
2. Login to the Linux system on the ARM core with user root
3. Copy the solution binary file from the USB 2.0 device to a local directory of the Linux system on the DE1-SoC board. 

## Usage ##
1. Run the executable passing two parameters: the source video file and a destination for a decrypted copy. E.g.
```
memdjpeg_VGA DW.mjpeg.cipher DW.mjpeg
``` 
2. Profiling is available in the gmon.out file after each run. Read this file using gprof.
3. Solution customisability is provided via software definitions. Instructions are provided as comments in the main file. Ensure to remake and recopy to DE1-SoC.

## Testing ##
* Test hardware components via the testbenches found in hardware/filters and hardware/trivium
