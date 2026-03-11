# ESP32-S3 Advanced Ride-On Car Controller

## Project Overview

This is a complete, safety-focused software controller for a child's ride-on car, built around an ESP32-S3 microcontroller. The system prioritizes collision avoidance and parental override above all else while providing an enhanced driving experience.

## Key Features

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

## Hardware Components

| Category             | Component & Model                                       | Role & Key Specifications                                                                                                                                                                   |
| -------------------- | ------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Microcontroller**  | `ESP32-S3` Development Board (N16R8)                    | The central processing unit ("brain") of the vehicle. Manages all I/O, runs the main control loop, and executes all safety and driving logic.                                               |
| **Power System**     | DeWalt 20V Battery + 20V to **12V** Step-Down Converter | Provides a stable, high-current **12V** source for the drive system and main board from a standard, rechargeable tool battery.                                                              |
| **Main Board**       | Freenove Breakout Board for `ESP32-S3`                  | The central "motherboard" for power distribution and signal routing. It accepts the **12V** input and provides regulated **5V** and **3.3V** power rails for sensors, receivers, and logic. |
| **Drive Control**    | Cytron MDDS30 (30A) Dual Motor Driver                   | A high-power, dual-channel H-bridge driver. Receives PWM and direction signals from the `ESP32` to precisely control the speed and direction of the two rear drive motors.                  |
| **Steering Control** | Radiolink CL9030 (90A) Brushed ESC                      | A dedicated Electronic Speed Controller that functions as a high-current servo. It interprets a standard servo PWM signal from the `ESP32` to manage the front steering motor.              |
| **Safety Sensing**   | `HC-SR04` Ultrasonic Module (x2)                        | The forward and reverse-facing "eyes" of the car. These sensors provide real-time distance measurements to the ground, forming the basis of the collision avoidance system.                 |
| **Human Interfaces** | Radiolink R6FG 6-Channel Receiver                       | The parental remote control interface. Receives signals from a handheld transmitter, allowing for an immediate safety override of both throttle and steering inputs.                        |
|                      | Analog Accelerator Pedal (Potentiometer/Hall Effect)    | The primary driver input. Provides a variable analog signal to the `ESP32`, which is then mapped to the desired vehicle speed.                                                              |
|                      | Momentary Pushbutton                                    | Functions as the "gear selector," allowing the driver to toggle the car's state between `Forward` and `Reverse`.                                                                            |

## Software Architecture

The software is engineered using modern design patterns to ensure it is robust, maintainable, and highly responsive.

- **Modular, Class-Based Design**: Each hardware component is abstracted into its own self-contained class, promoting clean code, encapsulation, and ease of management.
- **Non-Blocking State Machine**: The main application logic operates as a robust state machine, managing the car's operational states.
- **Centralized Definitions**: Core data structures and enumerations are located in a shared directory to prevent code duplication.
- **Dependency Injection**: System-wide services like the `Logger` are injected into modules that require them, making the architecture more flexible and testable.
- **Fully Non-Blocking Execution**: The entire codebase is non-blocking, ensuring maximum responsiveness to hazards and inputs.

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
│       └── CarState.h              # Car state enumeration
├── test/                          # Unit tests (if any)
├── include/                       # Additional header files
├── platformio.ini                 # Build configuration
├── CONTRIBUTING.md                # Contribution guidelines
├── CODE_OF_CONDUCT.md             # Code of conduct
└── PROJECT_README.md              # This file
```

## Hardware Integration Guide

**Before building the physical system, please review the comprehensive hardware guide:**

- **[HARDWARE_INTEGRATION_GUIDE.md](../HARDWARE_INTEGRATION_GUIDE.md)** - Complete wiring diagrams, component lists, pin connections, power requirements, assembly instructions, and testing procedures

For detailed documentation of each module, see the README files in their respective directories.

## Development Setup

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
