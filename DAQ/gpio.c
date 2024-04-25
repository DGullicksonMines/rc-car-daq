// Copyright 2024 by Dawson J. Gullickson

// https://www.kernel.org/doc/html/next/userspace-api/gpio/chardev.html

#include "gpio.h"

#include <stdio.h> //TODO remove with printf()s
#include <errno.h>

#include <stddef.h> // size_t
#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h> // open(), O_RDONLY
#include <linux/gpio.h> // gpio_v2_line_event, gpio_v2_line_config, gpio_v2_line_attribute, gpio_v2_line_config_attribute, gpio_v2_line_request, GPIO_V2_LINE_FLAG_INPUT, GPIO_V2_LINE_NUM_ATTRS_MAX, GPIO_V2_LINE_FLAG_EDGE_FALLING, GPIO_V2_LINE_FLAG_EDGE_RISING, GPIO_V2_LINE_FLAG_BIAS_PULL_UP, GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN, GPIO_V2_LINE_ATTR_ID_FLAGS, GPIO_V2_LINES_MAX, GPIO_V2_GET_LINE_IOCTL, GPIO_V2_LINE_GET_VALUES_IOCTL
#include <poll.h> // pollfd, poll(), POLL_IN
#include <pthread.h> // pthread_mutex_t, pthread_t, pthread_mutex_init(), pthread_mutex_destroy(), pthread_mutex_lock(), pthread_mutex_trylock(), pthread_mutex_unlock(), pthread_create(), pthread_join()
#include <sys/ioctl.h> // ioctl()
#include <unistd.h> // read(), close()

//NOTE The RPi4B uses gpiochip0 and the RPi5 uses gpiochip5
//FIXME This fact shouldn't be hidden here
#define GPIO_CHIP "/dev/gpiochip4"

typedef struct {
	const PinInterrupt *interrupts;
	size_t num_interrupts;
	struct pollfd poll_fd;
	pthread_mutex_t *canceled;
	bool received;
} _ThreadArgs;

void *_polling(void *args) {
	// Unpackage args
	_ThreadArgs *thread_args = (_ThreadArgs *)args;
	const PinInterrupt *const interrupts = thread_args->interrupts;
	const size_t num_interrupts = thread_args->num_interrupts;
	struct pollfd poll_fd = thread_args->poll_fd;
	pthread_mutex_t *canceled = thread_args->canceled;
	thread_args->received = true;

	// Loop until cancelled
	while (pthread_mutex_trylock(canceled) != 0) {
		// Poll for interrupt
		const int poll_result = poll(&poll_fd, 1, 1000);
		if (poll_result < 0) return (void *)-1; // Poll error
		if (poll_result == 0) continue; // Timed out
		if ((poll_fd.revents & POLLIN) == 0) return (void *)-2; // No POLLIN event
		// Read line event
		struct gpio_v2_line_event event;
		if (read(poll_fd.fd, &event, sizeof(struct gpio_v2_line_event)) <= 0)
			return (void *)-3;
		// Read line value
		//XXX will this slow us down significantly?
		// struct gpio_v2_line_values values = {
		// 	.bits = 0,
		// 	.mask = 1 << event.offset,
		// };
		// int res = ioctl(poll_fd.fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &values);
		// printf("res: %d, err: %d \n", res, errno);
		// if (res < 0)
		// 	return (void *)-4;
		// bool active = (values.bits & (1 << event.offset)) > 0;
		bool active = false;
		// Call interrupt
		//XXX what if we don't call an interrupt here?
		for (size_t i = 0; i < num_interrupts; i += 1) {
			if (interrupts[i].pin != event.offset) continue;
			interrupts[i].interrupt(event.timestamp_ns, active);
			break;
		}
	}

	if (pthread_mutex_unlock(canceled) < 0) return (void *)-5;
	return (void *)0;
}

