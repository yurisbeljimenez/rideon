#pragma once
#include "../Logger/Logger.h"

// Scoped enum for the possible gear states.
enum class Gear {
  FORWARD,
  REVERSE
};

/**
 * @class GearShifter
 * @brief Manages the car's gear state (Forward/Reverse) via a button.
 *
 * Detects a button press and toggles the gear state.
 */
class GearShifter {
public:
  /**
   * @brief Constructor for the GearShifter.
   * @param pin The GPIO pin the gear shifter button is connected to.
   * @param logger A pointer to a Logger object for debugging.
   */
  GearShifter(int pin, Logger* logger);

  void setup();
  void update(); // Checks for a button press to toggle the gear.
  Gear getGear();  // Returns the current gear (Gear::FORWARD or Gear::REVERSE).

private:
  int _pin;
  Logger* _logger;
  Gear _currentGear = Gear::FORWARD; // Default to Forward gear.

  // For detecting a single press event.
  int _lastButtonState = HIGH; 
};