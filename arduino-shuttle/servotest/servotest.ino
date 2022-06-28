/**
 * Servo Test
 * 
 * Runs a servo based on a potentiometer input, and prints the PWM signal used in microseconds.
 * Useful for setting up servos or testing them.
 */

#include "Servo.h"

const int SERVO_PIN = 2;  // the digital PWM pin the servo is plugged into
const int POT_PIN = A0;   // the analog I/O pin the potentiometer is plugged into

const int SERIAL_SPEED = 9600;  // bits per second, align with Tools -> Serial Monitor's dropdown menu
const int DELAY = 1000;         // milliseconds

const int POT_MAX = 1023;       // max value of the potentiometer (min is assumed 0)
const int SERVO_CENTER = 1500;  // center position, in microseconds
const int SERVO_DEV = 1000;     // how far the min/max are from the center, in microseconds

Servo servo;

/**
 * Sets up the servo and serial
 */
void setup() {
	servo.attach(SERVO_PIN);
	Serial.begin(SERIAL_SPEED);
}

/**
 * Reads the potentiometer, maps it to the servo range, prints its value, and sends that to the servo
 * Waits `DELAY`, then repeats
 */
void loop() {
	int pot_val = analogRead(POT_PIN);
	pot_val = map(pot_val, 0, POT_MAX, SERVO_CENTER - SERVO_DEV, SERVO_CENTER + SERVO_DEV);
	Serial.println(pot_val);

	servo.writeMicroseconds(pot_val);
	delay(DELAY);
}