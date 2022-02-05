## Synopsis

Fan Controller (fanctl) reads the temperature from one or more Dallas DS18S20 one-wire digital thermometers and controls the speed of a PWM fan accordingly. To control multiple fans, you can wire them in parallel. This program is intended for any Raspberry Pi and may also work on similar devices. A [Fritzing](http://fritzing.org/home/) sketch for the electrical components and setup is included. The program can output JSON to STDOUT, which can be useful with in combination with websocketd.

## Installation

You need to have [wiringPi](http://wiringpi.com/) installed on your Raspberry Pi in order to compile this program and you will need to add the following line to your `/boot/config.txt` (don't forget to reboot after making the change):  
```
dtoverlay=w1-gpio,gpiopin=4
```
Open `fanctl.hpp` in your favorite text editor, set the path(s) to your temperature sensor(s), configure your GPIO pins and set your temperature limits. Then use the following command from the src directory to compile:  
```
g++ -Wall -lwiringPi fanctl.cpp -o fanctl  
```
Or from the project root:
```
make
```
Then copy the binary to your `bin` directory.

Run this program as root with `./fanctl`, or edit and use the included `fanctl.service` file for systemd.  
