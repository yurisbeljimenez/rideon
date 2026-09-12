#include "Accelerator.h"
#include <Arduino.h>

Accelerator::Accelerator(int pedalPin, Logger* logger, int adcMin, int adcMax) :
  _pedalPin(pedalPin),
  _logger(logger),
  _adcMin(adcMin),
  _adcMax(adcMax)
{
  for (int i = 0; i < FILTER_WINDOW; i++) {
    _filterBuffer[i] = 0;
  }
}

void Accelerator::setup() {
  // Initialize the filter buffer to neutral
  for (int i = 0; i < FILTER_WINDOW; i++) {
    _filterBuffer[i] = _adcMin;
  }
  _filterSum = (long)_adcMin * FILTER_WINDOW;
  _pedalIntent = 0;

  if (_logger) {
    _logger->log("Initialized");
  }
}

void Accelerator::update() {
  // Read the raw analog value
  int rawPedalValue = analogRead(_pedalPin);

  // Apply moving average filter to reduce noise (motor EMI, pot contact noise)
  _filterSum = _filterSum - _filterBuffer[_filterIndex];
  _filterBuffer[_filterIndex] = rawPedalValue;
  _filterSum = _filterSum + rawPedalValue;
  _filterIndex = (_filterIndex + 1) % FILTER_WINDOW;

  int filteredPedalValue = (int)(_filterSum / FILTER_WINDOW);

  // Map the calibrated ADC range to 0..100
  _pedalIntent = (int)map(filteredPedalValue, _adcMin, _adcMax, 0, 100);
  if (_pedalIntent < 0) _pedalIntent = 0;
  if (_pedalIntent > 100) _pedalIntent = 100;

  // --- Intelligent Logging ---
  if (_logger) {
    bool shouldLog = (_pedalIntent > LOGGING_THRESHOLD);
    if (shouldLog || _wasLogging) {
      _logger->log(_pedalIntent);
    }
    _wasLogging = shouldLog;
  }
}

int Accelerator::getPedalIntent() {
  return _pedalIntent;
}
