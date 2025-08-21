#pragma once
#include "../Logger/Logger.h"

/**
 * @class RCReceiver
 * @brief Reads and interprets signals from a 2-channel RC receiver.
 *
 * Uses pulseIn() to measure the pulse width for throttle and steering,
 * and provides mapped values (-100 to 100) and an override status.
 */
class RCReceiver {
public:
  /**
   * @brief Constructor for the RCReceiver.
   * @param throttlePin The GPIO pin connected to the receiver's throttle channel.
   * @param steeringPin The GPIO pin connected to the receiver's steering channel.
   * @param logger A pointer to a Logger object for debugging.
   */
  RCReceiver(int throttlePin, int steeringPin, Logger* logger);

  void setup();
  void update(); // Reads the latest pulse widths from the receiver.
  int getThrottle(); // Returns throttle value mapped to -100 (rev) to 100 (fwd).
  int getSteering(); // Returns steering value mapped to -100 (left) to 100 (right).
  bool isOverriding(); // Returns true if the throttle is outside its neutral zone.

private:
  int _throttlePin;
  int _steeringPin;
  Logger* _logger;

  // Raw pulse width values in microseconds (µs).
  volatile long _rawThrottle = 1500;
  volatile long _rawSteering = 1500;

  // Defines the neutral "dead zone" for the throttle stick.
  const int _neutralMin = 1450;
  const int _neutralMax = 1550;
};