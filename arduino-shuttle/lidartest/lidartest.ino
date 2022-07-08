#include <stdint.h>
#include <Wire.h>
#include "LIDARLite_v4LED.h"

const int CONFIG = 0;

LIDARLite_v4LED lidar;

bool getLidarDistance(int *distance);

void setup() {
    Serial.begin(115200);

	// Start wire
    Wire.begin();
    Wire.setClock(400000UL); // I2C frequency, 400kHz

	// Pull pins back down
    digitalWrite(SCL, LOW);
    digitalWrite(SDA, LOW);

	// Set up LIDAR
    lidar.configure(CONFIG);
}


void loop() {
	// Try reading from LIDAR
	int distance;
    bool success = getLidarDistance(&distance);

	if (success) {
		Serial.println(distance);
	}
}

/**
 * Takes a new measurement with the LIDAR, if possible
 *
 * @param distance A pointer to an integer. Will be updated to hold the distance, if possible.
 * @return True if a new measurement was taken, false otherwise (eg: LIDAR was busy)
 */
bool getLidarDistance(int *distance) {
	if (!lidar.getBusyFlag())
    {
        lidar.takeRange(); // measure distance
        *distance = lidar.readDistance(); // report distance

        return true;
    }

    return false;
}