// Copyright 2024 by Dawson J. Gullickson

//NOTE Start GPSD using
// ```
// sudo gpsd -n /dev/ttyAMA0
// ```

#include <stdbool.h>
#include <stdio.h>
#include <signal.h>
#include <gps.h>

bool interrupted = false;
void sigint_handler(int signum) {
	(void)(signum); // Suppress unused-parameter
	interrupted = true;
}

void callback(struct gps_data_t *gps_data) {
	struct tm *time = localtime(&gps_data->fix.time.tv_sec);
	char time_str[9];
	strftime(time_str, sizeof(time_str), "%H:%M:%S", time);
	printf("Time: %s \n", time_str);
	printf("Satellites: %d \n", gps_data->satellites_used);
	printf("Location: %.4f, ", gps_data->fix.latitude);
	printf("%.4f \n", gps_data->fix.longitude);
	printf("Speed: %f \n", gps_data->fix.speed);
	printf("Heading: %f \n", gps_data->attitude.heading);
	printf("Altitude: %f \n", gps_data->fix.altitude);
}

int main() {
	signal(SIGINT, sigint_handler);

	//TODO we could start gpsd automatically here

	// Open GPSD connection
	struct gps_data_t gps_data;
	int res = gps_open(GPSD_SHARED_MEMORY, "", &gps_data);
	if (res < 0) {
		printf("error %d: opening gps \n", res);
		return -1;
	}

	// Register callback
	res = gps_mainloop(&gps_data, 1000, &callback);
	if (res < 0) {
		printf("error %d: registering gps callback \n", res);
		return -2;
	}

	while (!interrupted);

	// Close GPSD connection
	res = gps_close(&gps_data);
	if (res < 0) {
		printf("error %d: closing gps \n", res);
		return -2;
	}
}
