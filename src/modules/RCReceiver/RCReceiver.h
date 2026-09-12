#pragma once
#include <Arduino.h>
#include "../Logger/Logger.h"

/**
 * @class RCReceiver
 * @brief Reads and interprets a 2-channel RC receiver with configurable dead zones.
 *
 * Pulse widths are measured in hardware interrupts (non-blocking), so the main
 * control loop is never stalled while waiting for a servo signal edge. This is
 * critical for the safety loop: with the previous blocking pulseIn() approach
 * each loop iteration could stall for 20-50 ms, delaying collision avoidance
 * and braking.
 *
 * Signal-loss detection: if no pulse is seen on a channel within
 * _signalTimeoutMs the channel is considered LOST. Lost channels report
 * neutral (0) and never trigger an override, and isSignalValid() lets the
 * application raise a failsafe/alert when the transmitter link drops.
 */
class RCReceiver {
public:
  /**
   * @brief Constructor for the RCReceiver.
   * @param throttlePin The GPIO pin for the throttle channel (RC servo PWM).
   * @param steeringPin The GPIO pin for the steering channel (RC servo PWM).
   * @param logger A pointer to a Logger object for debugging.
   * @param throttleNeutralMin The minimum pulse width (µs) for throttle neutral.
   * @param throttleNeutralMax The maximum pulse width (µs) for throttle neutral.
   * @param steeringNeutralMin The minimum pulse width (µs) for steering neutral.
   * @param steeringNeutralMax The maximum pulse width (µs) for steering neutral.
   * @param signalTimeoutMs A channel is "lost" after this many ms without a pulse.
   */
  RCReceiver(int throttlePin, int steeringPin, Logger* logger,
             int throttleNeutralMin, int throttleNeutralMax,
             int steeringNeutralMin, int steeringNeutralMax,
             unsigned long signalTimeoutMs = 60);

  void setup();

  /**
   * @brief Refresh channel validity and logging. Non-blocking.
   */
  void update();

  /** @return Throttle -100..100; 0 when the channel signal is lost. */
  int getThrottle();

  /** @return Steering -100..100; 0 when the channel signal is lost. */
  int getSteering();

  /** @return true when the throttle channel is actively outside its dead zone. */
  bool isOverriding();

  /** @return true when the steering channel is actively outside its dead zone. */
  bool isSteeringActive();

  /** @return true when a valid pulse train is currently present on BOTH channels. */
  bool isSignalValid();

private:
  int _throttlePin;
  int _steeringPin;
  Logger* _logger;

  const int _throttleNeutralMin;
  const int _throttleNeutralMax;
  const int _steeringNeutralMin;
  const int _steeringNeutralMax;
  const unsigned long _signalTimeoutMs;

  // Latest measured pulse widths (µs), written by the ISR.
  volatile long _rawThrottle = 1500;
  volatile long _rawSteering = 1500;

  // ISR bookkeeping.
  volatile bool _throttlePulsing = false;
  volatile bool _steeringPulsing = false;
  volatile unsigned long _throttleStartUs = 0;
  volatile unsigned long _steeringStartUs = 0;
  volatile unsigned long _lastThrottleMs = 0;
  volatile unsigned long _lastSteeringMs = 0;

  // Channel validity (main-loop context).
  bool _throttleValid = false;
  bool _steeringValid = false;
  bool _wasSignalValid = false;

  // ISR bridge (C-style handler -> class instance).
  void IRAM_ATTR handleISR();
  static void IRAM_ATTR isr_handler(void* arg);
};
