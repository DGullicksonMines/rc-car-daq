#include <stdbool.h>
#include <stdint.h>
#include <math.h> // fmod()
#include <stdio.h> // FILE
#include <signal.h> // signal(), SIGINT
#include <time.h> // timespec, clock_gettime(), nanosleep(), CLOCK_REALTIME

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

//TODO document max values (based on intervals being uint32)
const double loop_frequency = 200.0; // Frequency of recording data
const double send_frequency = 10.0;  // Frequency of sending data
const double ADC_frequency = 1.0;    // Frequency of ADC sampling
const double IMU_frequency = 200.0;  // Frequency of IMU sampling

// ---== Recording/Sending ==--- //

void* ADC_sample; //TODO make these the appropriate types
void* IMU_sample;
void* GPS_sample;
void* RPM_sample;
bool ADC_sample_recorded = true, ADC_sample_sent = true;
bool IMU_sample_recorded = true, IMU_sample_sent = true;
bool GPS_sample_recorded = true, GPS_sample_sent = true;
bool RPM_sample_recorded = true, RPM_sample_sent = true;

int write_time(FILE *dest) { //TODO fill in all these functions
	//TODO
	(void)dest;
	return 0;
}
int write_ADC_sample(FILE *dest) {
	//TODO
	(void)dest;
	return 0;
}
int write_IMU_sample(FILE *dest) {
	//TODO
	(void)dest;
	return 0;
}
int write_GPS_sample(FILE *dest) {
	//TODO
	(void)dest;
	return 0;
}
int write_RPM_sample(FILE *dest) {
	//TODO
	(void)dest;
	return 0;
}

// ---== Callbacks ==--- //
//TODO

// ---== Main Loop ==--- //

int main() {
	signal(SIGINT, sigint_handler);

	// Initialize and open resources
	//TODO
	
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
			//TODO
		}
		// Sample IMU
		if (i % IMU_interval == 0) {
			//TODO
		}
		// Record samples
		//TODO
		// Send samples
		if (i % send_interval == 0) {
			//TODO
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
