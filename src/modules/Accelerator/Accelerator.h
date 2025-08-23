#pragma once
#include "../Logger/Logger.h"

/**
 * @class Accelerator
 * @brief Manages single-pedal driving with intelligent logging.
 *
 * This class reads an analog pedal input, applies a sophisticated smoothing
 * algorithm, and provides data on speed. It only logs its
 * output when the pedal is actively being used (outside of a dead zone).
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
   * @param loggingThreshold The speed value above which logging will be active.
   */
  Accelerator(int pedalPin, Logger* logger, long accelIntervalLow, long accelIntervalMid, long accelIntervalHigh, long brakingInterval, int loggingThreshold);

  void setup();
  void update();
  int getMotorOutput();
  void overrideSpeed(int speed);
  int getCurrentSpeed();
  // The getEngineLoad() function has been removed as it is no longer needed.

private:
  // Pin and Logger
  int _pedalPin;
  Logger* _logger;

  // Speed variables
  int _desiredSpeed;
  int _currentSpeed;
  
  // Timing variables
  unsigned long _previousAccelMillis;

  // Constants for the acceleration curve and braking, set by the constructor
  const long _accelIntervalLow;
  const long _accelIntervalMid;
  const long _accelIntervalHigh;
  const long _brakingInterval;

  // Defines the speed ranges for the different acceleration zones
  const int _speedThresholdLow = 25;
  const int _speedThresholdHigh = 80;

  // Intelligent logging variables
  const int _loggingThreshold;
  bool _wasLogging = false;
};
