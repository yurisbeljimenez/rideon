#pragma once
#include "../Logger/Logger.h"

/**
 * @class RCReceiver
 * @brief Reads and interprets signals from a 2-channel RC receiver with configurable dead zones.
 *
 * Uses pulseIn() to measure the pulse width for throttle and steering. It provides
 * mapped values (-100 to 100) and an override status. Logging is now intelligent,
 * only reporting values when either the throttle or steering is actively being used.
 */
class RCReceiver {
public:
  /**
   * @brief Constructor for the RCReceiver.
   * @param throttlePin The GPIO pin for the throttle channel.
   * @param steeringPin The GPIO pin for the steering channel.
   * @param logger A pointer to a Logger object for debugging.
   * @param throttleNeutralMin The minimum pulse width (µs) for throttle to be considered neutral.
   * @param throttleNeutralMax The maximum pulse width (µs) for throttle to be considered neutral.
   * @param steeringNeutralMin The minimum pulse width (µs) for steering to be considered neutral.
   * @param steeringNeutralMax The maximum pulse width (µs) for steering to be considered neutral.
   */
  RCReceiver(int throttlePin, int steeringPin, Logger* logger, int throttleNeutralMin, int throttleNeutralMax, int steeringNeutralMin, int steeringNeutralMax);

  void setup();
  void update();
  int getThrottle();
  int getSteering();
  bool isOverriding(); // Checks if throttle is active
  bool isSteeringActive(); // Checks if steering is active

private:
  int _throttlePin;
  int _steeringPin;
  Logger* _logger;

  volatile long _rawThrottle = 1500;
  volatile long _rawSteering = 1500;

  // Dead zone values are now separate for each channel
  const int _throttleNeutralMin;
  const int _throttleNeutralMax;
  const int _steeringNeutralMin;
  const int _steeringNeutralMax;
};
