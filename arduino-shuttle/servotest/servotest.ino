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
const int SW_PIN = 6;      // the digital pin the switch is plugged into

// Programatic constants
const int SERIAL_SPEED = 9600;  // bits per second, align with Tools -> Serial Monitor's dropdown menu
const int DELAY = 50;           // milliseconds

const float POT_ANALOG_MAX = 1023;  // max value of the potentiometer (min is assumed 0)
const float POT_MAX = 1;
float pot_dead_zone;  // on the [-POT_MAX, POT_MAX] scale
float pot_vel_factor;      // >0 if the pot should affect velocity instead of position in the Pot test

// Servo constants
float servo_center;  // center position, in microseconds
float servo_dev;     // how far the min/max are from the center, in microseconds
float servo_step;    // the size of step taken by the Range test, in microseconds

const char SPACER[] = "-----";

// Menu and test definitions
typedef enum state {
	MENU,
	POT,
	ZERO,
	RANGE,
	SETTINGS,
	QUIT,
	END
} state_t;

const char *MENU_OPTIONS[] = { "Menu", "Potentiometer", "Zero", "Range", "Settings", "Quit" };
const char *MENU_DESCRIPTIONS[] = { "Menu",
									"Runs the servo at a value set by the potentiometer. The value is displayed.",
									"Sets the servo to the zero position.",
									"Runs the servo from its minimum value to its maximum and back. Be careful it won't hit anything!",
									"Change servo or potentiometer settings.",
									"Stops the test. Be sure to zero first, if needed." };

// Settings menu and definitions
typedef enum settings {
	SETTINGS_MENU,
	DEAD_ZONE,
	VEL,
	CENTER,
	DEV,
	STEP,
	EXIT,
	SETTINGS_END
} settings_t;

// TODO: Add ability to flip potentiometer direction and change servo port

const char *SETTINGS_OPTIONS[] = { "Menu", "Pot dead zone", "Pot velocity factor", "Servo center", "Servo min/max",
		"Servo step", "Exit" };
const char *SETTINGS_DESCRIPTIONS[] = { "Menu", "The range in which small potentiometer values aren't registered.",
		"0: Potentiometer sets servo position. 1+: Potentiometer sets servo velocity, by this factor."
		"The zero point of the servo, in microseconds.",
		"How far the servo can get from its center, in microseconds.",
		"The size of the step taken by the Range test, in microseconds.",
		"Exit settings, saving all changes." };

// Function headers
void run_range();
void run_zero();
void run_pot();
void run_menu(bool pot_x_moved);
void run_settings(bool pot_x_moved);
void run_settings_set(bool pot_x_moved);
void run_settings_menu(bool pot_x_moved);

void end_menu();
void end_test();
void end_settings_menu();
void end_settings_set();

void print_instructions();
void print_any_menu(const char *options[], const char *descriptions[], float *vals_ptrs[],
					size_t num_options, size_t curr_option);
void print_menu();
void print_settings();
void print_settings_set();

float pot_update_val(int pin, float *val_ptr);
bool pot_in_dead_zone(float pot_val);
bool pot_adjust_var(bool pot_moved, float pot_val, float *var_ptr, float step,
					float min, float max, bool wrap);
bool pot_adjust_int(bool pot_moved, float pot_val, int *var_ptr, int step,
					int min, int max, bool wrap);

bool update_sw();
void set_servo(int val);
void increment_servo(int val);

float *settings_get_var_ptr(int setting);

// State variables
Servo servo;
int state;
int menu_option;
int settings_state;
int settings_option;

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
	digitalWrite(SW_PIN, HIGH);
	servo.attach(SERVO_PIN);

	// Initialize state variables
	state = MENU;
	menu_option = MENU + 1;
	settings_state = SETTINGS_MENU;
	settings_option = SETTINGS_MENU + 1;

	sw = false;
	pot_x = 0;
	servo_val = 0;
	servo_dir_left = true;

	pot_dead_zone = 0.05;
	pot_vel_factor = 0;

	servo_center = 1500;
	servo_dev = 500;
	servo_step = 50;

	// Print instructions
	Serial.begin(SERIAL_SPEED);
	delay(DELAY);
	print_instructions();
}

