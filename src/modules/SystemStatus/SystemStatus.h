#pragma once
#include <Adafruit_NeoPixel.h>
#include "../Logger/Logger.h"
#include "./Shared/CarState.h" // This include is correct

class SystemStatus {
public:
  SystemStatus(int pin, Logger* logger);
  void setup();
  void update(CarState state);

private:
  int _pin;
  Logger* _logger;
  Adafruit_NeoPixel _pixel;

  void setSolidColor(uint32_t color);
  void setPulsingColor(uint32_t color);

  unsigned long _previousMillis = 0;
  bool _pulseState = true;
  const int _pulseInterval = 250;
};