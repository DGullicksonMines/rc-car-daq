// Copyright 2024 by Dawson J. Gullickson

#include "jy901.h"

#include <stdbool.h>
#include <stdio.h>
#include <signal.h>
#include <math.h>
#include <time.h>

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

int main() {
	signal(SIGINT, sigint_handler);

    // Initialize IMU
    char *i2c_bus = "/dev/i2c-1";
    IMU imu;
    int init_result = IMU_init(&imu, i2c_bus);
    if (init_result < 0) {
        fprintf(stderr, "error %d: could not initialize IMU \n", init_result);
        return -3;
    }

	// Loop until interrupted
	int result;
	double acceleration[3];
	double angle[3];
	double vel_x = 0.0, vel_y = 0.0, vel_z = 0.0;
	double time = millis();
	uint8_t loop = 0;
	const uint8_t frequency = 100;
	double loops_start = time;
	while (!interrupted) {
		// Read acceleration and angle
		result = IMU_read_acceleration(imu, acceleration);
		if (result < 0) {
			fprintf(stderr, "error %d: could not read acceleration \n", result);
			return -2;
		}
		result = IMU_read_angle(imu, angle);
		if (result < 0) {
			fprintf(stderr, "error %d: could not read angle \n", result);
			return -3;
		}
		// Calculate corrected acceleration
		double acc_x = acceleration[0],
			acc_y = acceleration[1],
			acc_z = acceleration[2],
			roll = angle[0],
			pitch = angle[1],
			yaw = angle[2];
		double acc_x_corr = (
			acc_x * cos(pitch * M_PI / 180)
			+ acc_z * sin(pitch * M_PI / 180)
		);
		double acc_y_corr = (
			acc_y * cos(roll * M_PI / 180)
			- acc_z * sin(roll * M_PI / 180)
		);
		double acc_z_corr = (
			-acc_z * cos(roll * M_PI / 180) * cos(pitch * M_PI / 180)
			- acc_y * sin(roll * M_PI / 180)
			+ acc_x * sin(pitch * M_PI / 180)
		);
		// Calculate time difference
		double old_time = time;
		time = millis();
		double dt = time - old_time;
		// Integrate velocity
		vel_x += acc_x_corr * dt / 1000;
		vel_y += acc_y_corr * dt / 1000;
		vel_z += acc_z_corr * dt / 1000;
		// Print everything
		if (loop == frequency) {
			double aggregate_time = time - loops_start;
			loops_start = time;
			printf(
				"Acc (m/s^2): %f %f %f | "
				"Acc Corr (m/s^2) %f %f %f | "
				"RPY (degrees): %f %f %f | "
				"dt (ms): %f | %d loops (ms): %f | "
				"Vel (m/s): %f %f %f \n",
				acc_x, acc_y, acc_z,
				acc_x_corr, acc_y_corr, acc_z_corr,
				roll, pitch, yaw,
				dt, aggregate_time,
				vel_x, vel_y, vel_z
			);
		}
		loop += 1;
	}

    // Deinitialize IMU
    IMU_deinit(imu);
}
