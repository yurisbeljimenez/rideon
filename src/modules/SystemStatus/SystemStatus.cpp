#include "SystemStatus.h"
// The incorrect include for "main.cpp" has been removed.

// Color definitions
const uint32_t COLOR_BLUE = Adafruit_NeoPixel::Color(0, 0, 40);
const uint32_t COLOR_GREEN = Adafruit_NeoPixel::Color(0, 40, 0);
const uint32_t COLOR_YELLOW = Adafruit_NeoPixel::Color(40, 40, 0);
const uint32_t COLOR_RED = Adafruit_NeoPixel::Color(40, 0, 0);
const uint32_t COLOR_WHITE = Adafruit_NeoPixel::Color(40, 40, 40);
const uint32_t COLOR_OFF = Adafruit_NeoPixel::Color(0, 0, 0);

SystemStatus::SystemStatus(int pin, Logger* logger) : 
  _pixel(1, pin, NEO_GRB + NEO_KHZ800) 
{
  _pin = pin;
  _logger = logger;
}

void SystemStatus::setup() {
  _pixel.begin();
  _pixel.show();
  if (_logger) _logger->log("Initialized");
}

void SystemStatus::update(CarState state) {
  switch(state) {
    case CarState::STOPPED:
      setSolidColor(COLOR_BLUE);
      break;
    case CarState::FORWARD:
      setSolidColor(COLOR_GREEN);
      break;
    case CarState::REVERSE:
      setSolidColor(COLOR_YELLOW);
      break;
    case CarState::AVOIDING_OBSTACLE:
      setPulsingColor(COLOR_RED);
      break;
    case CarState::MANUAL_OVERRIDE:
      setPulsingColor(COLOR_WHITE);
      break;
  }
}

void SystemStatus::setSolidColor(uint32_t color) {
  if (_pixel.getPixelColor(0) != color) {
    _pixel.setPixelColor(0, color);
    _pixel.show();
  }
}

void SystemStatus::setPulsingColor(uint32_t color) {
  unsigned long currentMillis = millis();
  if (currentMillis - _previousMillis >= _pulseInterval) {
    _previousMillis = currentMillis;

    if (_pulseState) {
      _pixel.setPixelColor(0, color);
    } else {
      _pixel.setPixelColor(0, COLOR_OFF);
    }
    _pixel.show();
    _pulseState = !_pulseState;
  }
}