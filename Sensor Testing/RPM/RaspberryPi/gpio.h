// Copyright 2024 by Dawson J. Gullickson

// Simple library for reacting to GPIO events

#ifndef gpio_H
#define gpio_H

#include <stddef.h> // size_t
#include <stdint.h>
#include <pthread.h> // pthread_mutex_t, pthread_t

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
	void (*interrupt)(void);
} PinInterrupt;

typedef struct {
	pthread_mutex_t canceled;
	pthread_t thread;
	int chip_fd;
} Handle;

int begin_interrupt_polling(
	const PinInterrupt *const interrupts,
	const size_t num_interrupts,
	Handle *const handle
);

int end_interrupt_polling(Handle *const handle);

#endif
