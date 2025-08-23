#include "RCReceiver.h"
#include <Arduino.h>

// The constructor now accepts and saves the dead zone calibration values for both channels.
RCReceiver::RCReceiver(int throttlePin, int steeringPin, Logger* logger, int throttleNeutralMin, int throttleNeutralMax, int steeringNeutralMin, int steeringNeutralMax) :
  _throttleNeutralMin(throttleNeutralMin),
  _throttleNeutralMax(throttleNeutralMax),
  _steeringNeutralMin(steeringNeutralMin),
  _steeringNeutralMax(steeringNeutralMax)
{
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
  _rawThrottle = pulseIn(_throttlePin, HIGH, 25000);
  _rawSteering = pulseIn(_steeringPin, HIGH, 25000);

  if (_rawThrottle == 0) _rawThrottle = 1500;
  if (_rawSteering == 0) _rawSteering = 1500;

  // --- Truly Intelligent Logging ---
  // Only log the RC values if the throttle OR the steering is being used.
  if (_logger && (isOverriding() || isSteeringActive())) {
    _logger->log(_rawThrottle * 10000 + _rawSteering);
  }
}

int RCReceiver::getThrottle() {
  return map(_rawThrottle, 1000, 2000, -100, 100);
}

int RCReceiver::getSteering() {
  return map(_rawSteering, 1000, 2000, -100, 100);
}

// Checks if the throttle is actively being used.
bool RCReceiver::isOverriding() {
  return (_rawThrottle < _throttleNeutralMin || _rawThrottle > _throttleNeutralMax);
}

// NEW METHOD: Checks if the steering is actively being used.
bool RCReceiver::isSteeringActive() {
  return (_rawSteering < _steeringNeutralMin || _rawSteering > _steeringNeutralMax);
}
