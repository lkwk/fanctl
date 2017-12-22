
/**
 * Fan Controller (fanctl) helper functions.
 *
 * Copyright (c) 2017 lkwk
 */

// ----- HEADERS -----

#include <vector>
#include <sstream>							// ostringstream
#include <iostream>
#include <iomanip>							// setprecision
#include <fstream>
#include <string>
#include <unistd.h>
#include <math.h>								// round()
#include <signal.h>							// SIGINT, SIGQUIT, etc.
#include <sys/types.h>
#include <linux/kernel.h>				// sysinfo()
#include <linux/sysinfo.h>			// sysinfo()
#include <sys/sysinfo.h>				// sysinfo()
#include <sys/statvfs.h>				// statvfs()
#include <sys/utsname.h>				// uname()
#include <wiringPi.h>

// ----- CONFIGURATION VARIABLES -----

int pinFanSpeed = 18;						// The PWM-pin for fan speed regulation.
int pinFanPower = 25;						// The pin that will switch on 12V DC.
bool enableJsonOut = true;			// Enable/disable JSON output to STDOUT.
float fanOnTemp = 28.0;					// Temp. at which fan will turn on (deg C).
float fanOffTemp = 27.0;				// Temp. at which fan will turn off (deg C).
float fanMaxTemp =  33.0;				// Temp. at which fan will run at 100% (deg C).

// Full path to your sensor (i.e: /sys/bus/w1/devices/10-000802b57e57/w1_slave).
// Store as many paths as you like.
std::vector<std::string> sensors = {
	"/sys/bus/w1/devices/10-000802b57e57/w1_slave",
	"/sys/bus/w1/devices/10-000802b5a835/w1_slave"
};

// ----- HOUSEKEEPING VARIABLES -----

int step = 0;
float avg = 0.0;
float rpm = 0.0;
float pct = 0.0;
char hostname[1024];
std::string status = "off";
std::vector<std::ifstream*> streams;
std::vector<float> temps;
struct sysinfo si;
struct statvfs vfs;
struct utsname uts;

// ----- FUNCTIONS -----

// + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
// Receives raw data from temperature sensor and returns only a temperature.
float formatTemp(std::string temp) {

	// The temperature is found in the last five digits of the string.
	return (atof(temp.substr(69, 5).c_str()) / 1000);

}

// + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
// Iterates through the 'temps' vector and outputs all temps.
std::string concatTemps() {

	std::ostringstream out;

	for (unsigned int i = 0; i < temps.size(); i++) {

		out << temps[i] << ", ";

	}

	return out.str().substr(0, (out.str().size() - 2));

}

// + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
// Open the configured sensors and store them in the 'sensors' vector.
void openSensorStreams() {

	for (unsigned int i = 0; i < sensors.size(); i++) {

			std::ifstream* in = new std::ifstream;
			streams.push_back(in);
			streams[i]->open(sensors[i]);

			if (!streams[i]->is_open()) {

				std::cout << "Unable to open " << sensors[i] << std::endl;
				exit(1);

			}

	} // End for(sensors)

}

// + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
// Read from the opened streams and store returned data in the 'temps' vector.
void readSensorStreams() {

	for (unsigned int i = 0; i < streams.size(); i++) {

		std::string temp = "";

		// Read single stream.
		for (std::string line; getline(*streams[i], line); ) {

			temp += line + " ";

		}

		streams[i]->clear();
		streams[i]->seekg(0, streams[i]->beg);
		temps.push_back(formatTemp(temp));

	} // End for(streams)

}

// + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
// Receives a vector of temperatures and returns it's average.
float getAvg(std::vector<float> v) {

  float out = 0.0;
  int n = v.size();

	if (n > 0) {

	  for (int i = 0; i < n; i++) {

	  	out += v[i];

	  }

	  return (out / n);

	}

	return out;

}

// + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
// Handles control events (such as CTRL+C) that shut down the program.
// Close all open ifstreams and stop all fans by powering down the GPIO pins.
void handleControlEvent(int sig) {

	for (unsigned int i = 0; i < streams.size(); i++) {

		streams[i]->close();

	}

  pwmWrite(pinFanSpeed, 0);
  digitalWrite(pinFanPower, 0);
  delay(100);
  exit(0);

}
