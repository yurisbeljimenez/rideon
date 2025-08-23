#pragma once
#include "../Logger/Logger.h"

/**
 * @class ProximitySensor
 * @brief Manages an HC-SR04 sensor with intelligent, non-blocking operation.
 *
 * This version uses hardware interrupts for non-blocking reads and only logs
 * distance readings when an object enters a specified "warning zone" threshold.
 */
class ProximitySensor {
public:
  /**
   * @brief Constructor for the ProximitySensor.
   * @param trigPin The GPIO pin for the sensor's Trig pin.
   * @param echoPin The GPIO pin for the sensor's Echo pin.
   * @param logger A pointer to a Logger object for debugging.
   * @param loggingThreshold The distance in cm below which readings will be logged.
   */
  ProximitySensor(int trigPin, int echoPin, Logger* logger, int loggingThreshold);

  void setup();
  void update();
  long getDistanceCm();

private:
  int _trigPin;
  int _echoPin;
  Logger* _logger;
  const int _loggingThreshold; 

  unsigned long _lastPingTime = 0;
  const int _pingInterval = 60;

  static volatile unsigned long _echoStartTime;
  static volatile unsigned long _echoEndTime;
  static volatile bool _newDistanceAvailable;
  volatile long _distanceCm = 0;

  static ProximitySensor* _instance;
  static void IRAM_ATTR echo_isr();
};
