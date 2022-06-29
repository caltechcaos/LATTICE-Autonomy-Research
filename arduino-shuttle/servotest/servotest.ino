/**
 * Servo Test
 * 
 * Runs a servo based on a potentiometer input, and prints the PWM signal used in microseconds.
 * Useful for setting up servos or testing them.
 */

#include "Servo.h"

// Wiring constants
const int SERVO_PIN = 2;   // the digital PWM pin the servo is plugged into
const int POT_X_PIN = A0;  // the analog I/O pin the potentiometer's x axis is plugged into
const int SW_PIN = 3;      // the digital pin the switch is plugged into

// Programatic constants
const int SERIAL_SPEED = 9600;  // bits per second, align with Tools -> Serial Monitor's dropdown menu
const int DELAY = 50;           // milliseconds

const int POT_MAX = 1023;        // max value of the potentiometer (min is assumed 0)
const int POT_DEAD_ZONE = 0.01;  // on the [-1, 1] scale

// Servo constants
const int SERVO_CENTER = 1500;  // center position, in microseconds
const int SERVO_DEV = 500;      // how far the min/max are from the center, in microseconds
const int SERVO_STEP = 50;      // the size of step taken by the Range test, in microseconds

// Menu and test definitions
typedef enum state {
	MENU,
	POT,
	ZERO,
	RANGE,
	END
} state_t;

const char *MENU_OPTIONS[] = { "Menu", "Potentiometer", "Zero", "Range" };
const char *MENU_DESCRIPTIONS[] = { "Menu",
									"Runs the servo at a value set by the potentiometer. The value is displayed.",
									"Sets the servo to the zero position.",
									"Runs the servo from its minimum value to its maximum and back. Be careful it won't hit anything!" };

// Function headers
void run_range();
void run_zero();
void run_pot();
void run_menu(bool pot_x_moved);

void end_menu();
void end_test();

bool update_sw();
float pot_update_val(int pin, float *val_ptr);
bool pot_in_dead_zone(float pot_val);
void set_servo(int val);

void print_instructions();
void print_menu();

// State variables
Servo servo;
int state;
int menu_option;
bool sw;
float pot_x;
int servo_val;
bool servo_dir_left;

/**
 * Sets up the servo and serial
 */
void setup() {
	// Initialize pins
	pinMode(SW_PIN, INPUT);
	servo.attach(SERVO_PIN);

	// Initialize state variables
	state = MENU;
	menu_option = MENU + 1;
	sw = false;
	servo_dir_left = true;

	// Print instructions
	Serial.begin(SERIAL_SPEED);
	print_instructions();
}

/**
 * Reads the potentiometer, maps it to the servo range, prints its value, and sends that to the servo
 * Waits `DELAY`, then repeats
 */
void loop() {
	// Update state
	bool pot_x_moved = pot_update_val(POT_X_PIN, &pot_x);
	bool sw_pressed = update_sw();

	if (sw_pressed) {  // Button press
		if (state) {
			end_test();
		} else {
			end_menu();
		}
	} else {  // Otherwise
		switch (state) {
			case MENU:
				{
					run_menu(pot_x_moved);
					break;
				}
			case POT:
				{
					run_pot();
					break;
				}
			case ZERO:
				{
					run_zero();
					break;
				}
			case RANGE:
				{
					run_range();
					break;
				}
		}
	}

	delay(DELAY);
}

/**
 * Sets the servo to a given value
 *
 * @param val The value to set it to, in microseconds
 */
void set_servo(int val) {
	servo_val = val;
	servo.writeMicroseconds(val);
}

/**
 * Called every loop while in the range test
 */
void run_range() {
	if (servo_dir_left) {                             // heading left
		if (servo_val <= SERVO_CENTER - SERVO_DEV) {  // at minimum
			servo_dir_left = false;
			Serial.println("Maximum");
		} else {
			set_servo(servo_val - SERVO_STEP);
		}
	} else {                                          // heading right
		if (servo_val >= SERVO_CENTER + SERVO_DEV) {  // at maximum
			servo_dir_left = true;
			Serial.println("Minimum");
		} else {
			set_servo(servo_val + SERVO_STEP);
		}
	}
}

/**
 * Called every loop while in the zero test
 */
void run_zero() {
	set_servo(SERVO_CENTER);
}

/**
 * Called every loop while in the potentiometer test
 */
void run_pot() {
	int servo_in = (int)(pot_x * SERVO_DEV + SERVO_CENTER);
	Serial.println(servo_in);
	set_servo(servo_in);
}

/**
 * Prints the instructions for usage
 */
void print_instructions() {
	Serial.println("Point the joystick left or right to navigate the menu. Press down to select a test to run.");
	Serial.println("Press again to exit the test.");
}

/**
 * Prints the current state of the menu, indicating which option is being "hovered over"
 */
void print_menu() {
	Serial.println("-----");
	for (size_t i = MENU + 1; i < END; i++) {
		if (menu_option == i) {  // Star the hovered option
			Serial.print("* ");
		} else {
			Serial.print("  ");
		}
		Serial.print(MENU_OPTIONS[i]);
		Serial.print("  ");
	}
	// Print additional info about the hovered option
	Serial.println("\n");
	Serial.print("  * ");
	Serial.println(MENU_DESCRIPTIONS[menu_option]);
}

/**
 * Called every loop while in the menu
 * 
 * @param pot_x_moved True if the potentiometer's x has newly moved
 * 		away from zero this loop cycle
 */
void run_menu(bool pot_x_moved) {
	if (pot_x_moved) {
		if (pot_x > 0) {  // move right
			menu_option++;
			if (menu_option >= END) {
				menu_option = MENU + 1;
			}
		} else if (pot_x < 0) {  // move left
			menu_option--;
			if (menu_option <= MENU) {
				menu_option = END - 1;
			}
		}
		print_menu();
	}
}

/**
 * Called to exit the menu
 */
void end_menu() {
	state = menu_option;
}

/**
 * Called to end any test
 */
void end_test() {
	state = MENU;
	print_menu();
}

/**
 * Updates the global switch variable
 * 
 * @return True if the button has been newly pressed down in this loop cycle
 */
bool update_sw() {
	bool curr_sw = digitalRead(SW_PIN) == HIGH;
	bool sw_pressed = curr_sw && !sw;
	sw = curr_sw;
	return sw_pressed;
}

/**
 * Determines if the potentiometer is in the dead zone, where values should be treated as
 * zero, not minimal
 *
 * @param pot_val The value of the potentiometer, on the [-1, 1] scale
 * @return True if it's in the dead zone, false otherwise
 */
bool pot_in_dead_zone(float pot_val) {
	return pot_val < POT_DEAD_ZONE && pot_val > -POT_DEAD_ZONE;
}

/**
 * Gets the value at a pin, assuming a potentiometer is hooked up to it
 * Maps it to between [-1, 1] and applies a dead zone
 * 
 * @param pin The pin the potentiometer is plugged into
 * @param val_ptr Pointer to where the value of the potentiometer is stored
 * @return True if the potentiometer has newly moved from zero in this loop cycle
 */
float pot_update_val(int pin, float *val_ptr) {
	const float MIN = -1;
	const float MAX = 1;

	float pot_val = (float)analogRead(pin);
	// Anchors between MIN and MAX. Cubing is more intuitive for human control
	pot_val = pow(pot_val / POT_MAX * (MAX - MIN) + MIN, 3);

	if (pot_in_dead_zone(pot_val)) {
		return 0;
	}

	bool pot_moved = pot_val && !(*val_ptr);
	*val_ptr = pot_val;
	return pot_moved;
}