#pragma once

#include "CarState.h"

//================================================================================
// ControlLogic - pure, hardware-free control decision logic for the ride-on car.
//
// This is the heart of the safety hierarchy, implemented as a plain function
// (no Arduino APIs, no globals) so it can be unit tested on the host:
//
//     Priority 0: Emergency stop button        (absolute, hard stop)
//     Priority 1: Collision avoidance          (works in ALL driving states,
//                                               including parental override;
//                                               fail-closed on sensor loss)
//     Priority 2: Parental RC override         (direct throttle + steering)
//     Priority 3: Child pedal + gear input     (3-stage acceleration curve)
//
// The function is a pure state transition: given the inputs and the previous
// CarControlState it returns the new state plus the exact motor/steering
// commands. All timing is injected via `dtMs`, so tests can run without any
// real-time dependency.
//================================================================================

// Tuning parameters for the control loop.
struct TuneParams {
  // Acceleration curve (ms per speed step; a speed step is 1% of 0..100).
  long accelIntervalLow = 30;    // 0..25%  gentle start
  long accelIntervalMid = 10;    // 26..80% responsive mid range
  long accelIntervalHigh = 25;   // 81..100% ease to top speed
  long brakingInterval = 5;      // deceleration (single-pedal braking)

  // Speed zones for the acceleration curve (in speed units 0..100).
  int speedThresholdLow = 25;
  int speedThresholdHigh = 80;

  // Collision avoidance: dynamic minimum safe distance (cm).
  int minSafetyCm = 20;          // at zero speed
  int maxSafetyCm = 80;          // at 100% speed (linear interpolation)

  // Speed (0..100) above which the car counts as "moving" (state START/STOP).
  int startStopSpeed = 2;

  // Pedal intent (0..100) below which the pedal counts as "released".
  int pedalDeadband = 2;

  // Maximum motor output the parental RC may command (0..255).
  int rcMaxSpeed = 255;
};

// Everything the state machine needs to make a decision.
struct ControlInputs {
  bool estopPressed = false;

  // Child input
  int pedalIntent = 0;           // 0..100 (raw, filtered pedal position)
  Gear gear = Gear::FORWARD;

  // Parental RC input
  int rcThrottle = 0;            // -100..100 (0 when signal lost)
  int rcSteering = 0;            // -100..100
  bool rcSignalValid = true;     // false when the transmitter link is down
  bool rcOverrideActive = false; // throttle outside dead zone (signal valid)

  // Obstacle sensors (cm; negative value means "invalid/no reading")
  long frontCm = -1;
  bool frontValid = false;
  long backCm = -1;
  bool backValid = false;

  // Time since the previous call, in milliseconds.
  unsigned long dtMs = 10;
};

// Persistent control state (owned by the application, stepped by stepControl).
struct CarControlState {
  CarState state = CarState::STOPPED;
  int currentSpeed = 0;          // 0..100 ramped speed of the child-drive system
  int avoidDir = 0;              // +1 forward, -1 reverse, 0 none (set when entering AVOIDING)
  long speedTimerMs = 0;         // accumulator for the acceleration/braking ramp
};

// Result of one step: the new state plus the actuator commands.
struct ControlOutputs {
  int motorSpeed = 0;            // -255..255
  int steering = 0;              // -100..100
  bool fault = false;            // true when a safety fault is active (e-stop,
                                 // avoidance, or lost RC link) - drives the alarm
};

