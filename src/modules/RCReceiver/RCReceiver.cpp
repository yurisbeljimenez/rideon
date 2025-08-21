#include "RCReceiver.h"
#include <Arduino.h>

RCReceiver::RCReceiver(int throttlePin, int steeringPin, Logger* logger) {
  _throttlePin = throttlePin;
  _steeringPin = steeringPin;
  _logger = logger;
}

void RCReceiver::setup() {
  pinMode(_throttlePin, INPUT);
  pinMode(_steeringPin, INPUT);
  if (_logger) _logger->log("Initialized");
}

void RCReceiver::update() {
  // Read the pulse width from each channel.
  _rawThrottle = pulseIn(_throttlePin, HIGH, 25000);
  _rawSteering = pulseIn(_steeringPin, HIGH, 25000);

  // Failsafe: if pulseIn times out (returns 0), reset to neutral.
  if (_rawThrottle == 0) _rawThrottle = 1500;
  if (_rawSteering == 0) _rawSteering = 1500;

  if (_logger) {
    // A trick to log two values in one message to reduce spam.
    // e.g., 1500 throttle and 1510 steering becomes 15001510.
    _logger->log(_rawThrottle * 10000 + _rawSteering);
  }
}

// Maps the raw 1000-2000µs value to a user-friendly -100 to 100 range.
int RCReceiver::getThrottle() {
  return map(_rawThrottle, 1000, 2000, -100, 100);
}

int RCReceiver::getSteering() {
  return map(_rawSteering, 1000, 2000, -100, 100);
}

// Checks if the throttle is actively being used by the parent.
bool RCReceiver::isOverriding() {
  return (_rawThrottle < _neutralMin || _rawThrottle > _neutralMax);
}