#include "RCReceiver.h"
#include <Arduino.h>

// Acceptable pulse width window (µs). Standard RC servo pulses are 1000-2000 µs;
// this window is generous but still rejects electrical glitch edges.
static const long MIN_PULSE_US = 500;
static const long MAX_PULSE_US = 4000;

RCReceiver::RCReceiver(int throttlePin, int steeringPin, Logger* logger,
                       int throttleNeutralMin, int throttleNeutralMax,
                       int steeringNeutralMin, int steeringNeutralMax,
                       unsigned long signalTimeoutMs)
  : _throttleNeutralMin(throttleNeutralMin),
    _throttleNeutralMax(throttleNeutralMax),
    _steeringNeutralMin(steeringNeutralMin),
    _steeringNeutralMax(steeringNeutralMax),
    _signalTimeoutMs(signalTimeoutMs)
{
  _throttlePin = throttlePin;
  _steeringPin = steeringPin;
  _logger = logger;
}

void RCReceiver::setup() {
  pinMode(_throttlePin, INPUT);
  pinMode(_steeringPin, INPUT);

  // Non-blocking pulse measurement: one interrupt per pin, edge timestamped
  // with micros(). Same pattern as ProximitySensor.
  attachInterruptArg(digitalPinToInterrupt(_throttlePin), isr_handler, this, CHANGE);
  attachInterruptArg(digitalPinToInterrupt(_steeringPin), isr_handler, this, CHANGE);

  if (_logger) _logger->log("Initialized (non-blocking)");
}

void RCReceiver::update() {
  unsigned long now = millis();
  _throttleValid = (now - _lastThrottleMs) <= _signalTimeoutMs;
  _steeringValid = (now - _lastSteeringMs) <= _signalTimeoutMs;

  bool signalValid = _throttleValid && _steeringValid;

  // Log link state transitions (Logger dedupes identical values).
  if (_logger) {
    if (signalValid != _wasSignalValid) {
      _logger->log(signalValid ? "Signal OK" : "SIGNAL LOST");
    }
    if (signalValid && (isOverriding() || isSteeringActive())) {
      _logger->log((long)_rawThrottle * 10000 + (long)_rawSteering);
    }
  }
  _wasSignalValid = signalValid;
}

int RCReceiver::getThrottle() {
  if (!_throttleValid) return 0;  // no signal: safe neutral
  return (int)map(_rawThrottle, 1000, 2000, -100, 100);
}

int RCReceiver::getSteering() {
  if (!_steeringValid) return 0;  // no signal: safe neutral
  return (int)map(_rawSteering, 1000, 2000, -100, 100);
}

// Checks if the throttle is actively being used (and the link is alive).
bool RCReceiver::isOverriding() {
  if (!_throttleValid) return false;
  return (_rawThrottle < _throttleNeutralMin || _rawThrottle > _throttleNeutralMax);
}

// Checks if the steering is actively being used (and the link is alive).
bool RCReceiver::isSteeringActive() {
  if (!_steeringValid) return false;
  return (_rawSteering < _steeringNeutralMin || _rawSteering > _steeringNeutralMax);
}

bool RCReceiver::isSignalValid() {
  return _throttleValid && _steeringValid;
}

// --- Interrupt service -------------------------------------------------------

// Static C-style bridge: hardware interrupt -> class instance.
void IRAM_ATTR RCReceiver::isr_handler(void* arg) {
  RCReceiver* instance = static_cast<RCReceiver*>(arg);
  instance->handleISR();
}

// Timestamps servo PWM edges for both channels.
void IRAM_ATTR RCReceiver::handleISR() {
  // Throttle channel
  if (digitalRead(_throttlePin) == HIGH) {
    _throttleStartUs = micros();
    _throttlePulsing = true;
  } else if (_throttlePulsing) {
    long width = (long)micros() - (long)_throttleStartUs;
    if (width >= MIN_PULSE_US && width <= MAX_PULSE_US) {
      _rawThrottle = width;
      _lastThrottleMs = millis();
    }
    _throttlePulsing = false;
  }

  // Steering channel
  if (digitalRead(_steeringPin) == HIGH) {
    _steeringStartUs = micros();
    _steeringPulsing = true;
  } else if (_steeringPulsing) {
    long width = (long)micros() - (long)_steeringStartUs;
    if (width >= MIN_PULSE_US && width <= MAX_PULSE_US) {
      _rawSteering = width;
      _lastSteeringMs = millis();
    }
    _steeringPulsing = false;
  }
}
