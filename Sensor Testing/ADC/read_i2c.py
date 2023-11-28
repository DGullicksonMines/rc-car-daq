#! /usr/bin/env python

import board # type: ignore
import busio # type: ignore

import mcp3424

# Initialize I2C bus at the standard mode rate
bus = busio.I2C(board.SCL, board.SDA, frequency=100000)

#TODO

# Close I2C bus
bus.deinit()