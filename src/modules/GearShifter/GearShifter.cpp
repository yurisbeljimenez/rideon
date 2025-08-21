#include "GearShifter.h"
#include <Arduino.h>

GearShifter::GearShifter(int pin, Logger* logger) {
  _pin = pin;
  _logger = logger;
}

void GearShifter::setup() {
  pinMode(_pin, INPUT_PULLUP);
  if (_logger) {
    _logger->log("FORWARD"); // Log initial gear
  }
}

void GearShifter::update() {
  int currentButtonState = digitalRead(_pin);

  // Detect the moment the button goes from HIGH to LOW (a press event).
  if (_lastButtonState == HIGH && currentButtonState == LOW) {
    // Toggle the gear state.
    if (_currentGear == Gear::FORWARD) {
      _currentGear = Gear::REVERSE;
      if (_logger) _logger->log("REVERSE");
    } else {
      _currentGear = Gear::FORWARD;
      if (_logger) _logger->log("FORWARD");
    }
    delay(50); // Simple debounce to prevent multiple triggers.
  }

  _lastButtonState = currentButtonState;
}

Gear GearShifter::getGear() {
  return _currentGear;
}