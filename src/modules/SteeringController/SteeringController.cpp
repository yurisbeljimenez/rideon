#include "SteeringController.h"

SteeringController::SteeringController(int servoPin, Logger* logger) {
  _servoPin = servoPin;
  _logger = logger;
}

void SteeringController::setup() {
  // Associate the Servo object with the physical GPIO pin.
  //
  // The CL9030 is an ESC and expects standard 1000-2000 µs servo pulses.
  // The ESP32Servo default range is 500-2500 µs, which would command
  // out-of-spec pulses at full lock, so set the range explicitly.
  _servo.attach(_servoPin, 1000, 2000);
  // Center the steering on startup.
  setSteering(0);
  if (_logger) _logger->log("Initialized");
}

void SteeringController::setSteering(int direction) {
  direction = constrain(direction, -100, 100);

  // Map our desired control range (-100 to 100) to the servo's
  // expected command range (0 to 180 degrees). With the 1000-2000 µs
  // attach range, 90° is exactly 1500 µs (ESC center).
  int servoCommand = map(direction, -100, 100, 0, 180);

  // Send the command to the ESC.
  _servo.write(servoCommand);
  if (_logger) _logger->log(direction);
}