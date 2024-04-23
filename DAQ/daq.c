#include <stdbool.h>
#include <stdint.h>
#include <math.h> // fmod()
#include <stdio.h> // FILE, fprintf(), fwrite(), fputc(), fopen(), fclose(), stderr
#include <signal.h> // signal(), SIGINT
#include <time.h> // timespec, clock_gettime(), nanosleep(), CLOCK_REALTIME

#include "mcp3424.h"
#include "jy901.h"

// ---== NOTES ==--- ///
// Very little data processing is done here.
// This means:
// - Voltages are not undivided
// - Acceleration is not corrected
// - Duty cycle is given instead of steering angle

// ---== Signal Handler ==--- //

bool interrupted = false;
void sigint_handler(int signum) {
	(void)(signum); // Suppress unused-parameter
	//TODO if already interrupted, can I force the program to exit?
	interrupted = true;
}

// ---== Helpers ==--- //

struct timespec now();
void sleep_remainder(
	struct timespec duration,
	struct timespec from,
	struct timespec now
);

// ---== Settings ==--- //

#define OUT_FILE "data.bin"
#define I2C_BUS "/dev/i2c-1"
const uint8_t adc_low_addr = 0; // 0b000
const uint8_t adc_high_addr = 6; // 0b110
//TODO document max values (based on intervals being uint32)
const double loop_frequency = 200.0; // Frequency of recording data
const double send_frequency = 10.0;  // Frequency of sending data
const double ADC_frequency = 1.0;    // Frequency of ADC sampling
const double IMU_frequency = 200.0;  // Frequency of IMU sampling

// ---== Recording/Sending ==--- //

typedef struct {
	uint8_t satellites;
	double lat, lon;
	double speed; //TODO units?
} GPSSample;

double ADC_sample[6]; // lowest->highest; V
double IMU_sample[6]; // acc_x, acc_y, acc_z, roll, pitch, yaw; m/s^2, degrees
GPSSample GPS_sample;
double RPM_sample[4]; // bl, br, fl, fr
double PWM_sample; // Duty cycle
bool ADC_sample_recorded = true, ADC_sample_sent = true;
bool IMU_sample_recorded = true, IMU_sample_sent = true;
bool GPS_sample_recorded = true, GPS_sample_sent = true;
bool RPM_sample_recorded = true, RPM_sample_sent = true;
bool PWM_sample_recorded = true, PWM_sample_sent = true;

int write_double(FILE *dest, const void *data, size_t count) {
	if (fwrite(data, sizeof(double), count, dest) != count) return -1;
	return 0;
}
int write_time(FILE *dest, struct timespec time) {
	if (fputc(0, dest) < 0) return -1;
	double secs = time.tv_sec + time.tv_nsec/1e9;
	return write_double(dest, &secs, 1);
}
int write_ADC_sample(FILE *dest) {
	if (fputc(1, dest) < 0) return -1;
	return write_double(dest, ADC_sample, 6);
}
int write_IMU_sample(FILE *dest) {
	if (fputc(2, dest) < 0) return -1;
	return write_double(dest, IMU_sample, 6);
}
int write_GPS_sample(FILE *dest) {
	if (fputc(3, dest) < 0) return -1;
	if (fputc(GPS_sample.satellites, dest) < 0) return -1;
	if (write_double(dest, &GPS_sample.lat, 1) < 0) return -1;
	if (write_double(dest, &GPS_sample.lon, 1) < 0) return -1;
	if (write_double(dest, &GPS_sample.speed, 1) < 0) return -1;
	return 0;
}
int write_RPM_sample(FILE *dest) {
	if (fputc(4, dest) < 0) return -1;
	return write_double(dest, RPM_sample, 4);
}
int write_PWM_sample(FILE *dest) {
	if (fputc(5, dest) < 0) return -1;
	return write_double(dest, &PWM_sample, 1);
}
int write_samples(
	FILE *dest,
	struct timespec time, 
	bool *ADC_sample_written,
	bool *IMU_sample_written,
	bool *GPS_sample_written,
	bool *RPM_sample_written,
	bool *PWM_sample_written
) {
	int res = write_time(dest, time);
	if (res == 0 && !ADC_sample_written) {
		res = write_ADC_sample(dest);
		*ADC_sample_written = false;
	}
	if (res == 0 && !IMU_sample_written) {
		res = write_IMU_sample(dest);
		*IMU_sample_written = false;
	}
	if (res == 0 && !GPS_sample_written) {
		res = write_GPS_sample(dest);
		*GPS_sample_written = false;
	}
	if (res == 0 && !RPM_sample_written) {
		res = write_RPM_sample(dest);
		*RPM_sample_written = false;
	}
	if (res == 0 && !PWM_sample_written) {
		res = write_ADC_sample(dest);
		*PWM_sample_written = false;
	}
	return res;
}

// ---== Callbacks ==--- //
//TODO

// ---== Main Loop ==--- //

