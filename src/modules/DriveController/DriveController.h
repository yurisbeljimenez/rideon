#pragma once
#include "../Logger/Logger.h"

/**
 * @class DriveController
 * @brief Controls the Cytron MDDS30 drive motor controller.
 *
 * Takes a speed command (-255 to 255) and generates the appropriate
 * Direction (digital) and Speed (hardware PWM) signals.
 */
class DriveController {
public:
  /**
   * @brief Constructor for the DriveController.
   * @param dirPin The GPIO pin for the Direction signal.
   * @param pwmPin The GPIO pin for the PWM Speed signal.
   * @param pwmChannel The ESP32 LEDC hardware channel to use (0-15).
   * @param logger A pointer to a Logger object for debugging.
   */
  DriveController(int dirPin, int pwmPin, int pwmChannel, Logger* logger);

  void setup();
  void setSpeed(int speed); // Sets the motor speed and direction.

private:
  int _dirPin;
  int _pwmPin;
  int _pwmChannel;
  Logger* _logger;
};