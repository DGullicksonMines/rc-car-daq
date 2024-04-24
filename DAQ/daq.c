#include <stdbool.h>
#include <stddef.h> // size_t
#include <stdint.h>
#include <math.h> // fmod()
#include <stdio.h> // FILE, fprintf(), fwrite(), fputc(), fopen(), fclose(), stderr
#include <signal.h> // signal(), SIGINT
#include <time.h> // timespec, clock_gettime(), nanosleep(), CLOCK_REALTIME

#include "mcp3424.h"
#include "jy901.h"
#include "gpio.h"

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
double millis();
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
double PWM_sample[2]; // steering, throttle; duty cycle
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
	printf("time ");
	if (fputc(0, dest) < 0) return -1;
	double secs = time.tv_sec + time.tv_nsec/1e9;
	return write_double(dest, &secs, 1);
}
int write_ADC_sample(FILE *dest) {
	printf("adc ");
	if (fputc(1, dest) < 0) return -1;
	return write_double(dest, ADC_sample, 6);
}
int write_IMU_sample(FILE *dest) {
	printf("imu ");
	if (fputc(2, dest) < 0) return -1;
	return write_double(dest, IMU_sample, 6);
}
int write_GPS_sample(FILE *dest) {
	printf("gps ");
	if (fputc(3, dest) < 0) return -1;
	if (fputc(GPS_sample.satellites, dest) < 0) return -1;
	if (write_double(dest, &GPS_sample.lat, 1) < 0) return -1;
	if (write_double(dest, &GPS_sample.lon, 1) < 0) return -1;
	if (write_double(dest, &GPS_sample.speed, 1) < 0) return -1;
	return 0;
}
int write_RPM_sample(FILE *dest) {
	printf("rpm ");
	if (fputc(4, dest) < 0) return -1;
	return write_double(dest, RPM_sample, 4);
}
int write_PWM_sample(FILE *dest) {
	printf("pwm ");
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
		res = write_PWM_sample(dest);
		*PWM_sample_written = false;
	}
	printf("\n");
	return res;
}

// ---== Callbacks ==--- //
// --= RPM =-- //
#define RPM_INTERRUPTS 4
#define NUM_TIMES 5
const uint8_t rpm_pins[RPM_INTERRUPTS] = {10, 22, 27, 17};
double times[RPM_INTERRUPTS][NUM_TIMES];
bool all_valid[RPM_INTERRUPTS] = {0};
size_t cur_idx[RPM_INTERRUPTS] = {0};

void generic_rpm_handler(
	const size_t index,
	double cur_time,
	double *const times,
	bool *const all_valid,
	size_t *const cur_idx
) {
	// Print RPM
	const size_t first_idx = (*all_valid) ? *cur_idx : 0;
	// const size_t prev_idx = (*cur_idx - 1 + NUM_TIMES) % NUM_TIMES;
	const double first_time = times[first_idx];
	// const double prev_time = times[prev_idx];
	const size_t num_valid = (*all_valid) ? NUM_TIMES : *cur_idx;
	const double to_rpm = 1000.0*60.0/5.0; // interrupts/ms to rpm
	const double full_rpm = to_rpm*num_valid/(cur_time - first_time);
	// const double single_rpm = to_rpm/(cur_time - prev_time);
	//XXX Only one has to change for sample to be resent
	RPM_sample[index] = full_rpm;
	RPM_sample_recorded = false;
	RPM_sample_sent = false;
	printf("rpm sample \n");
	// Record time
	times[*cur_idx] = cur_time;
	// Increment
	*cur_idx = (*cur_idx + 1) % NUM_TIMES;
	if (*cur_idx == 0) *all_valid = true;
}

void generic_rpm_handler_wrapper(
	const size_t interrupt,
	uint64_t ns
) {
	generic_rpm_handler(
		interrupt,
		ns/1e6,
		times[interrupt],
		&all_valid[interrupt],
		&cur_idx[interrupt]
	);
}

