#pragma once
#include "../Logger/Logger.h"

/**
 * @class Accelerator
 * @brief Manages single-pedal driving with intelligent logging.
 *
 * This class reads an analog pedal input, applies a sophisticated smoothing
 * algorithm, and provides data on speed. It only logs its
 * output when the pedal is actively being used (outside of a dead zone).
 *
 * The accelerator implements a 3-stage acceleration curve:
 * 1. Gentle Start (0-25% speed) - Slow response for smooth starts
 * 2. Responsive Mid-Range (26-80% speed) - Fast response for maneuvering
 * 3. Ease to Top Speed (81-100% speed) - Gentle roll-off to prevent sudden acceleration
 */
class Accelerator {
public:
  /**
   * @brief Constructor for the Accelerator.
   * @param pedalPin The GPIO pin the analog pedal is connected to.
   * @param logger A pointer to a Logger object for debugging.
   * @param accelIntervalLow The slow interval for the "Gentle Start" zone (ms).
   * @param accelIntervalMid The fast interval for the "Responsive Mid-Range" zone (ms).
   * @param accelIntervalHigh The slow interval for the "Ease to Top Speed" zone (ms).
   * @param brakingInterval The very fast interval for active braking when the pedal is released (ms).
   * @param loggingThreshold The speed value above which logging will be active.
   */
  Accelerator(int pedalPin, Logger* logger, long accelIntervalLow, long accelIntervalMid, long accelIntervalHigh, long brakingInterval, int loggingThreshold);

  /**
   * @brief Initialize the accelerator and prepare for operation.
   */
  void setup();
  
  /**
   * @brief Update the accelerator state based on pedal input.
   * 
   * This function reads the analog pedal value, applies smoothing and
   * acceleration curve mapping, and calculates the appropriate motor output.
   */
  void update();
  
  /**
   * @brief Get the current motor output value.
   * @return The calculated motor output (0-255 range).
   */
  int getMotorOutput();
  
  /**
   * @brief Override the motor speed directly.
   * @param speed The speed value to override (0-255 range).
   */
  void overrideSpeed(int speed);
  
  /**
   * @brief Get the current calculated speed.
   * @return The current speed value (0-100 range).
   */
  int getCurrentSpeed();
  // The getEngineLoad() function has been removed as it is no longer needed.

private:
  // Pin and Logger
  int _pedalPin;                    // GPIO pin connected to the analog pedal
  Logger* _logger;                  // Logger for debugging output

  // Speed variables
  int _desiredSpeed;                // Desired speed based on pedal input
  int _currentSpeed;                // Smoothed current speed value
  
  // Timing variables
  unsigned long _previousAccelMillis; // Timestamp for acceleration timing

  // Constants for the acceleration curve and braking, set by the constructor
  const long _accelIntervalLow;     // Interval for gentle start (ms)
  const long _accelIntervalMid;     // Interval for responsive mid-range (ms)
  const long _accelIntervalHigh;    // Interval for ease to top speed (ms)
  const long _brakingInterval;      // Interval for active braking (ms)

  // Defines the speed ranges for the different acceleration zones
  int _speedThresholdLow;           // Speed threshold for low interval (25%)
  int _speedThresholdHigh;          // Speed threshold for mid interval (80%)

  // Intelligent logging variables
  const int _loggingThreshold;      // Speed threshold for logging activity
  bool _wasLogging = false;         // Tracks if we were previously logging
};
