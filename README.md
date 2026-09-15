# ESP32-S3 Advanced Ride-On Car Controller

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A complete, safety-focused software controller for a child's ride-on car built around an ESP32-S3 microcontroller. This system prioritizes collision avoidance and parental override while providing an enhanced driving experience.

## Features

### Safety System

- **3-Level Control Hierarchy**: Collision avoidance > Parental RC override > Driver input
- **Dynamic Safety Distance**: Adjusts minimum safe distance based on vehicle speed
- **Soft Stop**: Smooth deceleration instead of abrupt stops

### Driving Experience

- **3-Stage Acceleration Curve**: Gentle start, responsive mid-range, smooth top speed
- **Single-Pedal Driving**: Active braking when pedal is released
- **Visual Status Indicators**: Onboard RGB LED shows car state

### System Diagnostics

- **Comprehensive Logging**: Per-module debugging output
- **Hardware Abstraction**: Clean separation of hardware and software logic

## Design Philosophy

- **Modular, Class-Based Design**: Each hardware component is a self-contained class with clean encapsulation.
- **Non-Blocking State Machine**: Main loop is a robust state machine managing car operational states.
- **Centralized Definitions**: Core data structures in a shared directory to prevent duplication.
- **Dependency Injection**: System-wide services (e.g., `Logger`) are injected into modules for testability.
- **Fully Non-Blocking Execution**: Maximum responsiveness to hazards and inputs.

## Hardware Components

| Category | Component | Role |
|----------|-----------|------|
| **Microcontroller** | ESP32-S3 Dev Board (N16R8) | Central processing unit. Manages all I/O, control loop, and safety logic. |
| **Power System** | DeWalt 20V Battery + 20V→12V Step-Down | Stable 12V source for drive system and main board. |
| **Main Board** | Freenove Breakout Board for ESP32-S3 | Power distribution and signal routing. Provides 5V/3.3V rails. |
| **Drive Control** | Cytron MDDS30 (30A) Dual Motor Driver | H-bridge driver for two rear drive motors (PWM + DIR). |
| **Steering Control** | Radiolink CL9030 (90A) Brushed ESC | High-current servo replacing standard ESC for front steering. |
| **Safety Sensing** | HC-SR04 Ultrasonic (×2) | Forward/reverse obstacle detection. Basis of collision avoidance. |
| **Human Interface** | Radiolink R6FG 6-Channel Receiver | Parental RC override for throttle and steering. |
| | Analog Accelerator Pedal (Potentiometer/Hall Effect) | Primary driver input, mapped to vehicle speed. |
| | Momentary Pushbutton | Gear shifting (Forward/Neutral/Reverse). |
| **Status** | NeoPixel RGB LED (WS2812) | Visual state indicator (safe, warning, danger, fault). |

## Project Structure

```
.
├── src/
│   ├── main.cpp                    # Main application entry point
│   ├── pins.h                      # Pin definitions and tuning constants
│   ├── modules/                    # All system modules
│   │   ├── Accelerator/            # Accelerator pedal controller
│   │   ├── DriveController/        # Motor drive control
│   │   ├── GearShifter/            # Gear shifting logic
│   │   ├── Logger/                 # Debug logging system
│   │   ├── ProximitySensor/        # Ultrasonic obstacle detection
│   │   ├── RCReceiver/             # Remote control receiver
│   │   ├── SteeringController/     # Steering control system
│   │   └── SystemStatus/           # LED status indicators
│   └── Shared/                     # Shared data structures
│       ├── CarState.h              # Car state enumeration
│       └── ControlLogic.h          # Pure-logic control state machine
├── test/                           # Unit tests
├── platformio.ini                  # Build configuration
├── CONTRIBUTING.md                 # Contribution guidelines
├── CODE_OF_CONDUCT.md              # Code of conduct
└── CODE_ANALYSIS_REPORT.md         # Code quality analysis
```

## Getting Started

### Prerequisites

- PlatformIO IDE or Arduino IDE with ESP32 support
- ESP32-S3 development board
- Required libraries (listed in platformio.ini)

### Building the Project

1. Clone the repository
2. Install required libraries:
   ```bash
   pio lib install "Adafruit NeoPixel"
   pio lib install "ESP32Servo"
   ```
3. Build the project:
   ```bash
   pio run
   ```

### Flashing to Hardware

Connect your ESP32-S3 board and upload using:

```bash
pio run -t upload
```

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct and the process for submitting pull requests.

## Code of Conduct

Please read [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) for details on our code of conduct.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Special thanks to the ESP32 and Arduino communities
- Inspiration from various robotics and embedded systems projects

## Documentation

For detailed documentation of each module, please see:

- [SystemStatus Module](src/modules/SystemStatus/README.md)
- [Logger Module](src/modules/Logger/README.md)
- [ProximitySensor Module](src/modules/ProximitySensor/README.md)
- [SteeringController Module](src/modules/SteeringController/README.md)
- [Hardware Integration Guide](HARDWARE_INTEGRATION_GUIDE.md)
- [Code Analysis Report](CODE_ANALYSIS_REPORT.md)