#define rpm_handler(i) void rpm_handler_##i(uint64_t ns, bool active) { \
	(void)active; \
	generic_rpm_handler_wrapper(i, ns); \
}
rpm_handler(0)
rpm_handler(1)
rpm_handler(2)
rpm_handler(3)
void (* const rpm_handlers[RPM_INTERRUPTS])(uint64_t, bool) = {
	&rpm_handler_0,
	&rpm_handler_1,
	&rpm_handler_2,
	&rpm_handler_3
};

// --= PWM =-- //
#define PWM_INTERRUPTS 2
const uint8_t pwm_pins[PWM_INTERRUPTS] = {9, 11};
double prev_inactive_times[PWM_INTERRUPTS] = {0};
double prev_active_times[PWM_INTERRUPTS] = {0};

void generic_pwm_handler(
	size_t index,
	double cur_time,
	bool active,
	double *prev_active_time,
	double *prev_inactive_time
) {
	double active_duration;
	double total_duration;
	if (active) {
		active_duration = *prev_inactive_time - *prev_active_time;
		total_duration = cur_time - *prev_active_time;
	} else {
		active_duration = cur_time - *prev_active_time;
		total_duration = cur_time - *prev_inactive_time;
	}
	PWM_sample[index] = active_duration/total_duration;
	//XXX Only one has to change for sample to be resent
	PWM_sample_recorded = false;
	PWM_sample_sent = false;
	printf("pwm sample \n");
}

void generic_pwm_handler_wrapper(
	const size_t interrupt,
	uint64_t ns,
	bool active
) {
	generic_pwm_handler(
		interrupt,
		ns/1e6,
		active,
		&prev_active_times[interrupt],
		&prev_inactive_times[interrupt]
	);
}

#define pwm_handler(i) void pwm_handler_##i(uint64_t ns, bool active) { \
	generic_pwm_handler_wrapper(i, ns, active); \
}
pwm_handler(0)
pwm_handler(1)
void (* const pwm_handlers[PWM_INTERRUPTS])(uint64_t, bool) = {
	&pwm_handler_0,
	&pwm_handler_1
};

// ---== Main Loop ==--- //

