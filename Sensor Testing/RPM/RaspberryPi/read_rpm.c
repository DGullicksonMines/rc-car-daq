// Copyright 2024 by Dawson J. Gullickson

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

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

int single_write(const char *file, const char *data, ssize_t len) {
	const int fd = open(file, O_WRONLY);
	if (fd < 0) return -1;
	if (write(fd, data, len) < 0) return -2;
	if (close(fd) < 0) return -3;
	return 0;
}

int main() {
	signal(SIGINT, sigint_handler);

	// Export pin
	if (single_write("/sys/class/gpio/export", RPM_PIN, sizeof(RPM_PIN)) < 0)
		return -1;

	// Set direction
	if (single_write("/sys/class/gpio/gpio" RPM_PIN "/direction", "in", 2) < 0)
		return -2;

	// Set edge
	if (single_write("/sys/class/gpio/gpio" RPM_PIN "/edge", "rising", 7) < 0)
		return -3;
	
	// Open GPIO pin
	const int gpio_fd = open("/sys/class/gpio/gpio" RPM_PIN "/value", O_RDONLY);
	if (gpio_fd < 0) return -4;

	// Loop to poll for interrupt
	struct pollfd poll_fd;
	poll_fd.fd = gpio_fd;
	poll_fd.events = POLLPRI;
	const ssize_t num_times = 64;
	double times[num_times];
	bool all_valid = false;
	ssize_t cur_idx = 0;
	while (!interrupted) {
		// Poll for interrupt
		const int poll_result = poll(&poll_fd, 1, -1);
		if (poll_result < 0) return -5; // Poll error
		if (poll_result == 0) return -6; // No fds had an event
		if ((poll_fd.revents & POLLPRI) == 0) return -7; // No POLLPRI event
		// Clear interrupt with dummy read
		uint8_t buf;
		(void)read(gpio_fd, &buf, 1);
		// Record time
		times[cur_idx] = millis();
		// Print RPM
		const ssize_t first_idx = (all_valid) ? (cur_idx + 1) % num_times : 0;
		const double first_time = times[first_idx];
		const ssize_t prev_time = times[(cur_idx - 1 + num_times) % num_times];
		const double cur_time = times[cur_idx];
		const double to_rpm = 1000.0 * 60.0 / 5.0; // magnets/ms to rpm
		const double full_rpm = to_rpm * num_times / (cur_time - first_time);
		const double single_rpm = to_rpm * 12.0e3 / (cur_time - prev_time);
		printf(
			"RPM: %f (%ld samples) \n",
			full_rpm,
			(all_valid) ? num_times : cur_idx
		);
		printf("RPM: %f (1 sample) \n", single_rpm);
		// Increment
		cur_idx = (cur_idx + 1) % num_times;
		if (cur_idx == 0) all_valid = true;
	}

	// Close GPIO pin
	if (close(gpio_fd) < 0) return -8;

	// Unexport pin
	if (single_write("/sys/class/gpio/unexport", RPM_PIN, sizeof(RPM_PIN)) < 0)
		return -9;
}
