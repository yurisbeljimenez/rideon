# Project: ESP32-S3 Advanced Ride-On Car Controller

_Last Updated: August 21, 2025_

## 1. Project Overview

This document outlines the development of a complete, safe, and feature-rich software controller for a child's ride-on car, architected around an `ESP32-S3` microcontroller. The system's primary design goal is safety, achieved through a multi-layered control hierarchy that prioritizes collision avoidance and parental override above all else. Concurrently, the project aims to deliver an enhanced user experience with advanced, intuitive driving features like a custom acceleration curve and single-pedal operation.

---

## 2. Hardware Manifest

The following table details the core electronic components and their specific roles within the system.

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

---

## 3. Software Architecture

The software is engineered using modern design patterns to ensure it is robust, maintainable, and highly responsive.

- **Modular, Class-Based Design:** The architecture is fully object-oriented. Each hardware component (e.g., `Accelerator`, `DriveController`) is abstracted into its own self-contained class, promoting clean code, encapsulation, and ease of management.

- **Non-Blocking State Machine:** The main application logic in `main.cpp` operates as a robust state machine, managing the car's operational states (e.g., `STOPPED`, `DRIVING_FORWARD`, `AVOIDING_OBSTACLE`). This ensures predictable behavior and clean transitions between modes.

- **Centralized Definitions:** Core data structures and enumerations, such as `CarState`, are located in a shared directory (`src/Shared/`). This practice prevents code duplication and ensures system-wide consistency.

- **Dependency Injection:** System-wide services, like the `Logger`, are "injected" into the constructors of modules that require them. This design decouples modules from concrete implementations, making the architecture more flexible and testable.

- **Fully Non-Blocking Execution:** To guarantee maximum responsiveness, the entire codebase is non-blocking. The main `loop()` runs at maximum frequency, utilizing `millis()` for timing logic and hardware interrupts for time-sensitive tasks. This ensures the system can react to hazards and inputs instantaneously.

---

## 4. Key Features

The current software build is feature-complete and ready for physical integration and tuning.

- **Advanced Safety System**

  - **3-Level Control Hierarchy:** A strict priority order is enforced at all times:
    1.  Collision Avoidance System
    2.  Parental RC Override
    3.  Driver Input
  - **Dynamic Safety Distance:** The collision avoidance system intelligently increases its minimum safe distance buffer as the car's speed increases.
  - **Soft Stop:** Obstacle detection triggers a smooth, controlled deceleration rather than a jarring, instantaneous halt, improving comfort and safety.

- **Enhanced Driving Experience**

  - **3-Stage Acceleration Curve:** Provides a gentle start from a standstill, a responsive mid-range for maneuvering, and a smooth roll-off to top speed.
  - **Single-Pedal Driving:** The car features active braking, where releasing the accelerator pedal brings the vehicle to a quick and predictable stop, creating an intuitive control scheme.

- **System Diagnostics**
  - **Status Indicator:** The onboard RGB LED provides at-a-glance feedback on the car's current state (e.g., Blue for Stopped, Green for Driving, Pulsing Red for Obstacle Detected).
  - **Comprehensive Hardware Abstraction:** The software includes complete, documented modules for every component listed in the hardware manifest.

---

## 5. Next Steps

The project now moves from software development to the physical integration and testing phase.

1.  **Physical Assembly & Integration:**

    - Mount all electronic components securely onto the vehicle chassis.
    - Construct and route the wiring harness, ensuring proper gauge wiring for high-current paths.
    - Perform initial power-on "smoke tests" to verify power distribution and component integrity.

2.  **Calibration & Tuning:**

    - Systematically tune the software constants defined in `main.cpp`.
    - Calibrate pedal input range (min/max ADC values).
    - Adjust motor PWM limits to set the desired top speed.
    - Fine-tune the acceleration curve parameters for the desired driving feel.
    - Set the base safety distance and speed-scaling factor for the collision avoidance system.

3.  **Field Testing:**
    - Conduct real-world testing in a safe, controlled environment.
    - Verify the functionality of the control hierarchy, especially the parental RC override.
    - Test the collision avoidance system at various speeds and with different types of obstacles.
    - Evaluate the overall driving experience and make final tuning adjustments.
