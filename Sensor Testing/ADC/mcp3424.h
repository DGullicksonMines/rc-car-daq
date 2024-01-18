// Copyright 2024 by Dawson J. Gullickson

#ifndef MCP3424_H
#define MCP3424_H

#include <stdbool.h>
#include <stdint.h>

extern const double FULL_SCALE_RANGE;

typedef enum {
    HZ240 = 0,
    HZ60 = 1,
    HZ15 = 2,
    HZ3_75 = 3,
} SampleRate;

uint8_t bits_per_rate(SampleRate sample_rate);

typedef enum {
    CH1 = 0,
    CH2 = 1,
    CH3 = 2,
    CH4 = 3,
} Channel;

typedef struct {
    Channel channel; // 2 bits
    bool ready;
    bool continuous_mode;
    SampleRate sample_rate; // 2 bits
    uint8_t gain; // 2 bits
} Config;

uint8_t construct_config_byte(Config config);

Config parse_config_byte(uint8_t byte);

/// @brief Constructs an address byte for the MCP3424.
/// @param address 3-bit address of MCP3424
/// @return An address byte.
uint8_t construct_address_byte(uint8_t address);

typedef struct {
    int bus;
    uint8_t address;
    Config config;
} ADC;

int ADC_init(ADC *adc, const char *i2c_bus, uint8_t address);

int ADC_configure(ADC adc);

int ADC_read(ADC adc, double *value);

#endif