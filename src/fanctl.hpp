/**
 * Fan Controller (fanctl) header.
 *
 * Copyright (c) 2017 lkwk
 */
#include <vector>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <sys/types.h>
#include <linux/kernel.h>
#include <linux/sysinfo.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <sys/utsname.h>
#include <wiringPi.h>
#include <lcd.h>

/**
 * Configuration variables.
 */
int pinFanSpeed = 18;           // The PWM-pin for fan speed regulation.
int pinFanPower = 25;           // The pin that will switch on 12V DC.
bool enableLcdMonitor = true;   // Enable/disable output to a HD44780U LCD monitor.
bool enableJsonOut = true;      // Enable/disable JSON output to STDOUT.
float fanOnTemp = 28.0;         // Temp. at which fan will turn on (deg C).
float fanOffTemp = 27.0;        // Temp. at which fan will turn off (deg C).
float fanMaxTemp =  33.0;       // Temp. at which fan will run at 100% (deg C).

/**
 * Enter the full path to your sensors here.
 * Example: /sys/bus/w1/devices/10-000802b57e57/w1_slave
 */
std::vector<std::string> sensors = {
    "/sys/bus/w1/devices/10-000802b57e57/w1_slave",
    "/sys/bus/w1/devices/10-000802b5a835/w1_slave"
};

/**
 * Internal program variables.
 */
int mon = 0;
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
static unsigned char degsymbol[8] = {
    0b00000, 0b00110, 0b00110, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000
};
