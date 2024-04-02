// Copyright 2024 by Dawson J. Gullickson

// Simple library for reacting to GPIO events

#ifndef gpio_H
#define gpio_H

#include <stdint.h>
#include <unistd.h>
#include <poll.h>
#include <pthread.h>

typedef enum {
	EdgeTypeNone,
	EdgeTypeFalling,
	EdgeTypeRising,
	EdgeTypeBoth,
} EdgeType;

typedef enum {
	BiasNone,
	BiasPullUp,
	BiasPullDown,
} Bias;

typedef struct {
	uint8_t pin;
	EdgeType edge;
	Bias bias;
	int (*interrupt)(void);
} PinInterrupt;

typedef struct {
	pthread_mutex_t canceled;
	pthread_t thread;
	int chip_fd;
} Handle;

int begin_interrupt_polling(
	PinInterrupt *const interrupts,
	const ssize_t num_interrupts,
	Handle *handle
);

int end_interrupt_polling(Handle *handle);

#endif
