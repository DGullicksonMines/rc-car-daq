// Copyright 2024 by Dawson J. Gullickson

#ifndef jy901_H
#define jy901_H

#include <stdint.h>

typedef struct {
    int bus;
	uint8_t address;
} IMU;

int IMU_init(IMU *imu, const char *i2c_bus);

void IMU_deinit(IMU imu);

int IMU_read_acceleration(IMU imu, double acceleration[3]);

int IMU_read_angle(IMU imu, double angle[3]);

#endif
