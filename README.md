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

## Hardware Requirements

- ESP32-S3 Development Board
- Cytron MDDS30 Dual Motor Driver
- Radiolink CL9030 Brushed ESC for steering
- HC-SR04 Ultrasonic Sensors (2x)
- Radiolink R6FG 6-Channel Receiver
- Analog Accelerator Pedal
- Momentary Pushbutton for gear shifting
- NeoPixel RGB LED for status indication

## Project Structure

```
.
├── src/
│   ├── main.cpp                    # Main application entry point
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
│       └── CarState.h              # Car state enumeration
├── test/                          # Unit tests (if any)
├── include/                       # Additional header files
├── platformio.ini                 # Build configuration
├── CONTRIBUTING.md                # Contribution guidelines
├── CODE_OF_CONDUCT.md             # Code of conduct
└── PROJECT_README.md              # Detailed project documentation
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
- [Complete Project Documentation](PROJECT_README.md)
