#pragma once
#include "../Logger/Logger.h"

/**
 * @class Accelerator
 * @brief Manages single-pedal driving with a multi-stage acceleration curve and active braking.
 *
 * This class reads an analog pedal input and applies a sophisticated smoothing
 * algorithm. It uses a 3-stage curve for smooth acceleration and a separate,
 * much faster rate for deceleration, creating an intuitive single-pedal driving feel.
 */
class Accelerator {
public:
  /**
   * @brief Constructor for the Accelerator.
   * @param pedalPin The GPIO pin the analog pedal is connected to.
   * @param logger A pointer to a Logger object for debugging.
   * @param accelIntervalLow The slow interval for the "Gentle Start" zone.
   * @param accelIntervalMid The fast interval for the "Responsive Mid-Range" zone.
   * @param accelIntervalHigh The slow interval for the "Ease to Top Speed" zone.
   * @param brakingInterval The very fast interval for active braking when the pedal is released.
   */
  Accelerator(int pedalPin, Logger* logger, long accelIntervalLow, long accelIntervalMid, long accelIntervalHigh, long brakingInterval);

  void setup();
  void update();
  int getMotorOutput();
  void overrideSpeed(int speed);
  int getCurrentSpeed();

private:
  // Pin and Logger
  int _pedalPin;
  Logger* _logger;

  // Speed variables
  int _desiredSpeed;
  int _currentSpeed;
  
  // Timing variables
  unsigned long _previousAccelMillis; // CORRECTED: Changed from _previousUpdateMillis

  // Constants for the acceleration curve and braking, set by the constructor
  const long _accelIntervalLow;
  const long _accelIntervalMid;
  const long _accelIntervalHigh;
  const long _brakingInterval;

  // Defines the speed ranges for the different acceleration zones
  const int _speedThresholdLow = 25;
  const int _speedThresholdHigh = 80;
};
