// Copyright 2024 by Dawson J. Gullickson

// Simple Library for the JY901 9-Axis IMU

#ifndef JY901_H
#define JY901_H

#include <stdint.h>

typedef struct {
    int bus;
	uint8_t address;
} IMU;

/// @brief Initializes an IMU connection
/// @param adc Uninitialized `IMU` struct
/// @return 0 on success
int IMU_init(IMU *imu, const char *i2c_bus);

/// @brief Deinitializes an IMU connection
/// @param adc Initialized `IMU` struct
/// @return 0 on success
int IMU_deinit(IMU imu);

/// @brief Reads acceleration from an IMU
/// @param adc Initialized `IMU` struct
/// @return 0 on success
int IMU_read_acceleration(IMU imu, double acceleration[3]);

/// @brief Reads angle from an IMU
/// @param adc Initialized `IMU` struct
/// @return 0 on success
int IMU_read_angle(IMU imu, double angle[3]);

#endif
