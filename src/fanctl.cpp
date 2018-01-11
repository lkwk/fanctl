
/**
 * Fan Controller (fanctl)
 *
 * This program reads temperatures from Dallas DS18S20 one-wire digital
 * thermometers and controls PWM fan speed accordingly.
 *
 * Compile command:
 * g++ -Wall -lwiringPi fanctl.cpp -o fanctl
 *
 * Copyright (c) 2017 lkwk
 */

#include "fanctl.h"

// + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
// Application.
int main () {

	// Handle interrupt signals.
	(void)signal(SIGINT, handleControlEvent);
	(void)signal(SIGQUIT, handleControlEvent);
	(void)signal(SIGTERM, handleControlEvent);
	(void)signal(SIGABRT, handleControlEvent);

	// Check if we are root, since wiringPiSetupGpio() requires this.
	if (geteuid() == 0) {

    // Setup the GPIO pins.
    if (wiringPiSetupGpio() >= 0) {

			// Get system hostname and uname.
			gethostname(hostname, 1023);
			uname(&uts);

			// Pin 18 is the only pin on the Raspberry Pi that supports hardware PWM.
      // Pin 25 is used to control the power supply to the status LED and to
			// switch on the 12V DC to the fans.
      pwmSetMode(PWM_MODE_MS);
      pinMode(pinFanSpeed, PWM_OUTPUT);
      pinMode(pinFanPower, OUTPUT);

			// The base clock on the RPi's PWM is 19.2MHz. The PWM fans need a
			// frequency of 25kHz (+/- 4kHz) according to the PWM specification. In
			// order to get to this frequency we set the following values, which give
			// us a frequency of 25kHz (19200kHz/48/16).
			// The range defines the number of steps from 0 to 100% (in this case 16).
      pwmSetClock(48);
      pwmSetRange(16);

			// Open the paths to the sensor(s) for reading, the opened ifstreams will
			// be stored in the 'sensors' vector. If a stream can not be opened the
			// program will exit.
			openSensorStreams();

			// Instead of creating a daemon we'll do this for now.
			for (;;) {

				// Clear data from previous iteration.
				// Go over the open streams, read data and store in 'temps' vector.
				// Calculate the average of all temperatures and set fan speed.
				temps.clear();
				readSensorStreams();
				avg = getAvg(temps);

				// We need to turn the fan(s) ON, or adjust speed.
				if (avg >= fanOnTemp) {

					// The step is the speed the fan will run at and it needs to be
					// between 4 (the minimum to get the fan to spin-up) and 16. With the
					// following calculation values over 16 are possible, due to the fact
					// that the average can be over the fanMaxTemp.
					step = round((((avg - fanOnTemp) / (fanMaxTemp - fanOnTemp)) * 100) / 6.25);
					if (step < 4) { step = 4; }
					if (step > 16) { step = 16; }

					status = "on";

					// Turn fans ON and set the speed.
					digitalWrite(pinFanPower, 1);
					pwmWrite(pinFanSpeed, step);

				}

				// We need to turn the fan(s) OFF.
				if (avg <= fanOffTemp) {

					step = 0;
					status = "off";
					pwmWrite(pinFanSpeed, step);
					digitalWrite(pinFanPower, 0);

				}

				// Output some statistics in JSON format to STDOUT.
				// We could catch STDOUT with `websocketd` and push it to any webpage.
				if (enableJsonOut) {

					// For now the fan's RPM and speed-% is calculated, not measured.
					rpm = ((step * 46.875) + ((step < 4) ? (0) : (600)));
					pct = step * 6.25;

					// Get the sys- and diskinfo for our statistics feed.
					sysinfo(&si);
					statvfs("/", &vfs);

					// Assemble and output the statistics.
					std::cout << "{";
					std::cout << " \"fans\": { \"status\": \"" << status << "\", \"config\": { \"on_temp\": " << fanOnTemp << ", \"off_temp\": " << fanOffTemp << ", \"max_temp\": " << fanMaxTemp << "}, \"all_temps\": [" << concatTemps() << "], \"avg_temp\": " << avg << ", \"step\": " << step << ", \"rpm\": " << rpm << ", \"pct\": " << pct << " },";
					std::cout << " \"system\": { \"load\": [" << (si.loads[0] / 65536.0) << ", " << (si.loads[1] / 65536.0) << ", " << (si.loads[2] / 65536.0) << "], ";
					std::cout <<                "\"mem\": { \"total\": "<< si.totalram << ", \"free\": " << si.freeram << ", \"used\": " << (si.totalram - si.freeram) << " }, ";
					std::cout <<                "\"disk\": { \"total\": "<< std::setprecision(16) << (vfs.f_bsize * (double)vfs.f_blocks) << ", \"free\": " << std::setprecision(16) << (vfs.f_bsize * (double)vfs.f_bfree) << ", \"used\": " << std::setprecision(16) << ((vfs.f_bsize * (double)vfs.f_blocks) - (vfs.f_bsize * (double)vfs.f_bfree)) << " }, ";
					std::cout <<                "\"uts\": { \"sysname\": \"" << uts.sysname << "\", \"nodename\": \"" << uts.nodename << "\", \"release\": \"" << uts.release << "\", \"version\": \"" << uts.version << "\", \"machine\": \"" << uts.machine << "\" }, ";
					std::cout <<                "\"hostname\": \"" << hostname << "\", \"procs\": " << si.procs << ", \"uptime\": " << si.uptime << "}";
					std::cout << " }" << std::endl;

				}

			} // End for(;;)

		  return 0;

		} else {

			std::cerr << "The GPIO setup did not complete succesfully." << std::endl;
			exit(1);

		}

	} else {

		std::cout << "This program needs root privileges." << std::endl;
		exit(1);

	}

}
