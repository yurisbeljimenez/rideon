#include "SystemStatus.h"
#include "./Shared/CarState.h"

/**
 * @brief LED color definitions for different car states
 * Each color represents a specific driving state:
 * - Solid Blue: Stopped
 * - Solid Green: Moving Forward
 * - Solid Yellow: Moving in Reverse
 * - Pulsing Red: Avoiding Obstacle
 * - Pulsing White: Manual Override
 */
const uint32_t COLOR_BLUE = Adafruit_NeoPixel::Color(0, 0, 40);    // Stopped state
const uint32_t COLOR_GREEN = Adafruit_NeoPixel::Color(0, 40, 0);   // Forward motion
const uint32_t COLOR_YELLOW = Adafruit_NeoPixel::Color(40, 40, 0); // Reverse motion
const uint32_t COLOR_RED = Adafruit_NeoPixel::Color(40, 0, 0);     // Avoiding obstacle
const uint32_t COLOR_WHITE = Adafruit_NeoPixel::Color(40, 40, 40); // Manual override
const uint32_t COLOR_OFF = Adafruit_NeoPixel::Color(0, 0, 0);      // LED off

/**
 * @brief Constructor initializes the LED strip and stores pin/logger references
 * @param pin The GPIO pin connected to the NeoPixel LED
 * @param logger A pointer to a Logger object for debugging
 */
SystemStatus::SystemStatus(int pin, Logger* logger) : 
  _pixel(1, pin, NEO_GRB + NEO_KHZ800) 
{
  _pin = pin;
  _logger = logger;
}

/**
 * @brief Initialize the NeoPixel LED strip and log initialization
 * 
 * This function initializes the NeoPixel strip with proper configuration
 * and shows the LED off state initially.
 */
void SystemStatus::setup() {
  _pixel.begin();
  _pixel.show();
  if (_logger) _logger->log("SystemStatus initialized");
}

/**
 * @brief Update LED display based on current car state
 * 
 * This function interprets the CarState and displays the appropriate
 * LED pattern:
 * - Solid color for normal driving states (STOPPED, FORWARD, REVERSE)
 * - Pulsing color for warning states (AVOIDING_OBSTACLE, MANUAL_OVERRIDE)
 * 
 * @param state The current CarState to reflect visually
 */
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

/**
 * @brief Set LED to a solid color (no pulsing)
 * 
 * This function sets the LED to a constant color. It only updates
 * the LED if the requested color is different from the current color,
 * which prevents unnecessary operations and reduces flickering.
 * 
 * @param color The RGB color value to set
 */
void SystemStatus::setSolidColor(uint32_t color) {
  if (_pixel.getPixelColor(0) != color) {
    _pixel.setPixelColor(0, color);
    _pixel.show();
  }
}

/**
 * @brief Set LED to pulse between color and off state
 * 
 * This function creates a pulsing effect by alternating between the
 * specified color and off state at regular intervals. This is used
 * for warning states to draw attention.
 * 
 * @param color The RGB color value for pulsing effect
 */
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