int run() {
	signal(SIGINT, sigint_handler);

	// --= Initialize and open resources =-- //
	// Open out file
	FILE *out_file = fopen(OUT_FILE, "w");
	if (out_file == NULL) return -1;
	// Initialize ADCs
    ADC adc_low, adc_high;
    int init_result = ADC_init(&adc_low, I2C_BUS, adc_low_addr);
	init_result |= ADC_init(&adc_high, I2C_BUS, adc_high_addr); //XXX may have to share bus
	if (init_result < 0) return -2;
    adc_low.config.continuous_mode = false;
    adc_low.config.sample_rate = HZ3_75;
    adc_low.config.gain = 0;
	adc_high.config = adc_low.config;
	// Initialize IMU
    IMU imu;
    if (IMU_init(&imu, I2C_BUS) < 0) return -3; //XXX may have to share bus
	
	// --= Setup callbacks =-- //
	//TODO
	PinInterrupt pin_interrupts[RPM_INTERRUPTS + PWM_INTERRUPTS];
	// Setup rpm interrupts
	for (size_t i = 0; i < RPM_INTERRUPTS; i += 1) {
		pin_interrupts[i].pin = rpm_pins[i];
		pin_interrupts[i].edge = EdgeTypeFalling;
		pin_interrupts[i].bias = BiasPullUp;
		pin_interrupts[i].interrupt = rpm_handlers[i];
	}
	// Setup pwm interrupts
	for (size_t i = RPM_INTERRUPTS; i < RPM_INTERRUPTS + PWM_INTERRUPTS; i += 1) {
		pin_interrupts[i].pin = pwm_pins[i];
		pin_interrupts[i].edge = EdgeTypeBoth;
		pin_interrupts[i].bias = BiasNone;
		pin_interrupts[i].interrupt = pwm_handlers[i];
	}
	// Begin interrupt polling
	Handle handle;
	if (begin_interrupt_polling(pin_interrupts, RPM_INTERRUPTS, &handle) < 0)
		return -13;

	// --= Calculate sampling timings =-- //
	const uint32_t send_interval = (uint32_t)(loop_frequency/send_frequency); // loops/send
	const uint32_t ADC_interval = (uint32_t)(loop_frequency/ADC_frequency);   // loops/ADC sample
	const uint32_t IMU_interval = (uint32_t)(loop_frequency/IMU_frequency);   // loops/IMU sample
	const uint64_t common_interval = ((uint64_t)ADC_interval)*IMU_interval;
	const double loop_time = 1.0/loop_frequency;
	struct timespec sleep_time = {
		.tv_sec = (time_t)loop_time,
		.tv_nsec = (long)(fmod(loop_time, 1.0)*1e9)
	};
	printf("sleep time: %ld \n", sleep_time.tv_sec);

	// --= Main loop =-- //
	uint64_t i = 0;
	struct timespec last_time = now();
	while (!interrupted) {
		// Sample ADC
		if (i % ADC_interval == 0) {
			printf("sampling ADC \n");
			for (size_t i = 0; i < 3; i += 1) {
				adc_low.config.channel = i;
				adc_low.config.ready = false;
				// if (ADC_configure(adc_low) < 0) return -4;
				adc_high.config = adc_low.config;
				if (ADC_configure(adc_high) < 0) return -4;
				// while (ADC_is_ready(adc_low) == 0);
				// if (ADC_read(adc_low, &ADC_sample[i]) != 1) return -5;
				while (ADC_is_ready(adc_high) == 0);
				if (ADC_read(adc_high, &ADC_sample[3 + i]) != 1) return -5;
			}
			ADC_sample_recorded = false;
			ADC_sample_sent = false;
		}
		// Sample IMU
		if (i % IMU_interval == 0) {
			printf("sampling IMU \n");
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
			printf("sending \n");
			// if (write_samples(
			// 	stdout,
			// 	last_time,
			// 	&ADC_sample_sent,
			// 	&IMU_sample_sent,
			// 	&GPS_sample_sent,
			// 	&RPM_sample_sent,
			// 	&PWM_sample_sent
			// ) < 0) return -12;
		}
		// Increment
		i = (i + 1) % common_interval;
		// Sleep for remaining loop time
		struct timespec cur_time = now();
		sleep_remainder(sleep_time, last_time, cur_time);
		last_time = cur_time;
	}

	// --= Deinitialize and close resources =-- //
	//TODO
	// End interrupt polling
	if (end_interrupt_polling(&handle) < 0) return -14;
	// Deinitialize IMU
	if (IMU_deinit(imu) < 0) return -8;
	// Deinitialize ADCs
	int deinit_result = ADC_deinit(adc_high);
	deinit_result |= ADC_deinit(adc_high);
	if (deinit_result < 0) return -6;
	// Close out file
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
	case -13:
		fprintf(stderr, "error: beginning interrupt polling \n");
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
	case -14:
		fprintf(stderr, "error: ending interrupt polling \n");
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

double millis() {
	struct timespec ts = now();
	double ms = (double)ts.tv_sec*1e3;
	ms += (double)ts.tv_nsec/1e6;
	return ms;
}

void sleep_remainder(
	struct timespec duration,
	struct timespec from,
	struct timespec now
) {
	struct timespec req = {
		.tv_sec = duration.tv_sec - (now.tv_sec - from.tv_sec),
		.tv_nsec = duration.tv_nsec - (now.tv_nsec - from.tv_nsec)
	};
	if (req.tv_nsec < 0) {
		req.tv_sec -= 1;
		req.tv_nsec += 1e9;
	}
	printf("waiting %ld, target %ld \n", req.tv_sec, duration.tv_sec);
	nanosleep(&req, NULL);
}
