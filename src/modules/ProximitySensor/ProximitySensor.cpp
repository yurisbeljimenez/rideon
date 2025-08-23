#include "ProximitySensor.h"
#include <Arduino.h>

// Initialize static members of the class
volatile unsigned long ProximitySensor::_echoStartTime = 0;
volatile unsigned long ProximitySensor::_echoEndTime = 0;
volatile bool ProximitySensor::_newDistanceAvailable = false;
ProximitySensor* ProximitySensor::_instance = nullptr;

// The constructor now accepts and saves the logging threshold.
ProximitySensor::ProximitySensor(int trigPin, int echoPin, Logger* logger, int loggingThreshold) :
  _loggingThreshold(loggingThreshold)
{
  _trigPin = trigPin;
  _echoPin = echoPin;
  _logger = logger;
  _instance = this;
}

void ProximitySensor::setup() {
  pinMode(_trigPin, OUTPUT);
  pinMode(_echoPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(_echoPin), echo_isr, CHANGE);
  if (_logger) {
    _logger->log("Initialized (Non-Blocking)");
  }
}

void ProximitySensor::update() {
  unsigned long currentTime = millis();
  if (currentTime - _lastPingTime >= _pingInterval) {
    _lastPingTime = currentTime;
    digitalWrite(_trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(_trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(_trigPin, LOW);
    _newDistanceAvailable = false;
  }

  if (_newDistanceAvailable) {
    long duration = _echoEndTime - _echoStartTime;
    _distanceCm = duration * 0.0343 / 2;
    _newDistanceAvailable = false;
    
    // --- Intelligent Logging Logic ---
    // Only log the distance if it's a valid reading AND it's within our warning zone.
    if (_logger && _distanceCm > 0 && _distanceCm < _loggingThreshold) {
      _logger->log(_distanceCm);
    }
  }
}

long ProximitySensor::getDistanceCm() {
  return _distanceCm;
}

void IRAM_ATTR ProximitySensor::echo_isr() {
  if (_instance) {
    if (digitalRead(_instance->_echoPin) == HIGH) {
      _echoStartTime = micros();
    } 
    else {
      _echoEndTime = micros();
      _newDistanceAvailable = true;
    }
  }
}
