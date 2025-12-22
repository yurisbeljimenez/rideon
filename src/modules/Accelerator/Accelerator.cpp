#include "Accelerator.h"
#include <Arduino.h>

// The constructor now accepts the logging threshold.
Accelerator::Accelerator(int pedalPin, Logger* logger, long accelIntervalLow, long accelIntervalMid, long accelIntervalHigh, long brakingInterval, int loggingThreshold) : 
  _accelIntervalLow(accelIntervalLow), 
  _accelIntervalMid(accelIntervalMid), 
  _accelIntervalHigh(accelIntervalHigh),
  _brakingInterval(brakingInterval),
  _loggingThreshold(loggingThreshold),
  _speedThresholdLow(25),
  _speedThresholdHigh(80)
{
  _pedalPin = pedalPin;
  _logger = logger;
  
  // Initialize the analog filter buffer
  _filterBuffer = new int[_filterWindowSize];
  for (int i = 0; i < _filterWindowSize; i++) {
    _filterBuffer[i] = 0;
  }
}

// setup() is unchanged
void Accelerator::setup() {
  _currentSpeed = 0;
  _desiredSpeed = 0;
  
  // Initialize the filter buffer
  for (int i = 0; i < _filterWindowSize; i++) {
    _filterBuffer[i] = 0;
  }
  
  if (_logger) {
    _logger->log("Initialized");
  }
}

// update() with analog filtering
void Accelerator::update() {
  // Read the raw analog value
  int rawPedalValue = analogRead(_pedalPin);
  
  // Apply moving average filter to reduce noise
  _filterSum = _filterSum - _filterBuffer[_filterIndex];
  _filterBuffer[_filterIndex] = rawPedalValue;
  _filterSum = _filterSum + rawPedalValue;
  _filterIndex = (_filterIndex + 1) % _filterWindowSize;
  
  // Use the filtered value for speed calculation
  int filteredPedalValue = _filterSum / _filterWindowSize;
  
  _desiredSpeed = map(filteredPedalValue, 0, 4095, 0, 100);

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
    if (shouldLog || _wasLogging) {
      _logger->log(motorOutput);
    }
    _wasLogging = shouldLog;
  }
}

// The getEngineLoad() function has been removed as it is no longer needed.

void Accelerator::overrideSpeed(int speed) {
  _desiredSpeed = constrain(speed, 0, 100);
}

int Accelerator::getCurrentSpeed() {
  return _currentSpeed;
}

int Accelerator::getMotorOutput() {
  return map(_currentSpeed, 0, 100, 0, 255);
}