/**
 * Reads the potentiometer, maps it to the servo range, prints its value, and sends that to the servo
 * Waits `DELAY`, then repeats
 */
void loop() {
	if (state == QUIT) {
		return;
	}
	
	// Update state
	bool pot_x_moved = pot_update_val(POT_X_PIN, &pot_x);
	bool sw_pressed = update_sw();

	if (Serial.available()) {
		char input = (uint8_t) Serial.read();
		switch (input) {
			case ' ': {
				sw_pressed = true;
				break;
			}
			case 'A':
			case 'a': {
				pot_x_moved = true;
				pot_x = -POT_MAX;
				break;
			}
			case 'd':
			case 'D': {
				pot_x_moved = true;
				pot_x = POT_MAX;
				break;
			}
			default: {
				break;
			}
		}
	}

	if (sw_pressed) {  // Button press
		switch (state) {
			case MENU:
				{
					end_menu();
					break;
				}
			case SETTINGS:
				{
					if (settings_state) {
						end_settings_set();
					} else {
						end_settings_menu();
					}
					break;
				}
			default:
				{
					end_test();
					break;
				}
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
			case SETTINGS:
				{
					run_settings(pot_x_moved);
					break;
				}
		}
	}

	delay(DELAY);
}

/***********
 * RUNNERS *
 ***********/

/**
 * Called every loop while in the range test
 */
void run_range() {
	if (servo_dir_left) {                             // heading left
		if (servo_val <= servo_center - servo_dev) {  // at minimum
			servo_dir_left = false;
			Serial.println("Maximum");
		} else {
			set_servo(servo_val - servo_step);
		}
	} else {                                          // heading right
		if (servo_val >= servo_center + servo_dev) {  // at maximum
			servo_dir_left = true;
			Serial.println("Minimum");
		} else {
			set_servo(servo_val + servo_step);
		}
	}
}

/**
 * Called every loop while in the zero test
 */
void run_zero() {
	set_servo(servo_center);
}

/**
 * Called every loop while in the potentiometer test
 */
void run_pot() {
	if (pot_vel_factor) {
		int servo_in = (int) (pot_x * pot_vel_factor);
		increment_servo(servo_in);

		if (pot_x) {
			Serial.println(servo_val);
		}
	}
	else {
		int servo_in = (int)(pot_x * servo_dev + servo_center);
		set_servo(servo_in);

		if (pot_x) {
			Serial.println(servo_in);
		}
	}
}

/**
 * Called every loop while in the menu
 * 
 * @param pot_x_moved True if the potentiometer's x has newly moved
 * 		away from zero this loop cycle
 */
void run_menu(bool pot_x_moved) {
	if (pot_adjust_int(pot_x_moved, pot_x, &menu_option, 1, MENU + 1, END - 1, true)) {
		print_menu();
	}
}

/**
 * Called every loop while in the settings
 * 
 * @param pot_x_moved True if the potentiometer's x has newly moved from zero this loop cycle
 */
void run_settings(bool pot_x_moved) {
	if (settings_state) {
		run_settings_set(pot_x_moved);
	} else {
		run_settings_menu(pot_x_moved);
	}
}

/**
 * Called every loop while changing a setting
 * 
 * @param pot_x_moved True if the potentiometer's x has newly moved from zero this loop cycle
 */
void run_settings_set(bool pot_x_moved) {
	if (!pot_x_moved) {  // No need to find settings constraints
		return;
	}

	float step = 0;
	float min = 0;
	float max = 0;

	// These are just hard-coded
	switch (settings_state) {
		case DEAD_ZONE:
			{
				step = 0.01;
				max = 1;
				break;
			}
		case VEL: {
			step = 1;
			max = 10;
			break;
		}
		case CENTER:
			{
				step = 100;
				max = 3000;
				break;
			}
		case DEV:
			{
				step = 100;
				max = servo_center;
				break;
			}
		case STEP:
			{
				step = 10;
				max = servo_dev * 2;
				break;
			}
	}
	if (pot_adjust_var(pot_x_moved, pot_x, settings_get_var_ptr(settings_state), step, min, max, false)) {
		print_settings_set();
	}
}

/**
 * Called every loop while in the settings menu
 * 
 * @param pot_x_moved True if the potentiometer's x has newly moved from zero this loop cycle
 */
