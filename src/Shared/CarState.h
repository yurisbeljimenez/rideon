#pragma once

// This file contains definitions that need to be shared across multiple modules.
// It must stay pure C++ (no Arduino dependencies) so the control logic built on
// top of it can be unit tested on the host (see test/test_native).

// State Machine: Defines the possible states of the car.
enum class CarState {
  STOPPED,
  FORWARD,
  REVERSE,
  AVOIDING_OBSTACLE,
  MANUAL_OVERRIDE
};

// Gear: Defines the possible gear positions (Forward/Reverse).
enum class Gear {
  FORWARD,
  REVERSE
};

// Human-readable name for a car state (used by logging and diagnostics).
inline const char* carStateName(CarState state) {
  switch (state) {
    case CarState::STOPPED:            return "STOPPED";
    case CarState::FORWARD:            return "FORWARD";
    case CarState::REVERSE:            return "REVERSE";
    case CarState::AVOIDING_OBSTACLE:  return "AVOIDING_OBSTACLE";
    case CarState::MANUAL_OVERRIDE:    return "MANUAL_OVERRIDE";
  }
  return "UNKNOWN";
}
