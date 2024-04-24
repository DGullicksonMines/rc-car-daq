// Copyright 2024 by Dawson J. Gullickson

#include "mcp3424.h"

#include <stdint.h>
#include <fcntl.h> // open(), O_RDWR
#include <linux/i2c-dev.h> // I2C_SLAVE
#include <sys/ioctl.h> // ioctl()
#include <unistd.h> // write(), read(), close()

const double FULL_SCALE_RANGE = 2.048;

uint8_t construct_config_byte(Config config) {
    uint8_t byte = 0;
    byte |= ((config.ready) ? 0:1) << 7;
    byte |= config.channel << 5;
    byte |= ((config.continuous_mode) ? 1:0) << 4;
    byte |= config.sample_rate << 2;
    byte |= config.gain;
    return byte;
}

Config parse_config_byte(uint8_t byte) {
    Config config;
    config.ready = (byte >> 7) == 0;
    config.channel = ((byte >> 5) % 4);
    config.continuous_mode = (byte >> 4) % 2 == 1;
    config.sample_rate = (byte >> 2) % 4;
    config.gain = byte % 4;
    return config;
}


uint8_t construct_address_byte(uint8_t address) {
    uint8_t byte = 0xd << 3; // (1101) Set by manufacturer
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

int ADC_deinit(ADC adc) {
    return close(adc.bus);
}

BitMode ADC_bit_mode(ADC adc) {
    if (!adc.config.continuous_mode) return Bits18;
    switch (adc.config.sample_rate) {
    case HZ240: return Bits12;
    case HZ60: return Bits14;
    case HZ15: return Bits16;
    case HZ3_75: return Bits18;
    }
    return -1; // NOTE Unreachable
}

int ADC_configure(ADC adc) {
    uint8_t config = construct_config_byte(adc.config);
    uint8_t buffer[1] = { config };
    if (_ADC_set_slave_addr(adc) < 0) return -1;
    if (write(adc.bus, buffer, 1) != 1) return -2;
    return 0;
}

int ADC_read(ADC adc, double *value) {
    // Get bit mode
    BitMode bit_mode = ADC_bit_mode(adc);

    // Read data bytes and config byte from ADC
    // Data bytes: 3 in 18-bit mode, 2 otherwise
    if (_ADC_set_slave_addr(adc) < 0) return -2;
    uint8_t buffer[4];
    if (bit_mode == Bits18) {
        if (read(adc.bus, buffer, 4) != 4) return -1;
    } else {
        buffer[0] = 0;
        if (read(adc.bus, &buffer[1], 3) != 3) return -1;
    }
    
    // Combine bytes
    uint32_t data = buffer[2];
    data |= ((uint32_t)buffer[1]) << 8;
    data |= ((uint32_t)buffer[0]) << 16;
    data >>= 2; //XXX Unsure why this is necessary
    data &= ((uint32_t)1 << bit_mode) - 1;

    // Convert to voltage
    int32_t data_signed = data & (((uint32_t)1 << bit_mode) - 1);
    data_signed -= (data_signed & ((uint32_t)1 << (bit_mode - 1))) << 1;
    double LSB = FULL_SCALE_RANGE / ((uint32_t)1 << (bit_mode - 1));
    uint8_t gain = (uint32_t)1 << adc.config.gain;
    *value = data_signed * (LSB / gain);

    // Check if sample was up to date
    Config config = parse_config_byte(buffer[3]);
    if (config.ready) return 1;
    return 0;
}

int ADC_is_ready(ADC adc) {
    // Get bit mode
    BitMode bit_mode = ADC_bit_mode(adc);

    // Read data bytes and config byte from ADC
    // Data bytes: 3 in 18-bit mode, 2 otherwise
    if (_ADC_set_slave_addr(adc) < 0) return -2;
    uint8_t buffer[4];
    if (bit_mode == Bits18) {
        if (read(adc.bus, buffer, 4) != 4) return -1;
    } else {
        if (read(adc.bus, &buffer[1], 3) != 3) return -1;
    }

    // Check if sample was up to date
    Config config = parse_config_byte(buffer[3]);
    if (config.ready) return 1;
    return 0;
}
