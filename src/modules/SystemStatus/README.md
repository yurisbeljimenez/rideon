# SystemStatus Module

The SystemStatus module controls the onboard RGB LED to provide visual feedback about the car's current operational state.

## Overview

This module manages a NeoPixel LED that provides intuitive visual indicators for different driving conditions:

- **Solid Blue**: Car is stopped
- **Solid Green**: Car is moving forward
- **Solid Yellow**: Car is moving in reverse
- **Pulsing Red**: Car is avoiding an obstacle
- **Pulsing White**: Parental override is active

## Hardware Requirements

- NeoPixel LED (RGB)
- GPIO pin connected to the LED data line
- 3.3V power supply (LED is typically 5V tolerant but should be powered from 3.3V for ESP32 compatibility)

## Pin Configuration

The module uses the following pin configuration:

- **RGB_PIN**: GPIO pin connected to the NeoPixel LED data line (defined in `src/main.cpp`)

## Implementation Details

### Color Definitions

The module defines specific RGB color values for each state:

- `COLOR_BLUE`: 0, 0, 40 (dark blue)
- `COLOR_GREEN`: 0, 40, 0 (green)
- `COLOR_YELLOW`: 40, 40, 0 (yellow)
- `COLOR_RED`: 40, 0, 0 (red)
- `COLOR_WHITE`: 40, 40, 40 (white)
- `COLOR_OFF`: 0, 0, 0 (off)

### State Handling

The module implements two primary LED behaviors:

1. **Solid Color**: For normal driving states (stopped, forward, reverse)
2. **Pulsing Color**: For warning states (avoiding obstacle, manual override)

### Optimization

The implementation includes optimizations to reduce unnecessary LED updates:

- Only changes the LED color when the requested state differs from the current state
- Uses timestamp-based timing for pulsing effects to maintain responsiveness

## Integration with Main System

The SystemStatus module is integrated into the main control loop in `src/main.cpp` and receives updates based on the current `CarState`. The module is initialized in the `setup()` function and updated in each iteration of the main `loop()`.

## Usage Example

```cpp
// In main.cpp setup function:
SystemStatus systemStatus(RGB_PIN, &systemStatusLogger);

// In main.cpp loop function:
systemStatus.update(currentState);
```

## Dependencies

- `Adafruit_NeoPixel` library
- `Logger` module for debugging output
- `CarState` enum from shared headers
