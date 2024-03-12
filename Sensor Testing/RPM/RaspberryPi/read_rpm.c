// Copyright 2024 by Dawson J. Gullickson

#include <stdbool.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>

#include "rpm.h"

#define RPM_PIN "24"

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

#define NUM_TIMES 64
double times[NUM_TIMES];
bool all_valid = false;
ssize_t cur_idx = 0;

int interrupt_handler() {
	// Record time
	times[cur_idx] = millis(); //TODO figure out accuracy
	// Print RPM
	const ssize_t first_idx = (all_valid) ? (cur_idx + 1) % NUM_TIMES : 0;
	const ssize_t prev_idx = (cur_idx - 1 + NUM_TIMES) % NUM_TIMES;
	const double first_time = times[first_idx];
	const double prev_time = times[prev_idx];
	const double cur_time = times[cur_idx];
	const ssize_t num_valid = (all_valid) ? NUM_TIMES : cur_idx;
	const double to_rpm = 1000.0 * 60.0 / 5.0; // magnets/ms to rpm
	const double full_rpm = to_rpm * num_valid / (cur_time - first_time);
	const double single_rpm = to_rpm / (cur_time - prev_time);
	printf("RPM: %f (%ld samples) \n", full_rpm, num_valid);
	printf("RPM: %f (1 sample) \n", single_rpm);
	// Increment
	cur_idx = (cur_idx + 1) % NUM_TIMES;
	if (cur_idx == 0) all_valid = true;

	return 0;
}

int main() {
	signal(SIGINT, sigint_handler);

	PinInterrupt pin_interrupts[1];
	pin_interrupts[0].pin = 24;
	pin_interrupts[0].edge = Rising;
	pin_interrupts[0].interrupt = &interrupt_handler;

	begin_interrupt_polling(pin_interrupts, 1);

	while (!interrupted);

	end_interrupt_polling();
}
