#include <Encoder.h>
#include <Servo.h>

#define ENCODER_PIN_1 2
#define ENCODER_PIN_2 3

const int SERVO_PIN = 4;
const int CENTER = 1500;
const int DEV = 500;
const int STEP = 10;

const int START_DELAY = 1000; // milliseconds
const int DELAY = 50;

Servo servo;
Encoder encoder(ENCODER_PIN_1, ENCODER_PIN_2);
int servo_pos;
bool going_left;

void setup() {
	Serial.begin(9600);
	Serial.println("Basic Encoder Test:");

	// Set up servo
	servo.attach(SERVO_PIN);
	servo_pos = CENTER;
	going_left = true;

	// Set servo to center
	servo.writeMicroseconds(servo_pos);
	delay(START_DELAY);
}

void loop() {
	// Move servo
	Serial.print("Servo: ");
	if (going_left) {
		servo_pos += STEP;
		if (servo_pos >= CENTER + DEV) {
			going_left = false;
		}
	}
	else {
		servo_pos -= STEP;
		if (servo_pos <= CENTER - DEV) {
			going_left = true;
		}
	}
	servo.writeMicroseconds(servo_pos);
	Serial.print(servo_pos);

	// Update encoder
	long encoder_pos = encoder.read();
	Serial.print(", Encoder: ");
	Serial.println(encoder_pos);

	delay(DELAY);
}