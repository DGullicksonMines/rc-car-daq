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

typedef enum {
    Bits12 = 12,
    Bits14 = 14,
    Bits16 = 16,
    Bits18 = 18,
} BitMode;

typedef enum {
    CH1 = 0,
    CH2 = 1,
    CH3 = 2,
    CH4 = 3,
} Channel;

typedef struct {
    Channel channel; // 2 bits
    bool ready; // 1 bit
    bool continuous_mode; // 1 bit
    SampleRate sample_rate; // 2 bits
    uint8_t gain; // 2 bits, logarithmic
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

void ADC_deinit(ADC adc);

BitMode ADC_bit_mode(ADC adc);

int ADC_configure(ADC adc);

/// @brief Reads a voltage from an ADC into `value`
/// @param adc An ADC struct
/// @param value Pointer voltage is read into
/// @return
/// -2: Failed to set ADC as I2C slave
/// -1: Failed to read from the ADC
///  0: Successfully read an old value from the ADC
///  1: Successfully read a new value from the ADC
int ADC_read(ADC adc, double *value);

#endif
