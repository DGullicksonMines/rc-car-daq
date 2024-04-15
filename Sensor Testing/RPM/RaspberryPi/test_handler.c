// Copyright 2024 by Dawson J. Gullickson

#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>

#include "gpio.h"

bool interrupted = false;
void sigint_handler(int signum) {
	(void)(signum); // Suppress unused-parameter
	interrupted = true;
}

double target;

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
	printf("%d,%ld,%f,%f,%f,%f \n", pin, num_valid, cur_time, single_rpm, full_rpm, target);
	// Record time
	times[*cur_idx] = cur_time;
	// Increment
	*cur_idx = (*cur_idx + 1) % NUM_TIMES;
	if (*cur_idx == 0) *all_valid = true;
}

void test_handler_rand() {
	srand(time(NULL));
	int i = 0;
	double time;
	struct timespec sleep_time;
	while (!interrupted) {
		if (i == 0) {
			target = 10.0 + 2000.0 * ((double)rand() / RAND_MAX);
			// s/interrupt = min/rot * s/min * interrupts/rot
			time = (60.0 / 5.0) / target;
			sleep_time.tv_sec = (time_t)time;
			sleep_time.tv_nsec = (long)(fmod(time, 1.0) * 1e9);
		}
		i = (i + 1) % 100;
		generic_interrupt_handler(0, times[0], &all_valid[0], &cur_idx[0]);
		nanosleep(&sleep_time, NULL);
	}
}

int main() {
	signal(SIGINT, sigint_handler);

	printf("Pin,Valid Samples,Time (ms),Single Sample RPM,RPM,Target RPM \n");
	test_handler_rand();
}
