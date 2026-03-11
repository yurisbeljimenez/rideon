# SteeringController Module

The SteeringController module manages steering control via an ESC (Electronic Speed Controller) using standard RC servo signals.

## Overview

This module uses the ESP32Servo library to generate precise pulse-width modulation (PWM) signals required by the Radiolink CL9030 steering ESC. It provides intuitive direction control (-100 for full left, +100 for full right, 0 for center).

## Key Features

- **Standard Servo Protocol**: Uses ESP32Servo library for reliable servo signal generation
- **Direction Control**: Intuitive -100 to +100 range for steering direction
- **Auto-Center on Startup**: Initializes with neutral steering position
- **Intelligent Logging**: Logs all steering commands for debugging

## Hardware Requirements

### Steering ESC Configuration

| Component    | Model                  | Notes                            |
| ------------ | ---------------------- | -------------------------------- |
| Steering ESC | Radiolink CL9030 (90A) | Brushed ESC functioning as servo |

### Electrical Connections

| ESC Pin | ESP32-S3 Connection          | Function                 |
| ------- | ---------------------------- | ------------------------ |
| Signal  | STEERING_SERVO_PIN (GPIO 17) | PWM control signal       |
| VCC     | 5V or receiver voltage       | Power (check ESC rating) |
| GND     | GND                          | Ground                   |

## Usage Example

```cpp
#include "SteeringController.h"
#include "Logger.h"

Logger steeringLogger("Steering");
SteeringController steeringController(STEERING_SERVO_PIN, &steeringLogger);

void setup() {
    steeringController.setup();
}

void loop() {
    // Turn left (full)
    steeringController.setSteering(-100);

    // Center position
    steeringController.setSteering(0);

    // Turn right (partial)
    steeringController.setSteering(50);
}
```

## API Reference

### Constructor

```cpp
SteeringController(int servoPin, Logger* logger);
```

### Methods

| Method                       | Description                                            | Parameters                  | Returns |
| ---------------------------- | ------------------------------------------------------ | --------------------------- | ------- |
| `setup()`                    | Initialize the steering controller and center position | None                        | void    |
| `setSteering(int direction)` | Set steering direction                                 | -100 to 100 (left to right) | void    |

### Direction Values

| Value | Position   | Description              |
| ----- | ---------- | ------------------------ |
| -100  | Full Left  | Maximum left turn        |
| -50   | Half Left  | Partial left turn        |
| 0     | Center     | Straight ahead (neutral) |
| +50   | Half Right | Partial right turn       |
| +100  | Full Right | Maximum right turn       |

## Implementation Details

### Signal Mapping

The module maps its internal direction range to servo pulse widths:

```cpp
int servoCommand = map(direction, -100, 100, 0, 180);
_servo.write(servoCommand);
```

| Direction | Servo Angle | Approx. Pulse Width |
| --------- | ----------- | ------------------- |
| -100      | 0°          | 500µs (full left)   |
| 0         | 90°         | 1500µs (center)     |
| +100      | 180°        | 2500µs (full right) |

### Initialization Behavior

On startup, the module:

1. Attaches the servo to the specified GPIO pin
2. Calls `setSteering(0)` to center the steering wheel
3. Logs "Initialized" message if logger is provided

## Troubleshooting

| Issue                 | Possible Cause               | Solution                        |
| --------------------- | ---------------------------- | ------------------------------- |
| Steering doesn't move | ESC not powered              | Check power connections         |
| Erratic movement      | Servo signal interference    | Add ferrite bead on signal line |
| Limited range         | ESC needs calibration        | Perform ESC trim calibration    |
| Center drift          | ESC not calibrated to center | Recalibrate neutral position    |

## Calibration Procedure

To calibrate the steering ESC for full range:

1. Power up the system with steering centered
2. Follow ESC manufacturer's calibration procedure (typically power cycle with stick at extremes)
3. Verify that -100 and +100 commands produce full left/right positions
4. Adjust if necessary in software or via ESC trim settings

## Safety Considerations

- Always verify steering range before operation
- Ensure the ESC is properly calibrated for your specific servo horn geometry
- Consider adding limits to prevent mechanical over-travel damage
- Implement steering loss detection (e.g., timeout if not receiving updates)

## Integration with Proximity Sensors

The SteeringController works in conjunction with proximity sensors:

1. When an obstacle is detected, the car enters AVOIDING_OBSTACLE state
2. Steering may be adjusted to navigate around obstacles
3. Parental RC override takes precedence over autonomous steering commands
