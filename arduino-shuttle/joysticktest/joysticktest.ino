/**
 * Joystick Test
 * 
 * Code allowing the test of a two-directional potentiometer and a switch
 */

#include "Servo.h"

// Wiring constants
const int POT_X_PIN = A0;  // the analog I/O pin the potentiometer's x axis is plugged into
const int POT_Y_PIN = A1;
const int SW_PIN = 6;  // the digital pin the switch is plugged into

// Programatic constants
const int SERIAL_SPEED = 9600;  // bits per second, align with Tools -> Serial Monitor's dropdown menu
const int DELAY = 50;           // milliseconds

void setup() {
	// Initialize pins
	pinMode(SW_PIN, INPUT);
	digitalWrite(SW_PIN, HIGH);

	Serial.begin(SERIAL_SPEED);
}

void loop() {
	int pot_x = analogRead(POT_X_PIN);
	int pot_y = analogRead(POT_Y_PIN);
	bool curr_sw = digitalRead(SW_PIN) == LOW;

	Serial.print("x: ");
	Serial.print(pot_x);
	Serial.print(", y: ");
	Serial.print(pot_y);
	Serial.print(", sw: ");
	Serial.println(curr_sw);

	delay(DELAY);
}
