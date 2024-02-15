// Copyright 2024 by Dawson J. Gullickson

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>

#include "mcp3424.h"

//XXX No error is returned on overflow
int16_t read_int(const char *prompt) {
    uint8_t val = 0;
    if (fputs(prompt, stdout) == EOF) {
        fputs("error: could not show prompt (fputs). \n", stderr);
        return -1;
    }
    while (true) {
        uint8_t chr = fgetc(stdin);
        if (chr == EOF) {
            fputs("error: could not read character (fgetc). \n", stderr);
            return -2;
        }
        if (chr == '\n') break;
        if (chr < '0' || '9' < chr) {
            fputs("error: invalid character. \n", stderr);
            return -3;
        }
        val = val * 10 + (chr - '0');
    }
    return val;
}

int probe_loop(ADC adc, double divider_ratio) {
    // Prompt channel
    uint16_t channel = read_int("Channel: ");
    if (channel < 1 || 4 < channel) {
        fprintf(stderr, "error: invalid channel \"%d\" \n", channel);
        return -1;
    }
    adc.config.channel = channel - 1;
    adc.config.ready = true;
    ADC_configure(adc);

    // Get voltage
    double voltage;
    int read_result = ADC_read(adc, &voltage);
    if (read_result < 0) {
        fprintf(stderr, "error %d: could not read from ADC \n", read_result);
        return -2;
    }
    double voltage_undivided = voltage / divider_ratio;
    printf("Read %f V (%f V undivided) \n\n", voltage, voltage_undivided);

    return 0;
}

int main() {
    // Get voltage divider ratio
    double divider_ratio;
    int16_t r1 = read_int("Resistor 1 Value (int): ");
    if (r1 < 0) {
        fprintf(stderr, "error %d: could not get resistor 1 value \n", r1);
        return -1;
    }
    if (r1 != 0) {
        int16_t r2 = read_int("Resistor 2 Value (int): ");
        if (r1 < 0) {
            fprintf(stderr, "error %d: could not get resistor 2 value \n", r1);
            return -1;
        }
        divider_ratio = (double)r2 / ((double)r1 + r2);
    } else {
        divider_ratio = 1;
    }

    int16_t address = read_int("ADC Address: ");
    if (address < 0) {
        fprintf(stderr, "error %d: could not get address \n", address);
        return -1;
    }

    // Initialize ADC
    char *i2c_bus = "/dev/i2c-1";
    ADC adc;
    int init_result = ADC_init(&adc, i2c_bus, address);
    if (init_result < 0) {
        fprintf(stderr, "error %d: could not initialize ADC \n", init_result);
        return -2;
    }
    adc.config.channel = CH1;
    adc.config.ready = false;
    adc.config.continuous_mode = false;
    adc.config.sample_rate = HZ240;
    adc.config.gain = 0;
    ADC_configure(adc);

    // Loop until no channel entered
    int loop_result;
    while ((loop_result = probe_loop(adc, divider_ratio)) >= 0);
    if (loop_result != -1) return -3;

    // Deinitialize ADC
    ADC_deinit(adc);
}