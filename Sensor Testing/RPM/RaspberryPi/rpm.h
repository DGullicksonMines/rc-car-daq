// Copyright 2024 by Dawson J. Gullickson

#ifndef rpm_H
#define rpm_H

#include <poll.h>
#include <stdint.h>
#include <unistd.h>

typedef enum {
	None,
	Falling,
	Rising,
	Both,
} EdgeType;

typedef struct {
	uint8_t pin;
	EdgeType edge;
	int (*interrupt)(void);
	struct pollfd _poll_fd;
} PinInterrupt;

int begin_interrupt_polling(
	PinInterrupt *const interrupts,
	const ssize_t num_interrupts
);

int end_interrupt_polling();

#endif
