/**
 * Fan Controller (fanctl) functions.
 *
 * Copyright (c) 2017 lkwk
 */

/**
 * Receives raw data from temperature sensor and returns only a temperature.
 */
float formatTemp(std::string temp)
{
    // The temperature is found in the last five digits of the string.
    return (atof(temp.substr(69, 5).c_str()) / 1000);
}

/**
 * Iterates through the 'temps' vector and outputs all temps.
 */
std::string concatTemps()
{
    std::ostringstream out;
    for (unsigned int i = 0; i < temps.size(); i++) {
        out << temps[i] << ", ";
    }

    return out.str().substr(0, (out.str().size() - 2));
}

/**
 * Open the configured sensors and store them in the 'sensors' vector.
 */
void openSensorStreams()
{
    for (unsigned int i = 0; i < sensors.size(); i++) {
        std::ifstream* in = new std::ifstream;
        streams.push_back(in);
        streams[i]->open(sensors[i]);
        if (!streams[i]->is_open()) {
            std::cout << "Unable to open " << sensors[i] << std::endl;
            exit(1);
        }
    }
}

/**
 * Read from the opened streams and store returned data in the 'temps' vector.
 */
void readSensorStreams()
{
    for (unsigned int i = 0; i < streams.size(); i++) {
        std::string temp = "";
        for (std::string line; getline(*streams[i], line); ) {
            temp += line + " ";
        }
        streams[i]->clear();
        streams[i]->seekg(0, streams[i]->beg);
        temps.push_back(formatTemp(temp));
	}
}

/**
 * Receives a vector of temperatures and returns it's average.
 */
float getAvg(std::vector<float> v)
{
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

/**
 * Handles control events (such as CTRL+C) that shut down the program.
 *
 * Close all open ifstreams and stop all fans by powering down the GPIO pins.
 */
void handleControlEvent(int sig)
{
    for (unsigned int i = 0; i < streams.size(); i++) {
        streams[i]->close();
    }
    pwmWrite(pinFanSpeed, 0);
    digitalWrite(pinFanPower, 0);
    delay(100);
    exit(0);
}
