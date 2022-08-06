#include <algorithm>
#include <cmath>

// Note, the logic and math behind these functions can be found here: https://lattice22.atlassian.net/wiki/spaces/L/pages/37126159/ESCON+Motor+Control

constexpr int kPWM = 2;
constexpr int kEnable = 50;
constexpr int kSigPin = 0;

double kRPMCommand = 0;
int direction = 1;


// Percent output * max bit
constexpr double kMaxRPM = 1000;


float measuredSpeed = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  // Requires a digital signal to enable motor controller
  pinMode(kEnable, OUTPUT);
  digitalWrite(kEnable, LOW);
  

}

/**
 * Scales the rpm into a percent output within the deadband range.
 *
 * @param rpm The desired RPM 
 * @return The RPM in percent output between 10% and 90%
 */
double scale_percent_output(double rpm) {
  double percent_output = std::abs(rpm) / kMaxRPM / 2;
  double add = copysign(percent_output, rpm);
  return (0.5 + add)* 0.8 + .1;
}

/**
 * Scales a percent input coming from the motor to the RPM value.
 *
 * @param percent_input Percent input read from the port
 * @return The RPM
 */
double scale_input(double percent_input) {
  return kMaxRPM * ((percent_input) * 2 - 1);
}

void loop() {

  analogReadResolution(12);
  // Converting output into bits to command over the PWM port
  double kPWMOutput = scale_percent_output(direction * kRPMCommand) * 4095;

  // Converting input into sensor reading value
  measuredSpeed = scale_input((double)analogRead(kSigPin) / 4095.0);
  analogWriteResolution(12);
  
  analogWrite(kPWM, kPWMOutput);

  
  Serial.println(measuredSpeed);
  if (Serial.available()) {
    char input = (uint8_t) Serial.read();

    // Uses different input characters to change states of the motor
    if (input == 'a') {
      digitalWrite(kEnable, LOW);
    }
    if (input == 'b') {
      digitalWrite(kEnable, HIGH);
    }
    if (input =='c') {
      kRPMCommand = 200;
    }
    if (input =='d') {
      kRPMCommand = 500;
    }
    if (input =='e') {
      kRPMCommand = 900;
    }
    if (input =='f') {
      kRPMCommand = 0;
    }
    if (input =='g') {
      direction *= -1;
    }
  }
}
