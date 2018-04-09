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
### System Setup ###
1. Download and unzip project files from Canvas.
2. Download and unzip project amendment files from Canvas.
3. Follow project amendment README to complete project amendment.
### Software Setup ###
1. Clone this repository into project folder.
2. Open SoC EDS Command Shell and browse to the repository folder.
3. Build the solution using "make".
4. Copy the solution binary file to a USB 2.0 device.
### DE1-SoC Setup ###
1. Use Quartus Prime Programmer to program the DE1-SoC with the project .sof file.
2. Login to the Linux system on the ARM core with user root
3. Copy the solution binary file from the USB 2.0 device to the root directory of the Linux system on the DE1-SoC board. 

## Usage ##
