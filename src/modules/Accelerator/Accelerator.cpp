#include "Accelerator.h"
#include <Arduino.h>

// The constructor now accepts and saves the four interval tuning values for the driving feel.
Accelerator::Accelerator(int pedalPin, Logger* logger, long accelIntervalLow, long accelIntervalMid, long accelIntervalHigh, long brakingInterval) : 
  _accelIntervalLow(accelIntervalLow), 
  _accelIntervalMid(accelIntervalMid), 
  _accelIntervalHigh(accelIntervalHigh),
  _brakingInterval(brakingInterval)
{
  _pedalPin = pedalPin;
  _logger = logger;
}

// setup() is unchanged
void Accelerator::setup() {
  _currentSpeed = 0;
  _desiredSpeed = 0;
  if (_logger) {
    _logger->log("Initialized");
  }
}

// The update() method contains the logic for both the acceleration curve and active braking.
void Accelerator::update() {
  int pedalValue = analogRead(_pedalPin);
  _desiredSpeed = map(pedalValue, 0, 4095, 0, 100);

  // --- Select the correct interval for this update cycle ---
  long currentInterval;
  if (_currentSpeed < _desiredSpeed) {
    // ACCELERATING: Use the 3-stage acceleration curve for a smooth start.
    if (_currentSpeed < _speedThresholdLow) {
      currentInterval = _accelIntervalLow;
    } else if (_currentSpeed < _speedThresholdHigh) {
      currentInterval = _accelIntervalMid;
    } else {
      currentInterval = _accelIntervalHigh;
    }
  } else {
    // DECELERATING (BRAKING): Use the single, fast braking interval for a responsive stop.
    currentInterval = _brakingInterval;
  }

  // The core smoothing logic now uses the dynamically selected interval.
  unsigned long currentMillis = millis();
  // CORRECTED: Changed _previousUpdateMillis to _previousAccelMillis to match the header file.
  if (currentMillis - _previousAccelMillis >= currentInterval) {
    _previousAccelMillis = currentMillis;
    if (_currentSpeed < _desiredSpeed) {
      _currentSpeed++;
    } else if (_currentSpeed > _desiredSpeed) {
      _currentSpeed--;
    }
  }

  if (_logger) {
    _logger->log(getMotorOutput());
  }
}

void Accelerator::overrideSpeed(int speed) {
  _desiredSpeed = constrain(speed, 0, 100);
}

int Accelerator::getCurrentSpeed() {
  return _currentSpeed;
}

int Accelerator::getMotorOutput() {
  return map(_currentSpeed, 0, 100, 0, 255);
}
