#include "ProximitySensor.h"
#include <Arduino.h>

// The constructor now also creates the dynamic array for the smoothing filter.
ProximitySensor::ProximitySensor(int trigPin, int echoPin, Logger* logger, int loggingThreshold, int windowSize) :
  _loggingThreshold(loggingThreshold),
  _windowSize(windowSize)
{
  _trigPin = trigPin;
  _echoPin = echoPin;
  _logger = logger;
  _readings = new long[_windowSize];
}

void ProximitySensor::setup() {
  pinMode(_trigPin, OUTPUT);
  pinMode(_echoPin, INPUT);

  // Initialize the smoothing filter array.
  for (int i = 0; i < _windowSize; i++) {
    _readings[i] = 0;
  }

  // --- Correct Multi-Instance Interrupt Setup ---
  // We attach the interrupt and pass 'this' (a pointer to the current object)
  // as an argument. This allows our static handler to know which sensor instance
  // triggered the interrupt, fixing the bug.
  attachInterruptArg(digitalPinToInterrupt(_echoPin), isr_handler, this, CHANGE);

  if (_logger) {
    _logger->log("Initialized (Unified)");
  }
}

// The update function now handles both pinging and smoothing.
void ProximitySensor::update() {
  // 1. Periodically trigger a new sensor ping.
  unsigned long currentTime = millis();
  if (currentTime - _lastPingTime >= _pingInterval) {
    _lastPingTime = currentTime;
    digitalWrite(_trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(_trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(_trigPin, LOW);
  }

  // 2. If the ISR has captured a new raw reading, process it.
  if (_newDistanceAvailable) {
    long duration = _echoEndTime - _echoStartTime;
    long rawDistance = 0;
    if (duration > 0) {
      rawDistance = duration * 0.0343 / 2;
    }
    _newDistanceAvailable = false; // Reset the flag

    // --- Moving Average Filter Logic ---
    _total = _total - _readings[_readIndex];
    _readings[_readIndex] = rawDistance;
    _total = _total + _readings[_readIndex];
    _readIndex = (_readIndex + 1) % _windowSize; // Wrap index
    _smoothedDistanceCm = _total / _windowSize;

    // --- Intelligent Logging Logic ---
    if (_logger && _smoothedDistanceCm > 0 && _smoothedDistanceCm < _loggingThreshold) {
      _logger->log(_smoothedDistanceCm);
    }
  }
}

// This function now instantly returns the last known SMOOTHED distance.
long ProximitySensor::getDistanceCm() {
  return _smoothedDistanceCm;
}

// This is the static C-style function that the hardware interrupt calls.
// It acts as a bridge to our class-specific method.
void IRAM_ATTR ProximitySensor::isr_handler(void* arg) {
  // Cast the argument back to a ProximitySensor object pointer and call its handler.
  ProximitySensor* instance = static_cast<ProximitySensor*>(arg);
  instance->handleInterrupt();
}

// This is the instance-specific interrupt handler.
void IRAM_ATTR ProximitySensor::handleInterrupt() {
  if (digitalRead(_echoPin) == HIGH) {
    _echoStartTime = micros();
  } else {
    _echoEndTime = micros();
    _newDistanceAvailable = true;
  }
}
