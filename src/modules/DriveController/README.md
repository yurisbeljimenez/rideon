# DriveController Module

The DriveController module manages the Cytron MDDS30 dual motor driver for precise speed and direction control of the ride-on car's drive motors.

## Overview

This module translates motor speed commands (-255 to 255) into appropriate Direction (digital signal) and Speed (hardware PWM) signals for the Cytron MDDS30 motor driver. It includes error handling and initialization verification.

## Key Features

- **Bidirectional Control**: Full forward and reverse capability
- **Hardware PWM**: Uses ESP32 LEDC hardware for precise, non-blocking speed control
- **Error Tracking**: Monitors initialization status to prevent command failures
- **Intelligent Logging**: Logs all speed commands for debugging

## Hardware Requirements

### Cytron MDDS30 Motor Driver Configuration

The module is designed for "PWM Signed Magnitude" mode. Configure the DIP switches on the MDDS30 as follows:

| Switch | Position | Function                                            |
| ------ | -------- | --------------------------------------------------- |
| SW1    | ON       | Enable direction control                            |
| SW2    | OFF      | Disable PWM signed magnitude (use external dir pin) |
| SW3    | ON       | Configure for bidirectional operation               |
| SW4    | ON       | Set mode parameters                                 |
| SW5    | OFF      | Disable protection features as needed               |
| SW6    | ON       | Final configuration bit                             |

### Electrical Connections

| MDDS30 Pin | ESP32-S3 Connection    | Function                       |
| ---------- | ---------------------- | ------------------------------ |
| DIR1/DIR2  | MOTOR_DIR_PIN (GPIO 1) | Direction control              |
| PWM1/PWM2  | MOTOR_PWM_PIN (GPIO 2) | Speed control (LEDC channel 0) |

## Usage Example

```cpp
#include "DriveController.h"
#include "Logger.h"

Logger driveLogger("Drive Controller");
DriveController driveController(MOTOR_DIR_PIN, MOTOR_PWM_PIN, MOTOR_PWM_CHANNEL, &driveLogger);

void setup() {
    driveController.setup();
}

void loop() {
    // Forward at 50% speed
    driveController.setSpeed(127);

    // Reverse at 30% speed
    driveController.setSpeed(-76);

    // Stop
    driveController.setSpeed(0);
}
```

## API Reference

### Constructor

```cpp
DriveController(int dirPin, int pwmPin, int pwmChannel, Logger* logger);
```

### Methods

| Method                | Description                     | Parameters  | Returns |
| --------------------- | ------------------------------- | ----------- | ------- |
| `setup()`             | Initialize the drive controller | None        | void    |
| `setSpeed(int speed)` | Set motor speed and direction   | -255 to 255 | void    |
| `isInitialized()`     | Check if properly initialized   | None        | bool    |

### Speed Values

| Value Range | Direction | Description              |
| ----------- | --------- | ------------------------ |
| 0           | Neutral   | Motor stopped            |
| 1-255       | Forward   | Increasing forward speed |
| -1 to -255  | Reverse   | Increasing reverse speed |

## Implementation Details

### PWM Configuration

The module configures the ESP32's LEDC hardware with:

- **Frequency**: 5000 Hz (quiet operation)
- **Resolution**: 8-bit (0-255 duty cycle range)

### Initialization Check

After setup, the module verifies that PWM was successfully configured by reading back the channel value. If initialization fails, all subsequent `setSpeed()` calls will be rejected and logged.

## Troubleshooting

| Issue                 | Possible Cause        | Solution                                      |
| --------------------- | --------------------- | --------------------------------------------- |
| Motor doesn't respond | Initialization failed | Check motor connections, verify DIP switches  |
| Erratic movement      | PWM channel conflict  | Use different LEDC channel number             |
| Direction reversed    | Wiring swapped        | Swap DIR1/DIR2 connections on MDDS30          |
| Noisy operation       | PWM frequency too low | Increase MOTOR_PWM_FREQUENCY in configuration |

## Safety Considerations

- Always verify motor direction before full-speed operation
- Initialize the system and check `isInitialized()` return value
- Use proper current-limiting fuses in the power circuit
- Ensure adequate cooling for continuous high-duty-cycle operation
