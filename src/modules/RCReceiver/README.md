# RCReceiver Module

The RCReceiver module reads and interprets signals from a 2-channel RC receiver, providing parental override control for the ride-on car.

## Overview

This module reads pulse-width modulation (PWM) signals from an RC receiver using `pulseIn()`. It provides calibrated throttle and steering values (-100 to 100), along with status indicators for active control. The module includes configurable dead zones to ignore small stick movements.

## Key Features

- **Dead Zone Calibration**: Configurable neutral zones for both throttle and steering channels
- **Intelligent Logging**: Only logs when controls are actively being used
- **Status Indicators**: Methods to check if throttle or steering inputs are active
- **Timeout Handling**: Returns neutral (1500µs) on timeout conditions

## Hardware Requirements

### RC Receiver Wiring

| RC Receiver Pin | ESP32-S3 Connection          | Function                              |
| --------------- | ---------------------------- | ------------------------------------- |
| Signal          | THROTTLE_PIN or STEERING_PIN | PWM signal output                     |
| VCC             | 5V                           | Power (check receiver voltage rating) |
| GND             | GND                          | Ground                                |

### Recommended Receivers

The system is tested with:

- **Radiolink R6FG** - 6-channel receiver

### Control Channels

| Channel | Function                   | Default Pin | Pulse Range     |
| ------- | -------------------------- | ----------- | --------------- |
| CH1     | Steering (Left/Right)      | GPIO 14     | 1000µs - 2000µs |
| CH2     | Throttle (Forward/Reverse) | GPIO 15     | 1000µs - 2000µs |

## Usage Example

```cpp
#include "RCReceiver.h"
#include "Logger.h"

Logger remoteLogger("Remote Control");
RCReceiver remoteControl(REMOTE_THROTTLE_PIN, REMOTE_STEERING_PIN,
                         &remoteLogger,
                         1490, 1530,  // Throttle dead zone (µs)
                         1480, 1520); // Steering dead zone (µs)

void setup() {
    remoteControl.setup();
}

void loop() {
    remoteControl.update();

    if (remoteControl.isOverriding()) {
        int throttle = remoteControl.getThrottle();
        int steering = remoteControl.getSteering();

        // Parental control active
    }
}
```

## API Reference

### Constructor

```cpp
RCReceiver(int throttlePin, int steeringPin, Logger* logger,
           int throttleNeutralMin, int throttleNeutralMax,
           int steeringNeutralMin, int steeringNeutralMax);
```

### Methods

| Method               | Description                                  | Returns           |
| -------------------- | -------------------------------------------- | ----------------- |
| `setup()`            | Initialize the receiver pins                 | void              |
| `update()`           | Read current pulse widths from both channels | void              |
| `getThrottle()`      | Get calibrated throttle value                | int (-100 to 100) |
| `getSteering()`      | Get calibrated steering value                | int (-100 to 100) |
| `isOverriding()`     | Check if throttle is active                  | bool              |
| `isSteeringActive()` | Check if steering is active                  | bool              |

### Calibration Values

The neutral zone (dead zone) defines the range where input is considered "off":

```cpp
// Default values from main.cpp
const int REMOTE_THROTTLE_NEUTRAL_MIN = 1490;
const int REMOTE_THROTTLE_NEUTRAL_MAX = 1530;
const int REMOTE_STEERING_NEUTRAL_MIN = 1480;
const int REMOTE_STEERING_NEUTRAL_MAX = 1520;
```

### Value Mapping

| Raw Pulse (µs) | Mapped Value | Description                         |
| -------------- | ------------ | ----------------------------------- |
| < 1000         | -100         | Full reverse/left                   |
| 1000-1490      | -100 to 0    | Partial reverse/left toward neutral |
| 1490-1530      | 0            | Neutral (dead zone)                 |
| 1530-2000      | 0 to 100     | Partial forward/right from neutral  |
| > 2000         | 100          | Full forward/right                  |

## Implementation Details

### Pulse Measurement

The module uses Arduino's `pulseIn()` function:

```cpp
_rawThrottle = pulseIn(_throttlePin, HIGH, 25000);
_rawSteering = pulseIn(_steeringPin, HIGH, 25000);
```

- **Timeout**: 25ms (25000µs) - returns 0 if no pulse detected within timeout
- **On Timeout**: Value defaults to 1500µs (neutral position)

### Intelligent Logging

The module only logs when either control is actively being used:

```cpp
if (_logger && (isOverriding() || isSteeringActive())) {
    _logger->log(_rawThrottle * 10000 + _rawSteering);
}
```

This prevents log spam during idle periods while still providing debug information when parental control is active.

## Calibration Procedure

To calibrate the RC receiver dead zones:

1. Connect the receiver and power it up
2. Open Serial Monitor at 115200 baud
3. Observe raw pulse values from `RCReceiver::update()` logging
4. Adjust neutral zone values to match the actual "off" position range
5. Test that small stick movements don't trigger override

## Troubleshooting

| Issue           | Possible Cause               | Solution                                         |
| --------------- | ---------------------------- | ------------------------------------------------ |
| No response     | Receiver not powered         | Check VCC and GND connections                    |
| Erratic values  | Loose wiring or interference | Secure all connections, add decoupling capacitor |
| Always neutral  | Dead zone too wide           | Narrow the neutral range values                  |
| Values inverted | Channel mapping wrong        | Swap throttle/steering pins if needed            |

## Safety Considerations

- Always verify receiver calibration before operation
- Use proper dead zones to prevent drift-induced movement
- Implement timeout detection for lost signal scenarios
- Consider adding a physical kill switch as backup safety measure
