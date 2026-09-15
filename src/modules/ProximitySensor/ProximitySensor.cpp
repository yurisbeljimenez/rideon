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

// The update function now handles both pinging, smoothing, and auto-recovery.
void ProximitySensor::update() {
  unsigned long currentTime = millis();

  // 1. Periodically trigger a new sensor ping.
  if (currentTime - _lastPingTime >= _pingInterval) {
    _lastPingTime = currentTime;
    _pendingTimeout = false; // Reset timeout counter for this ping cycle
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
    // Sanity window: ~0.9 cm .. ~430 cm. Rejects electrical noise glitches.
    if (duration >= 50 && duration <= 25000) {
      rawDistance = duration * 0.0343 / 2;
    }
    _newDistanceAvailable = false; // Reset the flag
    _lastRawCm = rawDistance;

    // --- Moving Average Filter Logic ---
    _total = _total - _readings[_readIndex];
    _readings[_readIndex] = rawDistance;
    _total = _total + _readings[_readIndex];
    _readIndex = (_readIndex + 1) % _windowSize; // Wrap index
    _smoothedDistanceCm = _total / _windowSize;

    // --- Auto-recovery: valid echo resets miss count ---
    _missCount = 0;

    if (_wasInvalid) {
      // We were in the invalid state; count consecutive good readings to recover.
      _goodCount++;
      if (_goodCount >= RECOVER_THRESHOLD) {
        _valid = true;
        _wasInvalid = false;
        _goodCount = 0;
        if (_logger) _logger->log("Sensor Recovered");
      }
    } else {
      _valid = true;
      _goodCount = 0;
    }

    // --- Intelligent Logging Logic ---
    if (_logger && _smoothedDistanceCm > 0 && _smoothedDistanceCm < _loggingThreshold) {
      _logger->log(_smoothedDistanceCm);
    }

    // Update last echo time for timeout detection
    _lastEchoTime = currentTime;
  }
  else {
    // 3. No new echo — check for timeout (counted once per ping cycle).
    if (!_pendingTimeout && (currentTime - _lastEchoTime) > _sensorTimeout) {
      _pendingTimeout = true; // Only count this timeout once
      _missCount++;

      if (_valid && _missCount >= MISS_THRESHOLD) {
        // Fail CLOSED after sustained signal loss.
        _valid = false;
        _wasInvalid = true;
        _goodCount = 0;
        _smoothedDistanceCm = -1;
        if (_logger) _logger->log("Sensor Timeout");
      }
    }
  }
}

// This function now instantly returns the last known SMOOTHED distance.
long ProximitySensor::getDistanceCm() {
  return _smoothedDistanceCm;
}

/**
 * @brief Get the raw sensor reading (for diagnostic purposes).
 * @return Raw distance reading or -1 if error.
 */
long ProximitySensor::getRawDistanceCm() {
  return _lastRawCm; // -1 until the first successful echo
}

bool ProximitySensor::isValid() {
  return _valid;
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
    
    // Update last echo time when we get an echo
    _lastEchoTime = millis();
  }
}

// Destructor to clean up dynamically allocated memory
ProximitySensor::~ProximitySensor() {
    delete[] _readings;
}
