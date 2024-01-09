#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>

#include "mcp3424.h"

int16_t read_int(const char *prompt) {
    uint8_t val = 0;
    if (fputs(prompt, stdout) == EOF) {
        fputs("error: could not show prompt (fputs).", stdout);
        return -1;
    }
    while (true) {
        uint8_t chr = fgetc(stdin);
        if (chr == EOF) {
            fputs("error: could not read character (fgetc).", stdout);
            return -2;
        }
        if (chr == '\n') break;
        val = val * 10 + (chr - '0');
    }
    return val;
}

int probe_loop(ADC adc) {
    // Prompt channel
    uint16_t channel = read_int("Channel: ");
    if (channel == 0) return -1;
    if (channel < 0) return -2;
    adc.config.channel = channel - 1;

    // Get voltage
    double voltage;
    if (ADC_read(adc, &voltage) < 0) return -3;
    printf("Read %d V \n\n", voltage);

    return 0;
}

int main() {
    int16_t address = read_int("ADC Address: ");
    if (address < 0) return -1;

    // Initialize ADC
    char *i2c_bus = "/dev/i2c-2";
    ADC adc;
    if (ADC_init(&adc, i2c_bus, address) < 0) return -2;
    adc.config.channel = 1;
    adc.config.ready = false;
    adc.config.continuous_mode = true;
    adc.config.sample_rate = HZ240;
    adc.config.gain = 0;
    ADC_configure(adc);

    // Loop until no channel entered
    int exit;
    while ((exit = probe_loop) >= 0);
    if (exit != -1) return -3;
}