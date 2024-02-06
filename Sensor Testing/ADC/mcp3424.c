// Copyright 2024 by Dawson J. Gullickson

#include "mcp3424.h"

#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

const double FULL_SCALE_RANGE = 2.048;

uint8_t bits_per_rate(SampleRate sample_rate) {
    switch (sample_rate) {
    case HZ240: return 12;
    case HZ60: return 14;
    case HZ15: return 16;
    case HZ3_75: return 18;
    }
    return 0; //NOTE Unreachable
}

uint8_t construct_config_byte(Config config) {
    uint8_t byte = 0;
    byte |= ((config.ready) ? 1:0) << 7;
    byte |= config.channel << 5;
    byte |= ((config.continuous_mode) ? 1:0) << 4;
    byte |= config.sample_rate << 2;
    byte |= config.gain;
    return byte;
}

Config parse_config_byte(uint8_t byte) {
    Config config;
    config.ready = (byte >> 7) == 1;
    config.channel = ((byte >> 5) % 4);
    config.continuous_mode = (byte >> 4) % 2 == 1;
    config.sample_rate = (byte >> 2) % 4;
    config.gain = byte % 4;
    return config;
}


uint8_t construct_address_byte(uint8_t address) {
    uint8_t byte = 0b1101 << 3; // Set by manufacturer
    byte |= address;
    return byte;
}

int _ADC_set_slave_addr(ADC adc) {
    uint8_t addr = construct_address_byte(adc.address);
    return ioctl(adc.bus, I2C_SLAVE, addr);
}

int ADC_init(ADC *adc, const char *i2c_bus, uint8_t address) {
    // Open bus
    int bus = open(i2c_bus, O_RDWR);
    if (bus < 0) return bus; // Error
    
    // Initialize
    adc->bus = bus;
    adc->address = address;

    return 0;
}

int ADC_configure(ADC adc) {
    uint8_t config = construct_config_byte(adc.config);
    uint8_t buffer[1] = { config };
    if (_ADC_set_slave_addr(adc) < 0) return -1;
    if (write(adc.bus, buffer, 1) != 1) return -2;
    return 0;
}

int ADC_read(ADC adc, double *value) {
    // Read 3 bytes from ADC
    uint8_t buffer[3];
    if (_ADC_set_slave_addr(adc) < 0) return -1;
    if (read(adc.bus, buffer, 3) != 3) return -2;

    // Get bits
    uint8_t bits;
    if (adc.config.continuous_mode)
        bits = bits_per_rate(adc.config.sample_rate);
    else
        bits = 18;
    
    // Combine bytes
    uint32_t data = buffer[2];
    data |= ((uint32_t)buffer[1]) << 8;
    data |= ((uint32_t)buffer[0]) << 16;
    if (bits == 18) data <<= 8;

    // Convert to voltage
    uint8_t gain = 1 << adc.config.gain;
    int32_t data_signed = ~data + 1;
    double LSB = FULL_SCALE_RANGE / (1 << (uint32_t)(bits - 1));
    *value = (double)data_signed * (LSB / gain);

    return 0;
}