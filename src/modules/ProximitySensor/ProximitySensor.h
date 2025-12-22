#pragma once
#include "../Logger/Logger.h"

/**
 * @class ProximitySensor
 * @brief A unified, non-blocking, and smoothed ultrasonic sensor module.
 *
 * This class manages an HC-SR04 sensor from start to finish. It uses hardware
 * interrupts for fully non-blocking distance reads, correctly handles multiple
 * sensor instances, applies an internal moving average filter to smooth out noisy
 * readings, and only logs data when an object enters a specified "warning zone".
 * 
 * This version includes timeout protection to handle sensor communication issues.
 */
class ProximitySensor {
public:
  /**
   * @brief Constructor for the ProximitySensor.
   * @param trigPin The GPIO pin for the sensor's Trig pin.
   * @param echoPin The GPIO pin for the sensor's Echo pin.
   * @param logger A pointer to a Logger object for debugging.
   * @param loggingThreshold The distance in cm below which readings will be logged.
   * @param windowSize The number of readings to include in the moving average filter.
 */
  ProximitySensor(int trigPin, int echoPin, Logger* logger, int loggingThreshold, int windowSize);

  void setup();
  void update(); // Triggers pings and updates the smoothed average.
  long getDistanceCm(); // Instantly returns the last known smoothed distance.
  
  /**
   * @brief Get the raw sensor reading (for diagnostic purposes).
   * @return Raw distance reading or -1 if error.
   */
  long getRawDistanceCm();

private:
  // Pins, Logger, and Tuning
  int _trigPin;
  int _echoPin;
  Logger* _logger;
  const int _loggingThreshold;

  // Timing for non-blocking pings
  unsigned long _lastPingTime = 0;
  const int _pingInterval = 60;

  // Variables for the moving average filter
  const int _windowSize;
  long* _readings;
  int _readIndex = 0;
  long _total = 0;
  volatile long _smoothedDistanceCm = 0;

  // --- Interrupt Service Routine (ISR) Variables ---
  // These are specific to each instance of the class.
  volatile unsigned long _echoStartTime = 0;
  volatile unsigned long _echoEndTime = 0;
  volatile bool _newDistanceAvailable = false;
  
  // Timeout handling
  const unsigned long _sensorTimeout = 100; // Timeout in milliseconds for sensor responses
  unsigned long _lastEchoTime = 0;
  
  // The ISR function that will be called by the hardware.
  void IRAM_ATTR handleInterrupt();

  // A static helper that allows the C-style ISR to call our class method.
  static void IRAM_ATTR isr_handler(void* arg);
};
