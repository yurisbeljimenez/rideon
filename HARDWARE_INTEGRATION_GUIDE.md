# Hardware Integration Guide - ESP32-S3 Ride-On Car Controller

Complete wiring diagram and components integration guide for building the physical ride-on car controller system.

---

## System Overview

This document provides detailed hardware connection instructions, pin assignments, power requirements, and integration steps to build the complete ESP32-S3 Ride-On Car Controller system.

### System Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                    ESP32-S3 Development Board                        │
│  ┌──────────────────────────────────────────────────────────────┐   │
│  │                                                               │   │
│  │  GPIO Pins:                                                  │   │
│  │    • GPIO 1  → MDDS30 DIR (Direction Control)               │   │
│  │    • GPIO 2  → MDDS30 PWM (Speed Control - LEDC Ch 0)       │   │
│  │    • GPIO 3  → I2S LRC (Reserved for Audio)                 │   │
│  │    • GPIO 4  → Accelerator Pedal (ADC1_CH0 - Analog Input)  │   │
│  │    • GPIO 5  → I2S BCLK (Reserved for Audio)                │   │
│  │    • GPIO 6  → I2S DIN (Reserved for Audio)                 │   │
│  │    • GPIO 12 → Front Sensor TRIG                            │   │
│  │    • GPIO 13 → Front Sensor ECHO                            │   │
│  │    • GPIO 14 → RC Receiver CH1 - Steering                   │   │
│  │    • GPIO 15 → RGB LED / RC Receiver CH2 - Throttle         │   │
│  │    • GPIO 16 → Gear Shifter Button                          │   │
│  │    • GPIO 17 → Steering ESC Signal                          │   │
│  │    • GPIO 35 → Back Sensor TRIG (ADC2)                      │   │
│  │    • GPIO 36 → Back Sensor ECHO (ADC2)                      │   │
│  │                                                               │   │
│  └──────────────────────────────────────────────────────────────┘   │
│                              ▲                                       │
│                              │ Power & Signal                        │
└──────────────────────────────┼───────────────────────────────────────┘
                               │
        ┌──────────────────────┼──────────────────────┐
        ▼                      ▼                      ▼
