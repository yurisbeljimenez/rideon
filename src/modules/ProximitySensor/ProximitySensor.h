#pragma once
#include "../Logger/Logger.h"

/**
 * @class ProximitySensor
 * @brief Manages an HC-SR04 ultrasonic distance sensor in a fully non-blocking way.
 *
 * This advanced version uses hardware interrupts to measure the echo pulse
 * without ever pausing the main program loop, ensuring maximum responsiveness.
 */
class ProximitySensor {
public:
  /**
   * @brief Constructor for the ProximitySensor.
   * @param trigPin The GPIO pin connected to the sensor's Trig pin.
   * @param echoPin The GPIO pin connected to the sensor's Echo pin.
   * @param logger A pointer to a Logger object for debugging.
   */
  ProximitySensor(int trigPin, int echoPin, Logger* logger);

  void setup();
  void update(); // This function now triggers a new ping periodically.
  long getDistanceCm(); // This function now instantly returns the last known distance.

private:
  // Pins and Logger
  int _trigPin;
  int _echoPin;
  Logger* _logger;

  // Timing variables for non-blocking ping trigger
  unsigned long _lastPingTime = 0;
  const int _pingInterval = 60; // Ping every 60ms to avoid echo interference

  // --- Interrupt Service Routine (ISR) Variables ---
  // 'volatile' is crucial here. It tells the compiler that these variables can
  // be changed by an external process (the interrupt) at any time.
  static volatile unsigned long _echoStartTime;
  static volatile unsigned long _echoEndTime;
  static volatile bool _newDistanceAvailable;
  volatile long _distanceCm = 0;

  // The ISR must be a static function. We use a pointer to the specific
  // instance to access its members.
  static ProximitySensor* _instance;
  static void IRAM_ATTR echo_isr(); // IRAM_ATTR places the ISR in fast RAM
};