int run() {
	signal(SIGINT, sigint_handler);

	// Initialize and open resources
	//TODO
	FILE *out_file = fopen(OUT_FILE, "w");
	if (out_file == NULL) return -1;
    ADC adc_low, adc_high;
    int init_result = ADC_init(&adc_low, I2C_BUS, adc_low_addr);
	init_result |= ADC_init(&adc_high, I2C_BUS, adc_high_addr); //XXX may have to share bus
	if (init_result < 0) return -2;
    adc_low.config.continuous_mode = false;
    adc_low.config.sample_rate = HZ3_75;
    adc_low.config.gain = 0;
	adc_high.config = adc_low.config;
    IMU imu;
    if (IMU_init(&imu, I2C_BUS) < 0) return -3; //XXX may have to share bus
	
	// Setup callbacks
	//TODO

	// Calculate sampling timings
	const uint32_t send_interval = (uint32_t)(loop_frequency/send_frequency); // loops/send
	const uint32_t ADC_interval = (uint32_t)(loop_frequency/ADC_frequency);   // loops/ADC sample
	const uint32_t IMU_interval = (uint32_t)(loop_frequency/IMU_frequency);   // loops/IMU sample
	const uint64_t common_interval = ((uint64_t)ADC_interval)*IMU_interval;
	const double loop_time = 1.0/loop_frequency;
	struct timespec sleep_time = {
		.tv_sec = (time_t)loop_time,
		.tv_nsec = (long)(fmod(loop_time, 1.0) * 1e9)
	};

	// Main loop
	uint64_t i = 0;
	struct timespec last_time = now();
	while (!interrupted) {
		// Sample ADC
		if (i % ADC_interval == 0) {
			for (size_t i = 0; i < 3; i += 1) {
				adc_low.config.channel = i;
				adc_low.config.ready = false;
				if (ADC_configure(adc_low) < 0) return -4;
				adc_high.config = adc_low.config;
				if (ADC_configure(adc_high) < 0) return -4;
				while (ADC_is_ready(adc_low) == 0);
				if (ADC_read(adc_low, &ADC_sample[i]) != 1) return -5;
				while (ADC_is_ready(adc_high) == 0);
				if (ADC_read(adc_high, &ADC_sample[3 + i]) != 1) return -5;
			}
			ADC_sample_recorded = false;
			ADC_sample_sent = false;
		}
		// Sample IMU
		if (i % IMU_interval == 0) {
			if (IMU_read_acceleration(imu, &IMU_sample[0]) < 0) return -9;
			if (IMU_read_angle(imu, &IMU_sample[3]) < 0) return -10;
			IMU_sample_recorded = false;
			IMU_sample_sent = false;
		}
		// Record samples
		if (write_samples(
			out_file,
			last_time,
			&ADC_sample_recorded,
			&IMU_sample_recorded,
			&GPS_sample_recorded,
			&RPM_sample_recorded,
			&PWM_sample_recorded
		) < 0) return -11;
		// Send samples
		if (i % send_interval == 0) {
			if (write_samples(
				stdout,
				last_time,
				&ADC_sample_sent,
				&IMU_sample_sent,
				&GPS_sample_sent,
				&RPM_sample_sent,
				&PWM_sample_sent
			) < 0) return -12;
		}
		// Increment
		i = (i + 1) % common_interval;
		// Sleep for remaining loop time
		struct timespec cur_time = now();
		sleep_remainder(sleep_time, last_time, cur_time);
		last_time = cur_time;
	}

	// Deinitialize and close resources
	//TODO
	if (IMU_deinit(imu) < 0) return -8;
	int deinit_result = ADC_deinit(adc_high);
	deinit_result |= ADC_deinit(adc_high);
	if (deinit_result < 0) return -6;
	if (fclose(out_file) < 0) return -7;
	return 0;
}

int main() {
	switch (run()) {
	case -1:
		fprintf(stderr, "error: opening file \"%s\" \n", OUT_FILE);
		break;
	case -2:
		fprintf(stderr, "error: opening I2C bus for ADC \n");
		break;
	case -3:
		fprintf(stderr, "error: opening I2C bus for IMU \n");
		break;
	case -4:
		fprintf(stderr, "error: configuring ADC \n");
		break;
	case -5:
		fprintf(stderr, "error: reading ADC \n");
		break;
	case -9:
		fprintf(stderr, "error: reading acceleration \n");
		break;
	case -10:
		fprintf(stderr, "error: reading angle \n");
		break;
	case -11:
		fprintf(stderr, "error: recording samples \n");
		break;
	case -12:
		fprintf(stderr, "error: sending samples \n");
		break;
	case -8:
		fprintf(stderr, "error: deinitializing IMU bus \n");
		break;
	case -6:
		fprintf(stderr, "error: deinitializing I2C buses \n");
		break;
	case -7:
		fprintf(stderr, "error: closing file \"%s\" \n", OUT_FILE);
		break;
	}
}

// ---== Helpers ==--- //

struct timespec now() {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return ts;
}

void sleep_remainder(
	struct timespec duration,
	struct timespec from,
	struct timespec now
) {
	struct timespec req = {
		.tv_sec = duration.tv_sec + (now.tv_sec - from.tv_sec),
		.tv_nsec = duration.tv_nsec + (now.tv_nsec - from.tv_nsec)
	};
	if (req.tv_nsec < 0) {
		req.tv_sec -= 1;
		req.tv_nsec += 1e9;
	}
	nanosleep(&req, NULL);
}
