#include "SteeringController.h"

SteeringController::SteeringController(int servoPin, Logger* logger) {
  _servoPin = servoPin;
  _logger = logger;
}

void SteeringController::setup() {
  // Associate the Servo object with the physical GPIO pin.
  _servo.attach(_servoPin);
  // Center the steering on startup.
  setSteering(0);
  if (_logger) _logger->log("Initialized");
}

void SteeringController::setSteering(int direction) {
  direction = constrain(direction, -100, 100);

  // Map our desired control range (-100 to 100) to the servo's
  // expected command range (0 to 180 degrees).
  int servoCommand = map(direction, -100, 100, 0, 180);

  // Send the command to the ESC.
  _servo.write(servoCommand);
  if (_logger) _logger->log(direction);
}