#include "DriveController.h"
#include <Arduino.h>

/*
 * =================================================================
 * HARDWARE SETUP for Cytron MDDS30 Motor Driver
 * =================================================================
 * This code is designed to work with the MDDS30 in "PWM Signed Magnitude" mode.
 * Set the DIP switches on the board as follows:
 * SW1: ON, SW2: OFF, SW3: ON, SW4: ON, SW5: OFF, SW6: ON
 * =================================================================
 */

DriveController::DriveController(int dirPin, int pwmPin, int pwmChannel, Logger* logger) {
  _dirPin = dirPin;
  _pwmPin = pwmPin;
  _pwmChannel = pwmChannel;
  _logger = logger;
}

void DriveController::setup() {
  pinMode(_dirPin, OUTPUT);

  // Configure the ESP32's LEDC hardware for PWM signal generation.
  ledcSetup(_pwmChannel, 5000, 8); // 5kHz frequency, 8-bit resolution (0-255).
  ledcAttachPin(_pwmPin, _pwmChannel);

  // Check if PWM setup was successful
  if (ledcRead(_pwmChannel) >= 0) {
    _initialized = true;
  }

  if (_logger) _logger->log("Initialized");
}

void DriveController::setSpeed(int speed) {
  // Validate that the controller is initialized before attempting to set speed
  if (!_initialized) {
    if (_logger) _logger->log("DriveController not initialized");
    return;
  }
  
  speed = constrain(speed, -255, 255);

  // Set the direction pin based on the sign of the speed.
  if (speed >= 0) {
    digitalWrite(_dirPin, HIGH); // FORWARD
  } else {
    digitalWrite(_dirPin, LOW);  // REVERSE
  }

  // The PWM duty cycle is the absolute value of the speed.
  int dutyCycle = abs(speed);

  // Send the command to the hardware PWM channel.
  ledcWrite(_pwmChannel, dutyCycle);

  if (_logger) _logger->log(speed);
}

bool DriveController::isInitialized() {
  return _initialized;
}
