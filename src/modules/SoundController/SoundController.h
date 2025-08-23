#pragma once
#include "../Logger/Logger.h"

/**
 * @class SoundController
 * @brief Generates a dynamic, load-based engine sound.
 *
 * This module uses I2S and DMA to generate a non-blocking sine wave. The pitch
 * is controlled by the RPM, and the timbre (aggressiveness) is controlled by
 * the engine load, which modulates the frequency for a more realistic sound.
 */
class SoundController {
public:
  SoundController(int bclkPin, int lrcPin, int dinPin, Logger* logger);
  void setup();
  void update();

  /**
   * @brief Sets the target engine state (RPM and Load).
   * @param rpm The current speed of the car (0-100).
   * @param load The current load on the engine (0-100).
   */
  void setEngineState(int rpm, int load);

private:
  int _bclkPin;
  int _lrcPin;
  int _dinPin;
  Logger* _logger;

  // Sound generation variables
  int _targetRpm = 0;
  int _targetLoad = 0;
  int _currentRpm = 0;
  float _phase = 0.0;
  float _lfoPhase = 0.0; // Phase for the "wobble" wave (LFO)

  void fillBuffer();
};