void run_settings_menu(bool pot_x_moved) {
	if (pot_adjust_int(pot_x_moved, pot_x, &settings_option, 1, SETTINGS_MENU + 1, SETTINGS_END - 1, true)) {
		print_settings();
	}
}

/**********
 * ENDERS *
 **********/

/**
 * Called to exit the settings menu (either back to the main menu,
 * or into a setting to adjust it)
 */
void end_settings_menu() {
	Serial.println(SPACER);
	if (settings_option == EXIT) {  // exit case
		Serial.println("Settings saved.");
		settings_state = SETTINGS_MENU;
		settings_option = SETTINGS_MENU + 1;

		state = MENU;
		print_menu();
	} else {
		settings_state = settings_option;

		Serial.print(SETTINGS_OPTIONS[settings_state]);
		Serial.println(":");
		Serial.println(SETTINGS_DESCRIPTIONS[settings_state]);
		Serial.println("");

		print_settings_set();
	}
}

/**
 * Called to exit from adjusting a specific setting, back to the settings menu
 */
void end_settings_set() {
	Serial.print(SETTINGS_OPTIONS[settings_state]);
	Serial.println(" saved.");

	settings_state = MENU;
	print_settings();
}

/**
 * Called to exit the menu
 */
void end_menu() {
	state = menu_option;

	// Print transition to test
	switch (state) {
		case SETTINGS:
			{
				print_settings();
				break;
			}
		case QUIT:
			{
				Serial.println(SPACER);
				Serial.println("Servo test stopped. Press RESET on Arduino to restart.");
				break;
			}
		default:
			{
				Serial.println(SPACER);
				Serial.print(MENU_OPTIONS[state]);
				Serial.println(" test running.");
				break;
			}
	}
}

/**
 * Called to end any test
 */
void end_test() {
	// Print transition from test
	Serial.print(MENU_OPTIONS[state]);
	Serial.println(" test stopped.");

	state = MENU;
	print_menu();
}

/************
 * PRINTERS *
 ************/

/**
 * Prints the instructions for usage
 */
void print_instructions() {
	Serial.println("Point the joystick left or right to navigate the menu. Press down to select a test to run.");
	Serial.println("Press again to exit the test.");
	print_menu();
}

/**
 * Prints the current state of a given menu, indicating which option is being "hovered over"
 * Doesn't print option 0.
 * 
 * @param options The options for the menu to have
 * @param descriptions The descriptions of each option
 * @param vals_ptrs The pointers to values to display next to each option, if applicable. NULL to exclude
 * @param num_options How many options there are (including option 0, even though it isn't shown)
 * @param curr_option The option to star as being "hovered over"
 */
void print_any_menu(const char *options[], const char *descriptions[], float *vals_ptrs[],
					size_t num_options, size_t curr_option) {
	Serial.println(SPACER);
	for (size_t i = 1; i < num_options; i++) {
		if (curr_option == i) {  // Star the hovered option
			Serial.print("* ");
		} else {
			Serial.print("  ");
		}
		Serial.print(options[i]);

		// Display value, if known
		if (vals_ptrs != NULL && vals_ptrs[i] != NULL) {
			Serial.print(" (");
			Serial.print(*(vals_ptrs[i]));
			Serial.print(")");
		}
		Serial.print("  ");
	}
	// Print additional info about the hovered option
	Serial.println("\n");
	Serial.print("  * ");
	Serial.println(descriptions[curr_option]);
}

/**
 * Prints the current state of the setting being adjusted
 */
void print_settings_set() {
	Serial.print("< ");
	Serial.print(*settings_get_var_ptr(settings_state));
	Serial.println(" >");
}

/**
 * Prints the current state of the settings, indicating which option is being "hovered over"
 */
void print_settings() {
	float *vars_ptrs[SETTINGS_END];
	for (size_t i = 0; i < SETTINGS_END; i++) {
		vars_ptrs[i] = settings_get_var_ptr(i);
	}
	print_any_menu(SETTINGS_OPTIONS, SETTINGS_DESCRIPTIONS, vars_ptrs, SETTINGS_END, settings_option);
}

/**
 * Prints the current state of the menu, indicating which option is being "hovered over"
 */
