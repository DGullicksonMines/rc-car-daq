import mcp3424

import smbus

# Initialize I2C bus
BUS_NUMBER = 1 # Run `i2cdetect -l` for available busses.
bus = smbus.SMBus(BUS_NUMBER)

#TODO

# Close I2C bus
bus.close()