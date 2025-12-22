#pragma once
#include <Adafruit_NeoPixel.h>
#include "../Logger/Logger.h"
#include "./Shared/CarState.h"

/**
 * @class SystemStatus
 * @brief Controls the onboard RGB LED to indicate car status.
 *
 * This module manages a NeoPixel LED that provides visual feedback
 * about the car's current operational state. Different colors and
 * patterns indicate different driving conditions:
 * - Solid Blue: Stopped
 * - Solid Green: Moving Forward
 * - Solid Yellow: Moving in Reverse
 * - Pulsing Red: Avoiding Obstacle
 * - Pulsing White: Manual Override
 */
class SystemStatus {
public:
  /**
   * @brief Constructor for the SystemStatus class.
   * @param pin The GPIO pin connected to the NeoPixel LED.
   * @param logger A pointer to a Logger object for debugging.
   */
  SystemStatus(int pin, Logger* logger);
  
  /**
   * @brief Initialize the LED strip and prepare for operation.
   */
  void setup();
  
  /**
   * @brief Update the LED display based on current car state.
   * @param state The current CarState to reflect visually.
   */
  void update(CarState state);

private:
  int _pin;                    // GPIO pin connected to the LED
  Logger* _logger;             // Logger for debugging output
  Adafruit_NeoPixel _pixel;    // NeoPixel LED strip object

  /**
   * @brief Set LED to a solid color (no pulsing).
   * @param color The RGB color value to set.
   */
  void setSolidColor(uint32_t color);
  
  /**
   * @brief Set LED to pulse between color and off state.
   * @param color The RGB color value for pulsing effect.
   */
  void setPulsingColor(uint32_t color);

  unsigned long _previousMillis = 0;  // Timestamp for pulse timing
  bool _pulseState = true;            // Current state of pulsing (on/off)
  const int _pulseInterval = 250;     // Interval between pulses in milliseconds
};
