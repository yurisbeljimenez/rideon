#pragma once
#include "../Logger/Logger.h"
#include <ESP32Servo.h>

/**
 * @class SteeringController
 * @brief Controls the steering ESC using an RC-style servo signal.
 *
 * Uses the ESP32Servo library to generate the precise pulse widths
 * needed to command the steering motor left, right, or center.
 */
class SteeringController {
public:
  /**
   * @brief Constructor for the SteeringController.
   * @param servoPin The GPIO pin connected to the steering ESC's signal wire.
   * @param logger A pointer to a Logger object for debugging.
   */
  SteeringController(int servoPin, Logger* logger);

  void setup();
  // Accepts a value from -100 (full left) to 100 (full right).
  void setSteering(int direction);

private:
  int _servoPin;
  Logger* _logger;
  Servo _servo; // An instance of the ESP32Servo library's Servo class.
};