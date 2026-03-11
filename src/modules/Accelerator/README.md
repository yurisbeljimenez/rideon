# Accelerator Module

The Accelerator module manages single-pedal driving with intelligent smoothing and a 3-stage acceleration curve for optimal ride quality.

## Overview

This module reads an analog pedal input, applies sophisticated smoothing algorithms, and provides controlled motor output based on the driver's pedal position. It implements active braking when the pedal is released (single-pedal driving).

## Key Features

- **3-Stage Acceleration Curve**: Gentle start, responsive mid-range, smooth top speed
- **Active Braking**: Fast deceleration when pedal is released for single-pedal control
- **Analog Filtering**: Moving average filter to reduce noise from motor drivers
- **Intelligent Logging**: Only logs when output changes significantly

## Acceleration Curve Stages

| Stage                | Speed Range | Response Time | Purpose                             |
| -------------------- | ----------- | ------------- | ----------------------------------- |
| Gentle Start         | 0-25%       | ~150ms/step   | Prevents jerky starts from rest     |
| Responsive Mid-Range | 26-80%      | ~50ms/step    | Provides responsive driving control |
| Ease to Top Speed    | 81-100%     | ~125ms/step   | Smooth approach to maximum speed    |

## Active Braking

When the pedal is released, the module rapidly decreases motor output (25ms per step) for immediate response - enabling single-pedal driving where lifting off the accelerator causes rapid deceleration.

## Hardware Requirements

- Analog input pin connected to:
  - Potentiometer (variable resistor), or
  - Hall effect sensor (non-contact position sensing)

## Pin Configuration

The module accepts a GPIO pin number in its constructor, which should be connected to an ADC-capable pin on the ESP32-S3.

## Usage Example

```cpp
#include "Accelerator.h"
#include "Logger.h"

Logger accelLogger("Accelerator");
Accelerator accelerator(PEDAL_PIN, &accelLogger,
    30,  // Gentle start interval (ms)
    10,  // Mid-range interval (ms)
    25,  // Top speed interval (ms)
    5,   // Braking interval (ms)
    5    // Logging threshold
);

void setup() {
    accelerator.setup();
}

void loop() {
    accelerator.update();
    int motorOutput = accelerator.getMotorOutput();
    int currentSpeed = accelerator.getCurrentSpeed();
}
```

## API Reference

### Constructor

```cpp
Accelerator(int pedalPin, Logger* logger,
            long accelIntervalLow,
            long accelIntervalMid,
            long accelIntervalHigh,
            long brakingInterval,
            int loggingThreshold);
```

### Methods

| Method                     | Description                                 |
| -------------------------- | ------------------------------------------- |
| `setup()`                  | Initialize the accelerator module           |
| `update()`                 | Read pedal input and update motor output    |
| `getMotorOutput()`         | Get current PWM value (0-255)               |
| `getCurrentSpeed()`        | Get current speed percentage (0-100)        |
| `overrideSpeed(int speed)` | Override with direct speed control (0-100%) |

## Implementation Details

### Analog Filtering

The module uses an 8-element moving average filter to smooth raw analog readings. This reduces noise from:

- Motor driver electrical interference
- Potentiometer wear and contact noise
- Power supply fluctuations

### Speed Mapping

Raw ADC values (0-4095) are mapped to speed percentage (0-100%) using the `map()` function. The filtered value is used for all calculations.

## Troubleshooting

| Issue              | Possible Cause                       | Solution                                  |
| ------------------ | ------------------------------------ | ----------------------------------------- |
| Jerky acceleration | Calibration intervals too aggressive | Increase interval values                  |
| No response        | Pedal not calibrated                 | Check analog readings with Serial Monitor |
| Oscillating speed  | Noise interference                   | Verify filtering is enabled               |
