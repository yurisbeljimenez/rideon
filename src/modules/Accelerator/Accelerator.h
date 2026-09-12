#pragma once
#include <Arduino.h>
#include "../Logger/Logger.h"

/**
 * @class Accelerator
 * @brief Reads the accelerator pedal and reports a calibrated 0..100 intent value.
 *
 * This class owns only the analog front-end: it reads the pedal, applies a
 * moving-average filter to reject noise, and maps the calibrated ADC range to
 * a 0..100 "pedal intent".
 *
 * The 3-stage acceleration/braking RAMP that shapes the actual motor speed
 * lives in the testable control state machine (see src/Shared/ControlLogic.h),
 * so the pedal readout stays a pure sensor abstraction.
 */
class Accelerator {
public:
  /**
   * @brief Constructor for the Accelerator.
   * @param pedalPin The GPIO pin the analog pedal is connected to.
   * @param logger A pointer to a Logger object for debugging.
   * @param adcMin Calibrated minimum ADC value of the pedal (0..4095).
   * @param adcMax Calibrated maximum ADC value of the pedal (0..4095).
   */
  Accelerator(int pedalPin, Logger* logger, int adcMin = 0, int adcMax = 4095);

  /**
   * @brief Initialize the accelerator.
   */
  void setup();

  /**
   * @brief Update the pedal reading and filtering (call once per loop).
   */
  void update();

  /**
   * @brief Get the current pedal intent.
   * @return Pedal position 0..100 (filtered, calibrated).
   */
  int getPedalIntent();

private:
  int _pedalPin;                  // GPIO pin connected to the analog pedal
  Logger* _logger;                // Logger for debugging output
  const int _adcMin;              // Calibrated ADC minimum
  const int _adcMax;              // Calibrated ADC maximum

  int _pedalIntent = 0;           // Current filtered, mapped intent (0..100)

  // Analog filtering (fixed-size buffer, no heap allocation)
  static const int FILTER_WINDOW = 8;
  int _filterBuffer[FILTER_WINDOW];
  int _filterIndex = 0;
  long _filterSum = 0;

  // Intelligent logging state
  const int LOGGING_THRESHOLD = 5;
  bool _wasLogging = false;
};
