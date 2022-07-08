/**
 * Shuttle Drive
 * 
 * Code to drive the shuttle
 */

#include "Servo.h"

// Wiring constants
const int LEFT_ARM_PIN = 2;  // the digital PWM pin the servo is plugged into
const int RIGHT_ARM_PIN = 3;
const int LEFT_DRIVE_PIN = 4;
const int RIGHT_DRIVE_PIN = 5;

const int POT_X_PIN = A0;  // the analog I/O pin the potentiometer's x axis is plugged into
const int POT_Y_PIN = A1;
const int POT_OFFSET_PIN = A2; // for the third potentiometer, which controls offset
const int SW_PIN = 6;  // the digital pin the switch is plugged into

// Programatic constants
const int SERIAL_SPEED = 9600;  // bits per second, align with Tools -> Serial Monitor's dropdown menu
const int DELAY = 50;           // milliseconds

const float POT_ANALOG_MAX = 1023;  // max value of the potentiometer (min is assumed 0)
const float POT_MAX = 1;
const float POT_DEAD_ZONE = 0.05;  // on the [-POT_MAX, POT_MAX] scale

// Servo constants
const float ARM_CENTER = 1500;  // center position, in microseconds
const float ARM_DEV = 900;     // how far the min/max are from the center, in microseconds
const float ARM_FACTOR = 10;     // factor of acceleration of the arms, by potentiometer input
const float OFFSET_MAX = 50;  // how much the offset potentiometer can affect the arms by, in microseconds
const float DRIVE_CENTER = 1500;
const float DRIVE_DEV = 500;

// State variables
Servo left_arm, right_arm, left_drive, right_drive;
bool lock;
bool sw;
int arm_val;    // microseconds

bool update_sw();
bool pot_in_dead_zone(float pot_val);
float pot_get_val(int pin);
float pot_get_true_val(int pin);
void set_servo(Servo servo, int val, int min, int max);

void setup() {
	// Initialize pins
	pinMode(SW_PIN, INPUT);
	digitalWrite(SW_PIN, HIGH);

	// Initialize servos
	left_arm.attach(LEFT_ARM_PIN);
	right_arm.attach(RIGHT_ARM_PIN);
	left_drive.attach(LEFT_DRIVE_PIN);
	right_drive.attach(RIGHT_DRIVE_PIN);

	// State variables
	lock = false;
	sw = false;
	arm_val = ARM_CENTER - ARM_DEV;

	Serial.begin(SERIAL_SPEED);
}

/**
 * Updates the global switch variable
 * 
 * @return True if the button has been newly pressed down in this loop cycle
 */
bool update_sw() {
	bool curr_sw = digitalRead(SW_PIN) == LOW;
	bool sw_pressed = curr_sw && !sw;
	sw = curr_sw;
	return sw_pressed;
}

/**
 * Determines if the potentiometer is in the dead zone, where values should be treated as
 * zero, not minimal
 *
 * @param pot_val The value of the potentiometer, on the [-POT_MAX, POT_MAX] scale
 * @return True if it's in the dead zone, false otherwise
 */
bool pot_in_dead_zone(float pot_val) {
	return pot_val < POT_DEAD_ZONE && pot_val > -POT_DEAD_ZONE;
}

/**
 * Gets the value at a pin, assuming a potentiometer is hooked up to it
 * Maps it to between [-POT_MAX, POT_MAX]. Does not cube or apply a deadzone.
 * Less intuitive for human control.
 *
 * @param pin The pin the potentiometer is plugged into
 * @return The value of the potentiometer
 */
float pot_get_true_val(int pin) {
	float pot_val = (float)analogRead(pin);
	// Anchors between -MAX and MAX. 
	return pot_val / POT_ANALOG_MAX * (POT_MAX * 2) - POT_MAX;
}

/**
 * Gets the value at a pin, assuming a potentiometer is hooked up to it
 * Maps it to between [-POT_MAX, POT_MAX] and applies a dead zone
 * 
 * @param pin The pin the potentiometer is plugged into
 * @return The value of the potentiometer
 */
float pot_get_val(int pin) {
	float pot_val = pot_get_true_val(pin);
	// Cubing is more intuitive for human control
	pot_val = pow(pot_val, 3);

	if (pot_in_dead_zone(pot_val)) {
		pot_val = 0;
	}

	return pot_val;
}

/**
 * Sets a servo to a given value of microseconds, ensuring it is kept within the
 * minimum and maximum bounds
 *
 * @param servo The servo to control
 * @param val The value to give it, in microseconds
 * @param min The minimum value it can have, in microseconds
 * @param max The maximum value it can have, in microseconds
 */
void set_servo(Servo servo, int val, int min, int max) {
	// Crop between min and max
	if (val < min) {
		val = min;
	}
	if (val > max) {
		val = max;
	}
	servo.writeMicroseconds(val);
}

void loop() {
	bool sw_pressed = update_sw();
	if (sw_pressed) {
		lock = !lock;
	}

	if (!lock) {
		// Potentiometer value, in [-POT_MAX, POT_MAX]
		float drive_pot = pot_get_val(POT_X_PIN);
		float arm_pot = -pot_get_val(POT_Y_PIN); // flip the y axis for intuition reasons
		float offset_pot = pot_get_true_val(POT_OFFSET_PIN);

		// Drive value, in [CENTER - DEV, CENTER + DEV] (in microseconds)
		int drive_val = (int)(drive_pot * DRIVE_DEV + DRIVE_CENTER);

		// Adjust the arm value by the potentiometer (using the potentiometer as velocity)
		arm_val += (int)(arm_pot * ARM_FACTOR);

		// Apply the offset value
		int offset_val = (int)(offset_pot * OFFSET_MAX);

		Serial.print("Drive: ");
		Serial.print(drive_val);
		Serial.print(", Arm: ");
		Serial.print(arm_val);
		Serial.print(", Offset: ");
		Serial.println(offset_val); // 14 aligned them today (7/5/22)

		// Set servos accordingly
		set_servo(left_arm, arm_val + offset_val, ARM_CENTER - ARM_DEV, ARM_CENTER + ARM_DEV);
		set_servo(right_arm, ARM_CENTER * 2 - (arm_val - offset_val), ARM_CENTER - ARM_DEV, ARM_CENTER + ARM_DEV);
		set_servo(left_drive, drive_val, DRIVE_CENTER - DRIVE_DEV, DRIVE_CENTER + DRIVE_DEV);
		set_servo(right_drive, DRIVE_CENTER * 2 - drive_val, DRIVE_CENTER - DRIVE_DEV, DRIVE_CENTER + DRIVE_DEV);
	}

	delay(DELAY);
}