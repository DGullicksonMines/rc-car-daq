# Copyright (C) 2023 Dawson J. Gullickson All rights reserved.

from typing import Union

from enum import Enum

import busio # type: ignore

# ---== THE MCP3424 DATASHEET ==---
# https://www.mouser.com/datasheet/2/268/22088c-3136953.pdf
# Section 5.2: Configuration Register
# Section 5.3 I2C Serial Communications
# Figure 6.1: I2C Connection Diagram

FSR = 2.048
"""Full Scale Range"""

class SampleRate(Enum):
    """Available Sample Rates"""
    _240 = 0
    _60 = 1
    _15 = 2
    _3_75 = 3
    
BITS_PER_RATE = {
    SampleRate._240: 12,
    SampleRate._60: 14,
    SampleRate._15: 16,
    SampleRate._3_75: 18
}
"""Data Rates Per Sampling Rate"""

ConfigTuple = tuple[int, bool, bool, SampleRate, int]

def construct_config_byte(
    channel: int,
    ready: bool,
    continuous_mode: bool,
    sample_rate: SampleRate,
    gain: int
) -> int:
    """Constructs a configuration byte.
    
    :param bool ready:
    :param int channel: Channel to select, in the range [1,4]
    :param bool continuous_mode:
    :param SampleRate sample_rate:
    :param int gain: log2 of the desired gain, in the range [0,3].
    """
    if channel < 1 or 4 < channel:
        raise ValueError("Channel must be an integer in the range [1,4].")
    if gain < 0 or 3 < gain:
        raise ValueError("Gain must be an integer in the range [0,3].")
    
    config = ready << 7
    config += (channel - 1) << 5
    config += continuous_mode << 4
    config += sample_rate.value << 2
    config += gain
    return config

def parse_config_byte(config: int) -> ConfigTuple:
    """Parses a configuration byte."""
    channel = ((config >> 5) % 4) + 1
    ready = config >> 7 == 1
    continuous_mode = (config >> 4) % 2 == 1
    sample_rate = SampleRate((config >> 2) % 4)
    gain = config % 4

    return channel, ready, continuous_mode, sample_rate, gain

def update_config_byte(
    config: int,
    channel: Union[int, None] = None,
    ready: Union[bool, None] = None,
    continuous_mode: Union[bool, None] = None,
    sample_rate: Union[SampleRate, None] = None,
    gain: Union[int, None] = None
) -> int:
    """Updates a configuration byte."""
    (
        channel_old,
        ready_old,
        continuous_mode_old,
        sample_rate_old,
        gain_old
    ) = parse_config_byte(config)

    channel = channel if channel is not None else channel_old
    ready = ready if ready is not None else ready_old
    continuous_mode = continuous_mode if continuous_mode is not None else continuous_mode_old
    sample_rate = sample_rate if sample_rate is not None else sample_rate_old
    gain = gain if gain is not None else gain_old
    
    config_updated = construct_config_byte(
        channel, ready, continuous_mode, sample_rate, gain
    )
    return config_updated

def construct_address_byte(address: int) -> int:
    """Constructs an MCP3424 address byte."""
    if address < 0 or 7 < address:
        raise ValueError("Address must be an integer in the range [0,7].")
    
    addr = 0b1101 << 3 # Set by manufacturer
    addr += address
    return addr

class ADC:
    bus: busio.I2C
    address: int
    config: int

    def __init__(
        self,
        bus: busio.I2C,
        address: int,
        channel: int = 1,
        ready: bool = False,
        continuous_mode: bool = True,
        sample_rate: SampleRate = SampleRate._240,
        gain: int = 0
    ) -> None:
        self.bus = bus
        self.address = address
        self.config = construct_config_byte(
            channel, ready, continuous_mode, sample_rate, gain
        )

    def update_config(
        self,
        channel: Union[int, None] = None,
        ready: Union[bool, None] = None,
        continuous_mode: Union[bool, None] = None,
        sample_rate: Union[SampleRate, None] = None,
        gain: Union[int, None] = None
    ) -> None:
        self.config = update_config_byte(
            self.config, channel, ready, continuous_mode, sample_rate, gain
        )

    def configure(self) -> None:
        addr = construct_address_byte(self.address)
        
        try:
            assert self.bus.try_lock()
            self.bus.write_byte(addr, self.config)
        except AssertionError as e: raise e
        finally: self.bus.unlock()

    def update_and_configure(
        self,
        channel: Union[int, None] = None,
        ready: Union[bool, None] = None,
        continuous_mode: Union[bool, None] = None,
        sample_rate: Union[SampleRate, None] = None,
        gain: Union[int, None] = None
    ) -> None:
        self.update_config(
            channel, ready, continuous_mode, sample_rate, gain
        )
        self.configure()
        
    def read(self) -> float:
        addr = construct_address_byte(self.address)

        # Read 3 bytes from ADC
        buffer = bytearray(3)
        try:
            assert self.bus.try_lock()
            self.bus.readfrom_into(addr, buffer)
        except AssertionError as e: raise e
        finally: self.bus.unlock()
        data = int.from_bytes(buffer, byteorder="big")

        # Convert to voltage
        _, _, _, sample_rate, gain = parse_config_byte(self.config)
        bits = BITS_PER_RATE[sample_rate]
        converted = (data / (1 << bits)) * (FSR / gain)

        return converted