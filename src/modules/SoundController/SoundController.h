#pragma once
#include "../Logger/Logger.h"

/**
 * @class SoundController
 * @brief Generates a dynamic engine sound based on the final motor command.
 *
 * This module takes the final motor speed (-255 to 255) and calculates both
 * the RPM (pitch) and engine load (timbre/aggressiveness) internally. It uses
 * I2S and DMA for non-blocking, real-time audio synthesis.
 */
class SoundController {
public:
  SoundController(int bclkPin, int lrcPin, int dinPin, Logger* logger);
  void setup();
  void update();

  /**
   * @brief Sets the target motor command, which drives the entire sound simulation.
   * @param speed The final motor speed command (-255 to 255).
   */
  void setMotorCommand(int speed);

private:
  int _bclkPin;
  int _lrcPin;
  int _dinPin;
  Logger* _logger;

  // Sound generation variables
  int _targetSpeed = 0;
  float _currentSpeed = 0; // Use float for smoother transitions
  float _previousSpeed = 0;
  
  float _phase = 0.0;
  float _lfoPhase = 0.0;

  void fillBuffer();
};
