#! /usr/bin/env python

# Copyright 2023 by Dawson J. Gullickson

import board # type: ignore
import busio # type: ignore

import mcp3424

# Initialize I2C bus at the standard mode rate
bus = busio.I2C(board.SCL, board.SDA, frequency=100000)

# Prompt user for ADC's I2C Address
address = int(input("ADC Address: "))

# ADC initial configuration
adc = mcp3424.ADC(bus, address)
adc.configure()

try:
    # Allow user to probe channel voltages
    while True:
        channel = int(input("Channel: "))
        adc.update_and_configure(channel=channel)
        voltage = adc.read()
        print("Read {} V".format(voltage))
        print()
except KeyboardInterrupt: pass
finally:
    # Close I2C bus
    bus.deinit()