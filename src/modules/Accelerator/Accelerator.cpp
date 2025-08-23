#include "Accelerator.h"
#include <Arduino.h>

// The constructor now accepts the logging threshold.
Accelerator::Accelerator(int pedalPin, Logger* logger, long accelIntervalLow, long accelIntervalMid, long accelIntervalHigh, long brakingInterval, int loggingThreshold) : 
  _accelIntervalLow(accelIntervalLow), 
  _accelIntervalMid(accelIntervalMid), 
  _accelIntervalHigh(accelIntervalHigh),
  _brakingInterval(brakingInterval),
  _loggingThreshold(loggingThreshold)
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

// The update() method now contains the intelligent logging logic.
void Accelerator::update() {
  int pedalValue = analogRead(_pedalPin);
  _desiredSpeed = map(pedalValue, 0, 4095, 0, 100);

  long currentInterval;
  if (_currentSpeed < _desiredSpeed) {
    if (_currentSpeed < _speedThresholdLow) {
      currentInterval = _accelIntervalLow;
    } else if (_currentSpeed < _speedThresholdHigh) {
      currentInterval = _accelIntervalMid;
    } else {
      currentInterval = _accelIntervalHigh;
    }
  } else {
    currentInterval = _brakingInterval;
  }

  unsigned long currentMillis = millis();
  if (currentMillis - _previousAccelMillis >= currentInterval) {
    _previousAccelMillis = currentMillis;
    if (_currentSpeed < _desiredSpeed) {
      _currentSpeed++;
    } else if (_currentSpeed > _desiredSpeed) {
      _currentSpeed--;
    }
  }

  // --- Intelligent Logging ---
  if (_logger) {
    int motorOutput = getMotorOutput();
    bool shouldLog = (motorOutput > _loggingThreshold);
    // Log if we are in the active zone OR if we just left it (to log the final 0).
    if (shouldLog || _wasLogging) {
      _logger->log(motorOutput);
    }
    _wasLogging = shouldLog;
  }
}

int Accelerator::getEngineLoad() {
  int load = _desiredSpeed - _currentSpeed;
  return (load > 0) ? load : 0;
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