┌───────────────┐     ┌─────────────────┐    ┌───────────────────┐
│  MDDS30 Motor │     │   HC-SR04       │    │ Radiolink RC      │
│   Driver      │     │  Ultrasonic     │    │ Receiver (R6FG)   │
│               │     │  Sensors        │    │                   │
└───────────────┘     └─────────────────┘    └───────────────────┘
```

---

## Components Required

### Core Electronics

| #   | Component                   | Quantity | Specifications            | Notes                           |
| --- | --------------------------- | -------- | ------------------------- | ------------------------------- |
| 1   | ESP32-S3 Development Board  | 1        | N16R8 variant recommended | Main controller                 |
| 2   | Cytron MDDS30 Motor Driver  | 1        | 30A dual channel          | Drive motors                    |
| 3   | HC-SR04 Ultrasonic Sensor   | 2        | 2cm-4m range              | Front & rear obstacle detection |
| 4   | Radiolink R6FG Receiver     | 1        | 6-channel PWM             | Parental remote control         |
| 5   | Radiolink CL9030 ESC        | 1        | 90A brushed               | Steering motor controller       |
| 6   | Adafruit NeoPixel (RGB LED) | 1        | GRB type, 5V or 3.3V      | Status indicator                |

### User Interface

| #   | Component                          | Quantity | Specifications      | Notes             |
| --- | ---------------------------------- | -------- | ------------------- | ----------------- |
| 7   | Potentiometer / Hall Effect Sensor | 1        | 10KΩ or linear Hall | Accelerator pedal |
| 8   | Momentary Pushbutton               | 1        | SPST, normally open | Gear shifter      |

### Power System

| #   | Component                 | Quantity | Specifications             | Notes                  |
| --- | ------------------------- | -------- | -------------------------- | ---------------------- |
| 9   | DeWalt 20V Battery Pack   | 1        | 20V MAX Li-Ion             | Main power source      |
| 10  | DC-DC Step-Down Converter | 1        | 20V to 12V, ≥5A output     | Motor power regulation |
| 11  | DC-DC Step-Down Converter | 1        | 12V to 3.3V, ≥500mA output | Logic power (ESP32)    |

### Wiring & Misc

| #   | Component                | Quantity | Notes                               |
| --- | ------------------------ | -------- | ----------------------------------- |
| 12  | Jumper Wires             | Assorted | Male-to-female and female-to-female |
| 13  | Heat Shrink Tubing       | Assorted | Wire insulation                     |
| 14  | Wire Strippers & Crimper | Set      | For connector preparation           |
| 15  | Multimeter               | 1        | Testing and troubleshooting         |

---

## Detailed Pin Connections

### ESP32-S3 to Motor Driver (Cytron MDDS30)

| ESP32-S3 Pin           | Function          | MDDS30 Connection  | Wire Color Recommendation |
| ---------------------- | ----------------- | ------------------ | ------------------------- |
| GPIO 1 (MOTOR_DIR_PIN) | Direction Control | DIR1/DIR2 terminal | Blue                      |
| GPIO 2 (MOTOR_PWM_PIN) | PWM Speed Signal  | PWM1/PWM2 terminal | Yellow                    |

**MDDS30 DIP Switch Settings:**

```
SW1: ON   - Enable direction control via external pin
SW2: OFF  - Use external direction pin (not internal logic)
SW3: ON   - Bidirectional operation enabled
SW4: ON   - Mode configuration bit 1
SW5: OFF  - Protection features as needed
SW6: ON   - Final mode configuration
```

### ESP32-S3 to Ultrasonic Sensors

| Sensor | ESP32 Pin                | Function | HC-SR04 Connection | Wire Color |
| ------ | ------------------------ | -------- | ------------------ | ---------- |
| Front  | GPIO 12 (FRONT_TRIG_PIN) | Trigger  | TRIG pin           | Green      |
| Front  | GPIO 13 (FRONT_ECHO_PIN) | Echo     | ECHO pin           | White      |
| Back   | GPIO 35 (BACK_TRIG_PIN)  | Trigger  | TRIG pin           | Orange     |
| Back   | GPIO 36 (BACK_ECHO_PIN)  | Echo     | ECHO pin           | Pink       |

**HC-SR04 Power Connections:**

- VCC → 5V from ESP32 or external 5V regulator
- GND → Common ground with ESP32

### ESP32-S3 to RC Receiver (Radiolink R6FG)

| Channel | ESP32 Pin                     | Function         | Receiver Output | Wire Color |
| ------- | ----------------------------- | ---------------- | --------------- | ---------- |
| CH1     | GPIO 14 (REMOTE_STEERING_PIN) | Steering Control | CH1 output      | Gray       |
| CH2     | GPIO 15 (REMOTE_THROTTLE_PIN) | Throttle Control | CH2 output      | Red        |

**RC Receiver Power:**

- VCC → 5V from ESP32 (ensure current capability ≥100mA)
- GND → Common ground with ESP32
- Signal Output → Connected to respective GPIO pins above

### ESP32-S3 to Steering ESC (Radiolink CL9030)

| ESP32 Pin                    | Function         | ESC Signal Input | Wire Color |
| ---------------------------- | ---------------- | ---------------- | ---------- |
| GPIO 17 (STEERING_SERVO_PIN) | Servo PWM Signal | Signal wire      | Brown      |

**CL9030 Power Connections:**

- VCC → 5V from logic power supply
- GND → Common ground with ESP32
- Motor terminals → Front steering motor

### ESP32-S3 to Accelerator Pedal

| ESP32 Pin          | Function     | Sensor Connection | Wire Color |
| ------------------ | ------------ | ----------------- | ---------- |
| GPIO 4 (PEDAL_PIN) | Analog Input | Signal output     | Purple     |
| 3.3V               | Power        | VCC or positive   | Red        |
| GND                | Ground       | GND or negative   | Black      |

**Potentiometer Wiring:**

- Outer pin 1 → 3.3V
- Outer pin 2 → GND
- Center wiper → GPIO 4 (through optional 10KΩ pull-down resistor)

### ESP32-S3 to Gear Shifter Button

| ESP32 Pin             | Function      | Button Connection | Wire Color |
| --------------------- | ------------- | ----------------- | ---------- |
| GPIO 16 (SHIFTER_PIN) | Digital Input | One terminal      | Lime       |
| GND                   | Ground        | Other terminal    | Black      |

_Internal pull-up resistor enabled in software (INPUT_PULLUP mode)_

### ESP32-S3 to RGB LED (NeoPixel)

| ESP32 Pin         | Function   | NeoPixel Connection | Wire Color |
| ----------------- | ---------- | ------------------- | ---------- |
| GPIO 15 (RGB_PIN) | Data Input | DATA pin            | Cyan       |
| 5V or 3.3V\*      | Power      | VCC pin             | Red        |
| GND               | Ground     | GND pin             | Black      |

_Use appropriate voltage based on NeoPixel type. Add 470Ω resistor between data line and ESP32 for signal protection._

---

## Power System Design

### Main Power Distribution

```
┌─────────────────┐
│  DeWalt 20V     │
│  Battery Pack   │
└────────┬────────┘
         │
         ▼
┌──────────────────────────────────────┐
│       DC-DC Step-Down Converter      │
│        (20V → 12V, ≥5A)              │
├──────────────────────────────────────┤
│  Input:  +12V output                 │
│  Output: To motor driver & ESCs       │
└───────────┬──────────────────────────┘
            │
    ┌───────┴───────┐
    ▼               ▼
