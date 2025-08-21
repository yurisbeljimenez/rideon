#include "ProximitySensor.h"
#include <Arduino.h>

// Initialize static members of the class
volatile unsigned long ProximitySensor::_echoStartTime = 0;
volatile unsigned long ProximitySensor::_echoEndTime = 0;
volatile bool ProximitySensor::_newDistanceAvailable = false;
ProximitySensor* ProximitySensor::_instance = nullptr;

ProximitySensor::ProximitySensor(int trigPin, int echoPin, Logger* logger) {
  _trigPin = trigPin;
  _echoPin = echoPin;
  _logger = logger;
  _instance = this; // Store a pointer to this instance for the static ISR
}

void ProximitySensor::setup() {
  pinMode(_trigPin, OUTPUT);
  pinMode(_echoPin, INPUT);

  // Attach the hardware interrupt to the echo pin.
  // The 'echo_isr' function will be called automatically on any CHANGE (rising or falling edge).
  attachInterrupt(digitalPinToInterrupt(_echoPin), echo_isr, CHANGE);

  if (_logger) {
    _logger->log("Initialized (Non-Blocking)");
  }
}

// The new update function is called from the main loop. Its job is to trigger pings.
void ProximitySensor::update() {
  unsigned long currentTime = millis();
  // Periodically trigger a new sensor ping.
  if (currentTime - _lastPingTime >= _pingInterval) {
    _lastPingTime = currentTime;

    // Send the 10-microsecond ultrasonic ping.
    digitalWrite(_trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(_trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(_trigPin, LOW);
    _newDistanceAvailable = false;
  }

  // If the ISR has calculated a new distance, update our main variable.
  if (_newDistanceAvailable) {
    // Calculate distance from the times captured by the ISR.
    // We do the calculation here to keep the ISR as fast as possible.
    long duration = _echoEndTime - _echoStartTime;
    _distanceCm = duration * 0.0343 / 2;
    _newDistanceAvailable = false; // Reset the flag
    if (_logger) {
      _logger->log(_distanceCm);
    }
  }
}

// This function now instantly returns the last calculated distance. It does not wait.
long ProximitySensor::getDistanceCm() {
  return _distanceCm;
}

// --- Interrupt Service Routine (ISR) ---
// This special function is called by the hardware itself, not by our code.
// It must be as fast as possible. No delays, no serial prints.
void IRAM_ATTR ProximitySensor::echo_isr() {
  if (_instance) { // Check if the instance is valid
    // Check if the pin just went HIGH (start of echo)
    if (digitalRead(_instance->_echoPin) == HIGH) {
      _echoStartTime = micros(); // Record start time in microseconds
    } 
    // Otherwise, the pin just went LOW (end of echo)
    else {
      _echoEndTime = micros(); // Record end time
      _newDistanceAvailable = true; // Set a flag for the main loop to process
    }
  }
}