void print_menu() {
	print_any_menu(MENU_OPTIONS, MENU_DESCRIPTIONS, NULL, END, menu_option);
}

/***********
 * HELPERS *
 ***********/

/**
 * Increments the servo's current position by a given value
 * 
 * @param val The value to increment it by, in microseconds
 */
void increment_servo(int val) {
	servo_val += val;

	if (servo_val > servo_center + servo_dev) {
		servo_val = servo_center + servo_dev;
	}
	if (servo_val < servo_center - servo_dev) {
		servo_val = servo_center - servo_dev;
	}

	servo.writeMicroseconds(servo_val);
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
	return pot_val < pot_dead_zone && pot_val > -pot_dead_zone;
}

/**
 * Gets the value at a pin, assuming a potentiometer is hooked up to it
 * Maps it to between [-POT_MAX, POT_MAX] and applies a dead zone
 * 
 * @param pin The pin the potentiometer is plugged into
 * @param val_ptr Pointer to where the value of the potentiometer is stored
 * @return True if the potentiometer has newly moved from zero in this loop cycle
 */
float pot_update_val(int pin, float *val_ptr) {
	float pot_val = (float)analogRead(pin);
	// Anchors between MIN and MAX. Cubing is more intuitive for human control
	pot_val = pow(pot_val / POT_ANALOG_MAX * (POT_MAX * 2) - POT_MAX, 3);

	if (pot_in_dead_zone(pot_val)) {
		pot_val = 0;
	}

	bool pot_moved = pot_val && !(*val_ptr);
	*val_ptr = pot_val;
	return pot_moved;
}

/**
 * Allows for a float variable to be adjusted in discrete steps by a potentiometer
 * 
 * @param pot_moved True if the potentiometer has newly moved from zero, this loop cycle
 * @param pot_val The value of the potentiometer
 * @param var_ptr A pointer to the variable to be adjusted
 * @param step How much to adjust it by
 * @param min The minimum value which it can have
 * @param max The maximum value which it can have
 * @param wrap True if exceeding the max should go to the min (and vice versa), false if
 * 		it should be capped instead.
 * @return True if the variable has been adjusted, false otherwise
 */
bool pot_adjust_var(bool pot_moved, float pot_val, float *var_ptr, float step,
					float min, float max, bool wrap) {
	if (pot_moved) {
		if (pot_val > 0) {  // move right
			(*var_ptr) += step;
			if (*var_ptr > max) {
				*var_ptr = wrap ? min : max;
			}
			return true;
		} else if (pot_val < 0) {  // move left
			(*var_ptr) -= step;
			if (*var_ptr < min) {
				*var_ptr = wrap ? max : min;
			}
			return true;
		}
	}
	return false;
}

/**
 * Allows for an integer variable to be adjusted in discrete steps by a potentiometer
 * 
 * @param pot_moved True if the potentiometer has newly moved from zero, this loop cycle
 * @param pot_val The value of the potentiometer
 * @param var_ptr A pointer to the variable to be adjusted
 * @param step How much to adjust it by
 * @param min The minimum value which it can have
 * @param max The maximum value which it can have
 * @param wrap True if exceeding the max should go to the min (and vice versa), false if
 * 		it should be capped instead.
 * @return True if the variable has been adjusted, false otherwise
 */
bool pot_adjust_int(bool pot_moved, float pot_val, int *var_ptr, int step,
					int min, int max, bool wrap) {
	float var_float = (float)(*var_ptr);
	bool adjusted = pot_adjust_var(pot_moved, pot_val, &var_float, (float)step, (float)min, (float)max, wrap);
	*var_ptr = round(var_float);

	return adjusted;
}

/**
 * Gets the pointer to the value corresponding to a given setting
 * 
 * @param setting The setting to loop for
 * @return A pointer to that setting's value if it exists, else NULL
 */
float *settings_get_var_ptr(int setting) {
	switch (setting) {
		case DEAD_ZONE:
			{
				return &pot_dead_zone;
			}
		case VEL: {
			return &pot_vel_factor;
		}
		case CENTER:
			{
				return &servo_center;
			}
		case DEV:
			{
				return &servo_dev;
			}
		case STEP:
			{
				return &servo_step;
			}
		default:
			{
				return NULL;
			}
	}
}