┌─────────┐    ┌─────────┐
│ MDDS30  │    │ CL9030  │
│ Motor   │    │ Steering│
│ Driver  │    │ ESC     │
└─────────┘    └─────────┘

Parallel branch:
    ▼
┌──────────────────────────────────────┐
│       DC-DC Step-Down Converter      │
│        (12V → 3.3V, ≥500mA)          │
├──────────────────────────────────────┤
│  Output: To ESP32-S3 & peripherals   │
└──────────────────────────────────────┘
```

### Power Requirements Summary

| Component           | Voltage    | Current (Typical)   | Notes                    |
| ------------------- | ---------- | ------------------- | ------------------------ |
| ESP32-S3            | 3.3V       | 80-150mA            | During operation         |
| MDDS30 Motor Driver | 12V        | Up to 30A total     | Depends on motors/load   |
| CL9030 Steering ESC | 5-12V      | Up to 90A peak      | Transient current spikes |
| HC-SR04 Sensors     | 5V         | ~15mA each          | When actively pinging    |
| RC Receiver         | 4.8-6V     | 50-100mA            | Depends on receiver type |
| NeoPixel LED        | 5V or 3.3V | ~60mA per color max | At full white brightness |

**Important:** Ensure your DC-DC converters are rated for peak current demands, especially the motor power converter which must handle both drive motors simultaneously.

---

## Assembly Instructions

### Step 1: Prepare the ESP32-S3 Board

1. Install heat sink on ESP32 chip if operating in hot environments
2. Verify all GPIO pins are accessible (not blocked by other components)
3. Connect USB-C cable for initial programming and debugging

### Step 2: Wire the Motor Driver

1. Connect MDDS30 DIR pin to ESP32 GPIO 1
2. Connect MDDS30 PWM pin to ESP32 GPIO 2
3. Verify DIP switch settings match configuration above
4. Connect motor wires from MDDS30 outputs to drive motors
5. Connect 12V power from DC-DC converter to MDDS30 V+ terminals

### Step 3: Install Ultrasonic Sensors

1. Mount front HC-SR04 sensor facing forward at bumper level
2. Mount rear HC-SR04 sensor facing backward at same height
3. Wire TRIG and ECHO pins as specified in pin table above
4. Connect VCC and GND to 5V power source with common ground

### Step 4: Install RC Receiver

1. Securely mount R6FG receiver away from motor noise sources
2. Connect antenna for optimal remote range (≥30 feet recommended)
3. Wire CH1 output to ESP32 GPIO 14 (steering)
4. Wire CH2 output to ESP32 GPIO 15 (throttle)
5. Provide 5V power and common ground

### Step 5: Connect Steering ESC

1. Mount CL9030 ESC in protected location away from heat/moisture
2. Connect signal wire to ESP32 GPIO 17
3. Connect steering motor to ESC output terminals
4. Power ESC with appropriate voltage (check ESC specifications)

### Step 6: Install User Interface Components

1. Mount accelerator pedal potentiometer/Hall sensor in footwell area
2. Wire signal output to ESP32 GPIO 4
3. Mount gear shifter button in accessible location
4. Connect button between GPIO 16 and GND

### Step 7: Install Status LED

1. Mount NeoPixel RGB LED in visible location on vehicle body
2. Connect data pin to ESP32 GPIO 15 (may need time-division multiplexing with throttle)
3. Add 470Ω current-limiting resistor between data and ESP32
4. Provide appropriate power voltage

### Step 8: Power System Integration

1. Install DC-DC converters in protected, ventilated location
2. Connect battery to converter inputs with proper fusing (≥15A recommended)
3. Verify output voltages before connecting loads
4. Establish common ground point for all components

---

## Testing Procedure

### Phase 1: Pre-Power Checklist

Before applying power, verify:

- [ ] All connections match pin diagram
- [ ] No short circuits on any power lines
- [ ] Motor driver DIP switches correctly set
- [ ] Common ground established across all systems
- [ ] Battery polarity correct before connection

### Phase 2: ESP32 Programming

1. Connect ESP32-S3 to computer via USB-C
2. Open PlatformIO IDE or Arduino IDE
3. Verify board selection: "ESP32-S3 Dev Module"
4. Install required libraries:
   - Adafruit NeoPixel
   - ESP32Servo by madhephaestus
5. Build and upload firmware

### Phase 3: Individual Component Testing

**Test Sequence:**

1. **Serial Monitor Debug Output**

   ```bash
   pio device monitor --port /dev/ttyUSB0
   # Expected: Initialization messages from all modules
   ```

2. **LED Status Indicator**
   - Verify RGB LED shows solid blue (STOPPED state) on boot
   - Test color changes during different states

3. **Accelerator Pedal Reading**
   - Open Serial Monitor and observe analog readings
   - Vary pedal position; values should change smoothly from ~0 to 4095

4. **Gear Shifter Button**
   - Press button; expect gear toggle between FORWARD/REVERSE
   - Verify logging output shows state changes

5. **Ultrasonic Sensors**
   - Place hand in front of sensors at varying distances
   - Observe distance readings on Serial Monitor
   - Values should range from ~2cm to 400cm+

6. **RC Receiver**
   - Connect remote transmitter and power on
   - Move steering/throttle sticks; observe raw pulse values
   - Verify dead zones are properly calibrated

7. **Motor Control (No Load)**
   - With motors disconnected from wheels, apply low speed command
   - Verify direction changes with gear shifter
   - Test forward/reverse operation

### Phase 4: Full System Integration Testing

1. Start vehicle in STOPPED state with green LED
2. Press accelerator → transition to FORWARD/REVERSE based on gear
3. Approach obstacle → verify automatic stop at safe distance
4. Activate parental remote → manual override should engage
5. Release throttle → active braking engaged (rapid deceleration)

---

## Troubleshooting Guide

### Common Issues and Solutions

| Symptom                              | Possible Cause                         | Solution                                              |
| ------------------------------------ | -------------------------------------- | ----------------------------------------------------- |
| ESP32 won't boot                     | Insufficient power from 3.3V regulator | Verify converter output, check current rating         |
| Motors don't respond                 | Motor driver initialization failed     | Check DIP switches, verify PWM channel not in use     |
| Inconsistent sensor readings         | Surface reflectivity issues            | Adjust mounting angle, consider different sensor type |
| RC receiver erratic behavior         | Signal interference from motors        | Add ferrite beads, increase separation distance       |
| LED flickering or wrong color        | Wrong NeoPixel type configured         | Verify NEO_GRB vs NEO_RGB in code                     |
| Gear shifter triggers multiple times | Debounce delay too short               | Increase `_debounceDelay` value in module             |
| Vehicle moves unexpectedly           | RC dead zones too narrow               | Expand neutral range values in configuration          |

### Diagnostic Commands

```cpp
// In Serial Monitor, you can observe:
[Accelerator] Value changed: 50
[Front Sensor] Value changed: 45
[Drive Controller] Value changed: 127
[System Status] State: FORWARD
```

---

## Safety Guidelines

### Before Operation

- [ ] Verify all connections are secure and insulated
- [ ] Test emergency stop procedure (power disconnect)
- [ ] Confirm parental remote has adequate range
- [ ] Set safety distance thresholds appropriate for operator skill level
- [ ] Conduct test runs in open area away from hazards

### During Operation

- Always maintain visual contact with vehicle during operation
- Keep hands and body clear of moving parts during testing
- Never bypass collision avoidance system during normal operation
- Monitor battery voltage to prevent unexpected power loss

### Maintenance

- Regularly inspect all wire connections for wear or looseness
- Check motor driver heat dissipation under load
- Verify sensor mounting hasn't shifted over time
- Calibrate RC receiver dead zones periodically as components age

---

## Appendix: Complete Wire Color Reference

| Signal       | ESP32 Pin | Target Component | Recommended Color |
| ------------ | --------- | ---------------- | ----------------- |
| Motor DIR    | GPIO 1    | MDDS30 DIR       | Blue              |
| Motor PWM    | GPIO 2    | MDDS30 PWM       | Yellow            |
| Front TRIG   | GPIO 12   | HC-SR04 Front    | Green             |
| Front ECHO   | GPIO 13   | HC-SR04 Front    | White             |
| Back TRIG    | GPIO 35   | HC-SR04 Back     | Orange            |
| Back ECHO    | GPIO 36   | HC-SR04 Back     | Pink              |
| Steering RC  | GPIO 14   | R6FG CH1         | Gray              |
| Throttle RC  | GPIO 15   | R6FG CH2         | Red               |
| Gear Button  | GPIO 16   | Pushbutton       | Lime              |
| Steering ESC | GPIO 17   | CL9030 Signal    | Brown             |
| Accelerator  | GPIO 4    | Potentiometer    | Purple            |
| RGB LED Data | GPIO 15   | NeoPixel DATA    | Cyan              |

---

## References

- [ESP32-S3 Datasheet](https://espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- [Cytron MDDS30 Manual](<https://cytron.io/assets/product/uploaded/MDDS30%20Manual%20(1).pdf>)
- [HC-SR04 Ultrasonic Sensor Datasheet](http://www.kynix.com/PDF/HC-SR04-Datasheet.pdf)
- [Radiolink R6FG Receiver Manual](https://www.radiolinksport.com/products/r6fg-rc-receiver/)
- [ESP32 Arduino Documentation](https://arduino.esp8266.com/esp32/)

---

_Document Version: 1.0_
_Last Updated: March 2026_
