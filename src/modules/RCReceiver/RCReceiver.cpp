#include "RCReceiver.h"
#include <Arduino.h>

// The constructor now accepts and saves the dead zone calibration values.
RCReceiver::RCReceiver(int throttlePin, int steeringPin, Logger* logger, int neutralMin, int neutralMax) :
  _neutralMin(neutralMin),
  _neutralMax(neutralMax)
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

  if (_logger) {
    // This log is now our primary calibration tool.
    _logger->log(_rawThrottle * 10000 + _rawSteering);
  }
}

int RCReceiver::getThrottle() {
  return map(_rawThrottle, 1000, 2000, -100, 100);
}

int RCReceiver::getSteering() {
  return map(_rawSteering, 1000, 2000, -100, 100);
}

bool RCReceiver::isOverriding() {
  // The override check now uses our tunable dead zone values.
  return (_rawThrottle < _neutralMin || _rawThrottle > _neutralMax);
}
