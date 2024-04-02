// Copyright 2024 by Dawson J. Gullickson

#include "rpm.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/gpio.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define GPIO_CHIP "/dev/gpiochip0"

int begin_interrupt_polling(
	PinInterrupt *const interrupts,
	const ssize_t num_interrupts
) {
	// Open GPIO Chip
	int chip_fd = open(GPIO_CHIP, O_RDONLY);
	if (chip_fd < 0) return -1;

	// Get pins

	// Open GPIO Lines
	struct gpio_v2_line_config line_cfg = {
		.flags = (
			GPIO_V2_LINE_FLAG_INPUT
			| GPIO_V2_LINE_FLAG_EDGE_FALLING
			| GPIO_V2_LINE_FLAG_BIAS_PULL_UP
		),
		.num_attrs = 0,
		.padding = {0},
	};
	struct gpio_v2_line_request line_req = {
		.consumer = "rpm-poller",
		.config = line_cfg,
		.num_lines = num_interrupts,
		.event_buffer_size = 0, // Let Kernel choose
		.padding = {0},
	};
	for (ssize_t i = 0; i < num_interrupts; i += 1)
		line_req.offsets[i] = (unsigned int)interrupts[i].pin;
	if (ioctl(chip_fd, GPIO_V2_GET_LINE_IOCTL, &line_req) < 0) return -2;

	// Initialize polling
	struct pollfd poll_fd = {
		.fd = line_req.fd,
		.events = POLLIN,
	};

	// Create polling thread
	//TODO put this in a thread
	while (true) {
		// Poll for interrupt
		const int poll_result = poll(&poll_fd, 1, -1);
		if (poll_result < 0) return -3; // Poll error
		if (poll_result == 0) return -4; // Unreachable; timed out
		if ((poll_fd.revents & POLLIN) == 0) return -5; // No POLLPRI event
		// Read line event
		struct gpio_v2_line_event event;
		if (read(line_req.fd, &event, sizeof(struct gpio_v2_line_event)) <= 0)
			return -6;
		// Call interrupt
		for (ssize_t i = 0; i < num_interrupts; i += 1) {
			if (interrupts[i].pin != event.offset) continue;
			interrupts[i].interrupt();
			break;
		}
	}

	return 0;
}

int end_interrupt_polling() {
	return 0;
}
