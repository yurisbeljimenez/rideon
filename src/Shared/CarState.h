#pragma once

// This file contains definitions that need to be shared across multiple modules.

// State Machine: Defines the possible states of the car.
enum class CarState { 
  STOPPED, 
  FORWARD, 
  REVERSE, 
  AVOIDING_OBSTACLE, 
  MANUAL_OVERRIDE 
};
