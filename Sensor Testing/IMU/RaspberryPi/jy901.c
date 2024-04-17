// Copyright 2024 by Dawson J. Gullickson

#include "jy901.h"

#include <stddef.h> // size_t
#include <stdint.h>
#include <fcntl.h> // open(), O_RDWR
#include <linux/i2c.h> // i2c_msg, I2C_M_READ
#include <linux/i2c-dev.h> // i2c_rdwr_ioctl_data, I2C_RDWR
#include <sys/ioctl.h> // ioctl()
#include <unistd.h> // close()

const uint8_t REG_ACC = 0x34;
const uint8_t REG_ANG = 0x3d;

int IMU_init(IMU *imu, const char *i2c_bus) {
    // Open bus
    int bus = open(i2c_bus, O_RDWR);
    if (bus < 0) return bus; // Error
    
    // Initialize
	imu->bus = bus;
	imu->address = 0x50;

	return 0;
}

int IMU_deinit(IMU imu) {
	return close(imu.bus);
}

int _IMU_read_vec(IMU imu, uint8_t reg, int16_t data[3]) {
	uint8_t buffer[6];
	struct i2c_msg msgs[2];
	// Create write message
	msgs[0].addr = imu.address;
	msgs[0].flags = 0;
	msgs[0].len = 1;
	msgs[0].buf = &reg;
	// Create read message
	msgs[1].addr = imu.address;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = 6;
	msgs[1].buf = buffer;

	// Create transaction
	struct i2c_rdwr_ioctl_data transaction;
	transaction.msgs = msgs;
	transaction.nmsgs = 2;
	// Perform transaction
	int res = ioctl(imu.bus, I2C_RDWR, &transaction);
	if (res < 0) return res;

	// Combine data bytes
	for (size_t i = 0; i < 3; i += 1)
		data[i] = ((int16_t)buffer[2 * i + 1] << 8) | buffer[2 * i];
	
	return 0;
}

int IMU_read_acceleration(IMU imu, double acceleration[3]) {
	int16_t data[3];
	int res = _IMU_read_vec(imu, REG_ACC, data);
	if (res < 0) return res;
	for (size_t i = 0; i < 3; i += 1)
		acceleration[i] = ((double)data[i] / 32768.0) * 16.0 * 9.81;
	return 0;
}

int IMU_read_angle(IMU imu, double angle[3]) {
	int16_t data[3];
	int res = _IMU_read_vec(imu, REG_ANG, data);
	if (res < 0) return res;
	for (size_t i = 0; i < 3; i += 1)
		angle[i] = ((double)data[i] / 32768.0) * 180.0;
	return 0;
}
