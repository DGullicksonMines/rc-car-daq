// Copyright 2024 by Dawson J. Gullickson

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "gpio.h"

bool interrupted = false;
void sigint_handler(int signum) {
	(void)(signum); // Suppress unused-parameter
	interrupted = true;
}

double millis() {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	double ms = (double)ts.tv_sec * 1000;
	ms += (double)ts.tv_nsec / 1e6;
	return ms;
}

#define NUM_INTERRUPTS 4
#define NUM_TIMES 5
const uint8_t pins[NUM_INTERRUPTS] = {17, 27, 22, 10};
double times[NUM_INTERRUPTS][NUM_TIMES];
bool all_valid[NUM_INTERRUPTS] = {0};
ssize_t cur_idx[NUM_INTERRUPTS] = {0};

void generic_interrupt_handler(
	const uint8_t pin,
	double *const times,
	bool *const all_valid,
	ssize_t *const cur_idx
) {
	// Get current time
	const double cur_time = millis(); //TODO figure out accuracy
	// Print RPM
	const ssize_t first_idx = (*all_valid) ? *cur_idx : 0;
	const ssize_t prev_idx = (*cur_idx - 1 + NUM_TIMES) % NUM_TIMES;
	const double first_time = times[first_idx];
	const double prev_time = times[prev_idx];
	const ssize_t num_valid = (*all_valid) ? NUM_TIMES : *cur_idx;
	const double to_rpm = 1000.0 * 60.0 / 5.0; // interrupts/ms to rpm
	const double full_rpm = to_rpm * num_valid / (cur_time - first_time);
	const double single_rpm = to_rpm / (cur_time - prev_time);
	printf("%d,%ld,%f,%f,%f \n", pin, num_valid, cur_time, single_rpm, full_rpm);
	// Record time
	times[*cur_idx] = cur_time;
	// Increment
	*cur_idx = (*cur_idx + 1) % NUM_TIMES;
	if (*cur_idx == 0) *all_valid = true;
}

void generic_interrupt_handler_wrapper(
	const uint8_t pin,
	const ssize_t interrupt
) {
	generic_interrupt_handler(
		pin,
		times[interrupt],
		&all_valid[interrupt],
		&cur_idx[interrupt]
	);
}

#define interrupt_handler(i) void interrupt_handler_##i() { \
	generic_interrupt_handler_wrapper(pins[i], i); \
}
interrupt_handler(0)
interrupt_handler(1)
interrupt_handler(2)
interrupt_handler(3)
void (* const interrupt_handlers[NUM_INTERRUPTS])(void) = {
	&interrupt_handler_0,
	&interrupt_handler_1,
	&interrupt_handler_2,
	&interrupt_handler_3
};

int main() {
	signal(SIGINT, sigint_handler);

	// Setup pin interrupts
	PinInterrupt pin_interrupts[NUM_INTERRUPTS];
	for (ssize_t i = 0; i < NUM_INTERRUPTS; i += 1) {
		pin_interrupts[i].pin = pins[i];
		pin_interrupts[i].edge = EdgeTypeFalling;
		pin_interrupts[i].bias = BiasPullUp;
		pin_interrupts[i].interrupt = interrupt_handlers[i];
	}

	// Print header
	printf("Pin,Valid Samples,Time (ms),Single Sample RPM,RPM \n");

	// Begin interrupt polling
	Handle handle;
	int result = begin_interrupt_polling(pin_interrupts, NUM_INTERRUPTS, &handle);
	if (result < 0) {
		printf("error %d: beginning polling \n", result);
		return -1;
	}

	// Idle until interrupted
	while (!interrupted);

	// End interrupt polling
	result = end_interrupt_polling(&handle);
	if (result < 0) {
		printf("error %d: ending polling \n", result);
		return -2;
	}
}
