# DAQ Directory Documentation
This directory contains the main data acquisition program that runs on the Raspberry Pi 5.
It also contains a basic receiver that can read data being sent by a DAQ process.

## Important files
`daq.c`  
The entry point to the program.
Sets up callbacks for RPM, PWM, and GPS.
Uses a loop to sample ADC and IMU at regular rates.
Uses a loop to send and record data at regular rates.

`gpio.h`, `gpio.c`  
A library for registering interrupt handlers to the Raspberry Pi 5's GPIO pins.
Used for measuring RPM and PWM.

`mcp3424.h`, `mcp3424.c`  
A library for communicating with the MCP3424 ADC using the I2C protocol.

`jy901.h`, `jy901.c`  
A library for communicating with the JY901 IMU using the I2C protocol.

`receive.py`  
This basic program is capable of connecting to the Raspberry Pi, running the data acquisition program, decoding transmitted data, and writing that data to a file.
Two files are written to, `DAQ.csv` and `LOG.csv`.
`DAQ.csv` contains only the last data point, which was useful for basic live data acquisition with `../GUI_app.mlapp`.

## Data Format
For efficiency, data is sent and recorded from the data acquisition program in a non-human-readable format.

Data is sent in "packets" where the value of the first byte designates the type of packet and is followed by a number of data bytes depending on the packet type as follows:

**Time Packet**  
Byte 0: `0`  
Bytes 1-8: Raw C double representing seconds since the epoch

**ADC Packet**  
Byte 0: `1`  
Bytes 1-8: Raw C double representing the voltage of battery 1 cell 1 through a voltage divider.  
Bytes 9-16: Raw C double representing the voltage of battery 1 cells 1-2 through a voltage divider.  
Bytes 17-24: Raw C double representing the voltage of battery 1 cells 1-3 through a voltage divider.  
Bytes 25-32: Raw C double representing the voltage of battery 2 cell 1 through a voltage divider.  
Bytes 33-40: Raw C double representing the voltage of battery 2 cell 2 through a voltage divider.  
Bytes 41-48: Raw C double representing the voltage of battery 2 cell 3 through a voltage divider.

**IMU Packet**  
Byte 0: `2`  
Bytes 1-8: Raw C double representing the x-axis acceleration in m/s^2  
Bytes 9-16: Raw C double representing the y-axis acceleration in m/s^2  
Bytes 17-24: Raw C double representing the z-axis acceleration in m/s^2  
Bytes 25-32: Raw C double representing the roll in degrees  
Bytes 33-40: Raw C double representing the pitch in degrees  
Bytes 41-48: Raw C double representing the yaw in degrees 

**GPS Packet**  
Byte 0: `3`  
Byte 1: Raw byte representing the number of satellites  
Bytes 2-9: Raw C double representing latitude  
Bytes 10-17: Raw C double representing longitude  
Bytes 18-25: Raw C double representing speed

**RPM Packet**  
Byte 0: `4`  
Bytes 1-8: Raw C double representing the left rear RPM  
Bytes 9-16: Raw C double representing the right rear RPM  
Bytes 17-24: Raw C double representing the left front RPM  
Bytes 25-32: Raw C double representing the right front RPM

**PWM Packet**  
Byte 0: `5`  
Bytes 1-8: Raw C double representing the steering duty cycle  
Bytes 9-16: Raw C double representing the throttle duty cycle

## Next Steps
PWM and GPS still need to be fixed/implemented.

Code for sampling steering and throttle PWM duty cycles exists, but for some reason the interrupt handler is not being called. This may be a hardware problem.

GPS sampling was not implemented due to rime constraints, as it was not properly tested on the Raspberry Pi. However, code for testing the GPS exists in the `Sensor Testing` directory and, upon success, should be integrated into `daq.c` very easily.
