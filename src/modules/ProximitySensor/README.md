# ProximitySensor Module

The ProximitySensor module manages HC-SR04 ultrasonic sensors for collision avoidance, providing non-blocking distance measurements with built-in filtering and timeout protection.

## Overview

This module provides a unified interface for managing ultrasonic proximity sensors. It handles the complete sensor lifecycle including triggering pings, capturing echo responses via hardware interrupts, applying smoothing filters to noisy readings, and intelligent logging that only reports when objects enter warning zones.

## Key Features

- **Non-Blocking Operation**: Uses hardware interrupts for fully non-blocking distance reads
- **Multi-Sensor Support**: Correctly handles multiple sensor instances simultaneously
- **Noise Filtering**: Moving average filter smooths out erratic ultrasonic readings
- **Timeout Protection**: Detects and reports sensor communication failures
- **Intelligent Logging**: Only logs when objects enter the warning zone

## Hardware Requirements

### HC-SR04 Ultrasonic Sensor Wiring

| HC-SR04 Pin | ESP32-S3 Connection | Function      |
| ----------- | ------------------- | ------------- |
| VCC         | 5V or 3.3V          | Power         |
| GND         | GND                 | Ground        |
| Trig        | TRIG_PIN (GPIO)     | Trigger input |
| Echo        | ECHO_PIN (GPIO)     | Echo output   |

### Front and Back Sensor Configuration

The system uses two HC-SR04 sensors:

- **Front Sensor**: GPIO 12 (Trig), GPIO 13 (Echo) - for forward collision avoidance
- **Back Sensor**: GPIO 35 (Trig), GPIO 36 (Echo) - for reverse collision avoidance

Note: Back sensor uses ADC2 pins to avoid WiFi conflicts on ESP32-S3.

## Usage Example

```cpp
#include "ProximitySensor.h"
#include "Logger.h"

Logger frontLogger("Front Sensor");
ProximitySensor frontSensor(FRONT_TRIG_PIN, FRONT_ECHO_PIN,
                            &frontLogger,
                            50,   // Log below 50cm
                            5);   // 5-sample smoothing window

void setup() {
    frontSensor.setup();
}

void loop() {
    frontSensor.update();
    long distance = frontSensor.getDistanceCm();

    if (distance < 30) {
        Serial.println("Obstacle detected!");
    }
}
```

## API Reference

### Constructor

```cpp
ProximitySensor(int trigPin, int echoPin, Logger* logger,
                int loggingThreshold, int windowSize);
```

### Methods

| Method               | Description                                  | Returns                  |
| -------------------- | -------------------------------------------- | ------------------------ |
| `setup()`            | Initialize the sensor and attach interrupt   | void                     |
| `update()`           | Trigger ping if due and process new readings | void                     |
| `getDistanceCm()`    | Get smoothed distance in centimeters         | long (cm)                |
| `getRawDistanceCm()` | Get most recent raw reading for diagnostics  | long (cm or -1 on error) |

### Parameters

| Parameter        | Type     | Description                                       |
| ---------------- | -------- | ------------------------------------------------- |
| trigPin          | int      | GPIO pin connected to Trig input                  |
| echoPin          | int      | GPIO pin connected to Echo output                 |
| logger           | Logger\* | Pointer to Logger for debug output                |
| loggingThreshold | int      | Distance below which readings will be logged (cm) |
| windowSize       | int      | Number of samples for moving average filter       |

## Implementation Details

### Interrupt-Based Operation

The module uses ESP32's external interrupt capability to capture echo pulse timing:

- **ISR Handler**: `IRAM_ATTR handleInterrupt()` captures start/end times in microseconds
- **Multi-Sensor Support**: Each instance registers its own ISR with a pointer to itself
- **Static Bridge**: `isr_handler()` is the C-style function that calls instance methods

### Distance Calculation

```cpp
duration = echoEndTime - echoStartTime;
distance_cm = duration * 0.0343 / 2;
```

The speed of sound (0.0343 cm/µs) is used with the round-trip time divided by 2 for one-way distance.

### Moving Average Filter

A rolling window buffer maintains recent readings:

1. Subtract oldest value from running sum
2. Add new reading to buffer and sum
3. Calculate average = sum / window_size
4. Advance index (wraps around)

This smooths out erratic ultrasonic readings caused by:

- Surface reflectivity variations
- Multiple echo paths
- Electrical noise

### Timeout Detection

If no echo is received within the timeout period, the sensor is marked as timed out:

- Default timeout: 100ms (covers ~17m maximum range)
- On timeout, returns -1 to indicate error condition
- Logs "Sensor Timeout" message for debugging

## Troubleshooting

| Issue                        | Possible Cause                    | Solution                                          |
| ---------------------------- | --------------------------------- | ------------------------------------------------- |
| Inconsistent readings        | Surface too reflective/absorptive | Add angled mounting or different sensor type      |
| Zero distance always         | Echo pin not connected            | Verify wiring and GPIO assignments                |
| Timeout errors               | Object beyond max range           | Increase logging threshold or check sensor health |
| Multiple sensors interfering | Sensors triggering simultaneously | Stagger ping intervals manually if needed         |

## Safety Considerations

- Always verify sensor readings before collision-critical operations
- Use timeout detection to handle failed sensors gracefully
- Mount sensors securely and protect from vibration damage
- Consider environmental factors (temperature affects sound speed)
