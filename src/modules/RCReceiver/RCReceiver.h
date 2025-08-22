#pragma once
#include "../Logger/Logger.h"

/**
 * @class RCReceiver
 * @brief Reads and interprets signals from a 2-channel RC receiver with a configurable dead zone.
 */
class RCReceiver {
public:
  /**
   * @brief Constructor for the RCReceiver.
   * @param throttlePin The GPIO pin for the throttle channel.
   * @param steeringPin The GPIO pin for the steering channel.
   * @param logger A pointer to a Logger object for debugging.
   * @param neutralMin The minimum pulse width (µs) to be considered neutral.
   * @param neutralMax The maximum pulse width (µs) to be considered neutral.
   */
  RCReceiver(int throttlePin, int steeringPin, Logger* logger, int neutralMin, int neutralMax);

  void setup();
  void update();
  int getThrottle();
  int getSteering();
  bool isOverriding();

private:
  int _throttlePin;
  int _steeringPin;
  Logger* _logger;

  volatile long _rawThrottle = 1500;
  volatile long _rawSteering = 1500;

  // These are now set by the constructor for easy tuning
  const int _neutralMin;
  const int _neutralMax;
};
