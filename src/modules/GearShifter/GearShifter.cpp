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
    unsigned long currentMillis = millis();
    
    // Non-blocking debounce: only process if enough time has passed
    if (currentMillis - _lastDebounceTime >= _debounceDelay) {
      // Toggle the gear state.
      if (_currentGear == Gear::FORWARD) {
        _currentGear = Gear::REVERSE;
        if (_logger) _logger->log("REVERSE");
      } else {
        _currentGear = Gear::FORWARD;
        if (_logger) _logger->log("FORWARD");
      }
      
      // Update debounce timer to prevent multiple triggers
      _lastDebounceTime = currentMillis;
    }
  }

  _lastButtonState = currentButtonState;
}

Gear GearShifter::getGear() {
  return _currentGear;
}