int begin_interrupt_polling(
	const PinInterrupt *const interrupts,
	const size_t num_interrupts,
	Handle *const handle
) {
	// Open GPIO Chip
	const int chip_fd = open(GPIO_CHIP, O_RDONLY);
	if (chip_fd < 0) return -1;
	handle->chip_fd = chip_fd;

	// Create main configuration
	uint64_t default_flags = GPIO_V2_LINE_FLAG_INPUT;
	struct gpio_v2_line_config line_cfg = {
		.flags = default_flags,
		.num_attrs = 0,
		.padding = {0},
	};
	// Create line configurations
	if (num_interrupts > GPIO_V2_LINE_NUM_ATTRS_MAX) return -2;
	for (size_t i = 0; i < num_interrupts; i += 1) {
		uint64_t flags = default_flags;
		// Set edge
		switch (interrupts[i].edge) {
		case EdgeTypeNone: break;
		case EdgeTypeFalling: flags |= GPIO_V2_LINE_FLAG_EDGE_FALLING; break;
		case EdgeTypeRising: flags |= GPIO_V2_LINE_FLAG_EDGE_RISING; break;
		case EdgeTypeBoth:
			flags |= GPIO_V2_LINE_FLAG_EDGE_FALLING;
			flags |= GPIO_V2_LINE_FLAG_EDGE_RISING;
			break;
		}
		// Set bias
		switch (interrupts[i].bias) {
		case BiasNone: break;
		case BiasPullUp: flags |= GPIO_V2_LINE_FLAG_BIAS_PULL_UP; break;
		case BiasPullDown: flags |= GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN; break;
		}
		if (flags == default_flags) continue;
		// Add attribute
		struct gpio_v2_line_attribute attr = {
			.id = GPIO_V2_LINE_ATTR_ID_FLAGS,
			.padding = 0,
			.flags = flags,
		};
		struct gpio_v2_line_config_attribute attribute = {
			.attr = attr,
			.mask = (uint64_t)1 << i,
		};
		line_cfg.attrs[line_cfg.num_attrs] = attribute;
		line_cfg.num_attrs += 1;
	}
	// Open GPIO Lines
	struct gpio_v2_line_request line_req = {
		.consumer = "interrupt-poller",
		.config = line_cfg,
		.num_lines = num_interrupts,
		.event_buffer_size = 0, // Let Kernel choose
		.padding = {0},
	};
	if (num_interrupts > GPIO_V2_LINES_MAX) return -3;
	for (size_t i = 0; i < num_interrupts; i += 1)
		line_req.offsets[i] = (uint32_t)interrupts[i].pin;
	if (ioctl(chip_fd, GPIO_V2_GET_LINE_IOCTL, &line_req) < 0) return -4;

	// Initialize polling
	const struct pollfd poll_fd = {
		.fd = line_req.fd,
		.events = POLLIN,
	};

	// Create polling thread
	if (pthread_mutex_init(&handle->canceled, NULL) < 0) return -5;
	if (pthread_mutex_lock(&handle->canceled) < 0) return -6;
	_ThreadArgs args = {
		.interrupts = interrupts,
		.num_interrupts = num_interrupts,
		.poll_fd = poll_fd,
		.canceled = &handle->canceled,
		.received = false,
	};
	int res = pthread_create(&handle->thread, NULL, _polling, (void *)&args);
	if (res < 0) return -7;
	// Wait for arguments to be received
	while (!args.received);

	return 0;
}

int end_interrupt_polling(Handle *const handle) {
	// Cancel thread
	if (pthread_mutex_unlock(&handle->canceled) < 0) return -1;
	// Join thread
	void *ret;
	if (pthread_join(handle->thread, (void **)&ret) < 0) return -2;
	if ((intptr_t)ret < 0) return -3;
	// Destroy mutex
	if (pthread_mutex_destroy(&handle->canceled) < 0) return -4;
	// Close file
	if (close(handle->chip_fd) < 0) return -5;

	return 0;
}
