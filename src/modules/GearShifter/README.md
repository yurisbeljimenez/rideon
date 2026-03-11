# GearShifter Module

The GearShifter module manages the car's gear state (Forward/Reverse) via a momentary pushbutton interface.

## Overview

This module detects button presses and toggles between Forward and Reverse gear states. It implements non-blocking debounce timing to prevent multiple triggers from a single press event.

## Key Features

- **Non-Blocking Debounce**: Uses `millis()` for timing instead of blocking delays
- **State Tracking**: Maintains current gear state (FORWARD or REVERSE)
- **Intelligent Logging**: Only logs when gear changes occur
- **Default State**: Defaults to FORWARD on initialization

## Hardware Requirements

### Gear Shifter Button Wiring

| Component Pin | ESP32-S3 Connection | Function          |
| ------------- | ------------------- | ----------------- |
| Switch Pin 1  | GND                 | Ground            |
| Switch Pin 2  | SHIFTER_PIN (GPIO)  | Input with pullup |

### Recommended Components

- **Momentary Pushbutton** - SPST normally open switch
- **Pull-up Resistor** - Internal pull-up enabled via `INPUT_PULLUP` mode

## Usage Example

```cpp
#include "GearShifter.h"
#include "Logger.h"

Logger shifterLogger("Gear Shifter");
GearShifter shifter(SHIFTER_PIN, &shifterLogger);

void setup() {
    shifter.setup();
}

void loop() {
    shifter.update();

    if (shifter.getGear() == Gear::FORWARD) {
        Serial.println("Car in Forward gear");
    } else {
        Serial.println("Car in Reverse gear");
    }
}
```

## API Reference

### Constructor

```cpp
GearShifter(int pin, Logger* logger);
```

### Methods

| Method      | Description                                        | Returns                        |
| ----------- | -------------------------------------------------- | ------------------------------ |
| `setup()`   | Initialize the shifter with pull-up mode           | void                           |
| `update()`  | Check for button press and toggle gear if detected | void                           |
| `getGear()` | Get current gear state                             | Gear enum (FORWARD or REVERSE) |

### Enumerations

```cpp
enum class Gear {
    FORWARD,  // Forward driving mode
    REVERSE   // Reverse driving mode
};
```

## Implementation Details

### Debounce Timing

The module uses non-blocking debounce timing:

```cpp
if (currentMillis - _lastDebounceTime >= _debounceDelay) {
    // Process button press and update timestamp
}
```

- **Default Delay**: 50ms
- **Mechanism**: Only processes new presses after the delay has elapsed since last change

### Button State Detection

The module detects a press event by monitoring for the transition:

| Previous State        | Current State   | Action                    |
| --------------------- | --------------- | ------------------------- |
| HIGH (released)       | LOW (pressed)   | Toggle gear state         |
| LOW (pressed)         | HIGH (released) | No action                 |
| Any other combination | -               | Wait for clean transition |

### Logging Behavior

Gear changes are logged automatically:

- Press button while in FORWARD → Logs "REVERSE"
- Press button while in REVERSE → Logs "FORWARD"

## Troubleshooting

| Issue                           | Possible Cause                 | Solution                                          |
| ------------------------------- | ------------------------------ | ------------------------------------------------- |
| Multiple gear toggles per press | Debounce delay too short       | Increase `_debounceDelay` value                   |
| No response to button press     | Loose wiring or switch failure | Check connections, test with multimeter           |
| Gear changes spontaneously      | Electrical noise               | Add external pull-up resistor, check power supply |

## Integration Notes

The GearShifter module is integrated into the main state machine:

1. **Initial State**: Car starts in STOPPED state
2. **Gear Selection**: When accelerator is pressed, car transitions to FORWARD or REVERSE based on current gear
3. **Safety Check**: Forward motion requires FORWARD gear; reverse motion requires REVERSE gear

## Safety Considerations

- Always verify current gear before starting movement
- The module defaults to FORWARD for safety (prevents unexpected reverse)
- Consider adding visual feedback (LED indicator) showing current gear state
