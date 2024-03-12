#include "rpm.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

int _write_single(
	const char *const file,
	const char *const data,
	const ssize_t len
) {
	const int fd = open(file, O_WRONLY);
	if (fd < 0) return -1;
	if (write(fd, data, len) < 0) return -2;
	if (close(fd) < 0) return -3;
	return 0;
}

int _init_pin(const uint8_t pin, const EdgeType edge) {
	// Convert pin to string
	char pin_str[4];
	ssize_t pin_str_len;
	snprintf(pin_str, 4, "%d", pin);
	if (pin > 99) pin_str_len = 3;
	else if (pin > 9) pin_str_len = 2;
	else pin_str_len = 1;

	char *const prefix = "/sys/class/gpio";
	char path[34];

	// Export pin
	snprintf(path, 34, "%s/export", prefix);
	if (_write_single(path, pin_str, pin_str_len) < 0) return -1;
	// Set direction
	snprintf(path, 34, "%s/gpio%s/direction", prefix, pin_str);
	if (_write_single(path, "in", 2) < 0) return -2;
	// Set edge
	snprintf(path, 34, "%s/gpio%s/edge", prefix, pin_str);
	int result;
	switch (edge) {
		case None: result = _write_single(path, "none", 4); break;
		case Falling: result = _write_single(path, "falling", 7); break;
		case Rising: result = _write_single(path, "rising", 6); break;
		case Both: result = _write_single(path, "both", 4); break;
	}
	if (result < 0) return -2;

	// Open GPIO pin
	snprintf(path, 34, "%s/gpio%s/value", prefix, pin_str);
	const int gpio_fd = open(path, O_RDONLY);
	if (gpio_fd < 0) return -3;
	return gpio_fd;
}

int begin_interrupt_polling(
	PinInterrupt *const interrupts,
	const ssize_t num_interrupts
) {
	struct pollfd *poll_fds = (
		(struct pollfd *)malloc(num_interrupts * sizeof(struct pollfd))
	);
	if (poll_fds == NULL) return -1;

	// Initialize pins
	for (ssize_t i = 0; i < num_interrupts; i += 1) {
		int result = _init_pin(interrupts[i].pin, interrupts[i].edge);
		if (result < 0) return -2;
		poll_fds[i].fd = result;
		poll_fds[i].events = POLLPRI;
	}

	// Create polling thread
	//TODO put this in a thread
	while (true) {
		// Poll for interrupt
		const int poll_result = poll(poll_fds, num_interrupts, -1);
		if (poll_result < 0) return -3; // Poll error
		if (poll_result == 0) return -4; // Unreachable; timed out

		for (ssize_t i = 0; i < num_interrupts; i += 1) {
			if (poll_fds[i].revents == 0) continue;
			if ((poll_fds[i].revents & POLLPRI) == 0) return -5; // No POLLPRI event
			// Call interrupt
			interrupts[i].interrupt();
			// Clear interrupt with dummy read
			uint8_t buf;
			(void)read(poll_fds[i].fd, &buf, 1);
		}
	}

	// Deinitialize pins
	for (ssize_t i = 0; i < num_interrupts; i += 1)
		if (close(poll_fds[i].fd) < 0) return -2;

	free(poll_fds);
	return 0;
}

int end_interrupt_polling() {
	return 0;
}