namespace control {

//================================================================================
// STATE MACHINE TRANSITION DIAGRAM
//
//   ┌─────────────────────────────────────────────────────────────────┐
//   │                        E-STOP (Priority 0)                       │
//   │   ANY STATE ──────────────────────────────► STOPPED (hard stop)  │
//   └─────────────────────────────────────────────────────────────────┘
//
//   ┌─────────────────────────────────────────────────────────────────┐
//   │                 COLLISION AVOIDANCE (Priority 1)                │
//   │                                                                 │
//   │   FORWARD ──[obstacle ahead]──────────► AVOIDING_OBSTACLE       │
//   │   REVERSE ──[obstacle behind]─────────► AVOIDING_OBSTACLE       │
//   │   MANUAL_OVERRIDE ──[obstacle]────────► AVOIDING_OBSTACLE       │
//   │                                                                 │
//   │   AVOIDING_OBSTACLE ──[clear + pedal released]──► STOPPED       │
//   │   AVOIDING_OBSTACLE ──[clear + pedal held]────► HOLD (fault)    │
//   └─────────────────────────────────────────────────────────────────┘
//
//   ┌─────────────────────────────────────────────────────────────────┐
//   │                 RC OVERRIDE (Priority 2)                        │
//   │                                                                 │
//   │   STOPPED ──[RC throttle active]──► MANUAL_OVERRIDE             │
//   │   FORWARD ──[RC throttle active]─► MANUAL_OVERRIDE              │
//   │   REVERSE ──[RC throttle active]─► MANUAL_OVERRIDE              │
//   │                                                                 │
//   │   MANUAL_OVERRIDE ──[RC lost]──► STOPPED (fail-safe)            │
//   └─────────────────────────────────────────────────────────────────┘
//
//   ┌─────────────────────────────────────────────────────────────────┐
//   │                 CHILD PEDAL (Priority 3)                        │
//   │                                                                 │
//   │   STOPPED ──[pedal > deadband + speed > threshold]──► FORWARD   │
//   │   STOPPED ──[pedal > deadband + speed > threshold]──► REVERSE   │
//   │   FORWARD ──[pedal = 0, speed → 0]──────────────────► STOPPED   │
//   │   REVERSE ──[pedal = 0, speed → 0]─────────────────► STOPPED    │
//   └─────────────────────────────────────────────────────────────────┘
//
// PRIORITY HIERARCHY: E-STOP > COLLISION > RC > PEDAL
// Each higher priority can preempt lower priorities at any time.
//================================================================================

inline int clampI(int v, int lo, int hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}

// Linear interpolation with clamped input; works for outLo < outHi or outLo > outHi.
inline long mapRange(long v, long lo, long hi, long outLo, long outHi) {
  if (hi == lo) return outLo;
  if (v < lo) v = lo;
  if (v > hi) v = hi;
  return outLo + (v - lo) * (outHi - outLo) / (hi - lo);
}

// Speed (0..100) -> motor units (0..255).
inline int speedToMotor(int speed) {
  return (int)((long)clampI(speed, 0, 100) * 255L / 100L);
}

// Dynamic minimum safe distance based on speed (0..100).
inline long dynamicSafetyDistance(int speed, int minCm, int maxCm) {
  return mapRange(clampI(speed, 0, 100), 0, 100, minCm, maxCm);
}

// Advance the speed ramp towards `target` (0..100) using the stage-dependent
// intervals. Multiple steps per call are allowed (a slow loop does not slow
// the ramp down).
inline void rampSpeed(CarControlState& st, int target, unsigned long dtMs, const TuneParams& t) {
  target = clampI(target, 0, 100);
  if (st.currentSpeed == target) return;
  const bool accel = st.currentSpeed < target;
  st.speedTimerMs += (long)dtMs;
  int steps = 0;
  while (st.speedTimerMs > 0 && st.currentSpeed != target && steps < 100) {
    long interval;
    if (accel) {
      if (st.currentSpeed < t.speedThresholdLow)      interval = t.accelIntervalLow;
      else if (st.currentSpeed < t.speedThresholdHigh) interval = t.accelIntervalMid;
      else                                             interval = t.accelIntervalHigh;
    } else {
      interval = t.brakingInterval;
    }
    if (interval <= 0) interval = 1;
    if (st.speedTimerMs < interval) break;
    st.speedTimerMs -= interval;
    st.currentSpeed += accel ? 1 : -1;
    steps++;
  }
  if (st.currentSpeed == target) st.speedTimerMs = 0;
}

// The single control step. Advances `st` and returns the actuator commands.
inline ControlOutputs stepControl(const ControlInputs& in, CarControlState& st, const TuneParams& t) {
  ControlOutputs out;

  // ---- Priority 0: Emergency stop -------------------------------------------
  if (in.estopPressed) {
    st.state = CarState::STOPPED;
    st.currentSpeed = 0;
    st.speedTimerMs = 0;
    st.avoidDir = 0;
    out.motorSpeed = 0;
    out.steering = 0;
    out.fault = true;
    return out;
  }

  // ---- Determine travel direction (for hazard detection) --------------------
  bool movingFwd = (st.state == CarState::FORWARD) ||
                   (st.state == CarState::MANUAL_OVERRIDE && in.rcThrottle > 0);
  bool movingRev = (st.state == CarState::REVERSE) ||
                   (st.state == CarState::MANUAL_OVERRIDE && in.rcThrottle < 0);
  if (st.state == CarState::AVOIDING_OBSTACLE) {
    // While avoiding, keep watching the direction we were heading so the
    // car cannot creep back into the obstacle.
    movingFwd = (st.avoidDir > 0);
    movingRev = (st.avoidDir < 0);
  }

  // ---- Priority 1: Collision avoidance (fail-closed) ------------------------
  // Effective speed for the dynamic safety distance.
  int speedForSafety = (st.state == CarState::MANUAL_OVERRIDE)
                           ? (in.rcThrottle < 0 ? -in.rcThrottle : in.rcThrottle)
                           : st.currentSpeed;
  long safetyCm = dynamicSafetyDistance(speedForSafety, t.minSafetyCm, t.maxSafetyCm);

  bool hazard = false;
  if (movingFwd && (!in.frontValid || in.frontCm < safetyCm)) hazard = true;
  if (movingRev && (!in.backValid || in.backCm < safetyCm)) hazard = true;

  if (hazard) {
    if (st.state != CarState::AVOIDING_OBSTACLE) {
      st.avoidDir = movingFwd ? 1 : (movingRev ? -1 : 0);
    }
    st.state = CarState::AVOIDING_OBSTACLE;
    // Soft stop: ramp to zero at the fast braking rate, regardless of the pedal.
    rampSpeed(st, 0, in.dtMs, t);
    out.motorSpeed = (st.avoidDir >= 0 ? 1 : -1) * speedToMotor(st.currentSpeed);
    if (st.avoidDir == 0) out.motorSpeed = 0;
    out.steering = 0;
    out.fault = true;
    return out;
  }

  // ---- Leaving AVOIDING: obstacle cleared -----------------------------------
  if (st.state == CarState::AVOIDING_OBSTACLE) {
    st.currentSpeed = 0;
    st.speedTimerMs = 0;
    out.motorSpeed = 0;
    out.steering = 0;
    if (in.pedalIntent < t.pedalDeadband) {
      // Pedal released and path clear: hand control back, at zero speed.
      st.state = CarState::STOPPED;
      st.avoidDir = 0;
    } else {
      // Pedal still held: hold the stop (driver must lift off first).
      st.avoidDir = 0;
      out.fault = true;
    }
    return out;
  }

  // ---- Priority 2: Parental RC override -------------------------------------
  if (in.rcOverrideActive) {
    st.state = CarState::MANUAL_OVERRIDE;
    st.avoidDir = 0;
    out.motorSpeed = (int)mapRange(in.rcThrottle, -100, 100, -t.rcMaxSpeed, t.rcMaxSpeed);
    out.steering = clampI(in.rcSteering, -100, 100);
    if (!in.rcSignalValid) out.fault = true;
    return out;
  }

  if (st.state == CarState::MANUAL_OVERRIDE) {
    // Override dropped: fail safe to a stopped car.
    st.state = CarState::STOPPED;
    st.currentSpeed = 0;
    st.speedTimerMs = 0;
    st.avoidDir = 0;
    out.motorSpeed = 0;
    out.steering = 0;
    if (!in.rcSignalValid) out.fault = true;  // transmitter lost mid-override
    return out;
  }

  // ---- Priority 3: Child pedal + gear ---------------------------------------
  rampSpeed(st, in.pedalIntent, in.dtMs, t);

  if (st.currentSpeed > t.startStopSpeed) {
    st.state = (in.gear == Gear::FORWARD) ? CarState::FORWARD : CarState::REVERSE;
  } else {
    st.state = CarState::STOPPED;
  }

  const int dir = (in.gear == Gear::FORWARD) ? 1 : -1;
  out.motorSpeed = dir * speedToMotor(st.currentSpeed);
  out.steering = 0;
  if (!in.rcSignalValid) out.fault = true;   // parental link down: alert
  return out;
}

} // namespace control
