#pragma once

//================================================================================
// pins.h — CENTRAL HARDWARE DEFINITIONS (SINGLE SOURCE OF TRUTH)
//
// Every GPIO assignment and hardware-facing tuning constant for the ride-on
// car lives in this file. main.cpp (and any module that needs a raw pin)
// includes this header — do NOT re-declare pins or tuning constants elsewhere.
//
// Target board: ESP32-S3 (N16R8 module on a Freenove breakout board).
//
// Pin-map constraints honored below:
//   * GPIO 0, 3, 45, 46  — strapping pins (boot mode / flash voltage): avoided.
//   * GPIO 19, 20        — USB D-/D+: avoided.
//   * GPIO 35–38         — tied to octal PSRAM on N16R8 modules. The back
//                           ultrasonic already uses 35/36 on this board and
//                           works, so they are kept, but do not re-assign
//                           them without verifying your module variant.
//
// CONFLICT RESOLVED (this revision):
//   GPIO15 was assigned to BOTH the RC throttle input and the RGB LED.
//   Resolution: the RC throttle (safety-critical parental override input,
//   already wired and interrupt-tested) KEEPS GPIO15. The NeoPixel status
//   LED moves to GPIO18 (free, plain GPIO).
//   A new E-STOP button is assigned to GPIO9 (free, plain GPIO, internal
//   pull-up available).
//
// E-STOP WIRING CONVENTION (active-low):
//   ESTOP_PIN --[momentary button]-- GND, read with INPUT_PULLUP.
//   Button open  -> pin HIGH  (estop NOT pressed)
//   Button closed-> pin LOW   (estop pressed)
//   Software e-stop is a fail-safe, not a substitute for hardware: for a
//   production car the e-stop switch should also cut motor power at the
//   driver/battery level.
//
// Note: this header is intended for Arduino C++ translation units (it uses
// LOW/HIGH and static_assert).
//================================================================================

// Allow this header to be parsed standalone (before <Arduino.h>) — same values
// Arduino would define, so a later Arduino.h include remains compatible.
#ifndef LOW
#define LOW 0
#define HIGH 1
#endif

//================================================================================
// --- PIN DEFINITIONS ---
//================================================================================

// Inputs - User Interface
#define PEDAL_PIN 4              // Analog pin for the accelerator pedal (ADC1_CH0)
#define SHIFTER_PIN 16           // Digital pin for the FWD/REV gear shifter button
#define ESTOP_PIN 9              // Digital pin for the E-STOP button (active-low, INPUT_PULLUP)

// Inputs - Parental RC Receiver (Radiolink R6FG)
#define REMOTE_THROTTLE_PIN 15   // CH2 servo PWM — KEEPS GPIO15 (was conflicted with RGB)
#define REMOTE_STEERING_PIN 14   // CH1 servo PWM

// Proximity Sensors - HC-SR04 Ultrasonic
#define FRONT_TRIG_PIN 12        // Front sensor Trig
#define FRONT_ECHO_PIN 13        // Front sensor Echo
#define BACK_TRIG_PIN 35         // Back sensor Trig (ADC2 — avoids WiFi conflicts)
#define BACK_ECHO_PIN 36         // Back sensor Echo (ADC2 — avoids WiFi conflicts)

// Outputs - Motor Control (Cytron MDDS30)
#define MOTOR_DIR_PIN 1          // DIR1/DIR2
#define MOTOR_PWM_PIN 2          // PWM1/PWM2 (LEDC channel 0)

// Outputs - Steering (Radiolink CL9030 ESC)
#define STEERING_SERVO_PIN 17    // ESC signal

// Outputs - Status Indicators
#define RGB_PIN 18               // NeoPixel data — moved from 15 to resolve the
                                 // RC-throttle/LED conflict (see header note)

// Reserved - I2S (UNUSED, reserved for future audio: horn, engine sound, voice)
#define I2S_BCLK_PIN 5           // I2S Bit Clock pin
#define I2S_LRC_PIN 3            // I2S Left/Right Clock (Word Select) pin
#define I2S_DIN_PIN 6            // I2S Data In pin for stereo audio input
// Note: ESP32-S3 I2S peripherals support various configurations; these can be
// reconfigured based on specific audio requirements.

// E-Stop electrical convention (see header note)
#define ESTOP_ACTIVE_LEVEL LOW   // pressed = LOW (button pulls pin to GND)
#define ESTOP_DEBOUNCE_MS 30     // min ms a stable level is required to register

//================================================================================
// --- PIN ASSIGNMENT GUARD ---
// Fails the build if any two of the defined signals share a GPIO. This is the
// compile-time net that catches the class of bug GPIO15 was (RC throttle and
// RGB LED both on 15).
//================================================================================

namespace pins {
// C++11-safe uniqueness check (C++11 constexpr bodies must be a single return,
// so no loops/local variables): give every pin a bit position. The bitwise OR
// of all masks equals their arithmetic sum if and only if no two pins share a
// GPIO (a shared pin would add its bit value twice, making sum > OR).
constexpr bool allUnique() {
  return ( (1ULL << PEDAL_PIN)
        | (1ULL << SHIFTER_PIN)
        | (1ULL << ESTOP_PIN)
        | (1ULL << REMOTE_THROTTLE_PIN)
        | (1ULL << REMOTE_STEERING_PIN)
        | (1ULL << FRONT_TRIG_PIN)
        | (1ULL << FRONT_ECHO_PIN)
        | (1ULL << BACK_TRIG_PIN)
        | (1ULL << BACK_ECHO_PIN)
        | (1ULL << MOTOR_DIR_PIN)
        | (1ULL << MOTOR_PWM_PIN)
        | (1ULL << STEERING_SERVO_PIN)
        | (1ULL << RGB_PIN)
        | (1ULL << I2S_BCLK_PIN)
        | (1ULL << I2S_LRC_PIN)
        | (1ULL << I2S_DIN_PIN) )
       == ( (1ULL << PEDAL_PIN)
        + (1ULL << SHIFTER_PIN)
        + (1ULL << ESTOP_PIN)
        + (1ULL << REMOTE_THROTTLE_PIN)
        + (1ULL << REMOTE_STEERING_PIN)
        + (1ULL << FRONT_TRIG_PIN)
        + (1ULL << FRONT_ECHO_PIN)
        + (1ULL << BACK_TRIG_PIN)
        + (1ULL << BACK_ECHO_PIN)
        + (1ULL << MOTOR_DIR_PIN)
        + (1ULL << MOTOR_PWM_PIN)
        + (1ULL << STEERING_SERVO_PIN)
        + (1ULL << RGB_PIN)
        + (1ULL << I2S_BCLK_PIN)
        + (1ULL << I2S_LRC_PIN)
        + (1ULL << I2S_DIN_PIN) );
}
} // namespace pins

static_assert(pins::allUnique(),
              "pins.h: two signals are assigned to the same GPIO — fix the assignments above");

//================================================================================
// --- TUNING CONSTANTS ---
// Central place for all performance/safety tuning parameters.
//================================================================================

// Acceleration Curve - Controls how the car accelerates based on pedal input
const long ACCEL_INTERVAL_LOW = 30;   // Gentle start (0-25% speed): ~150ms per step
const long ACCEL_INTERVAL_MID = 10;   // Responsive mid-range (26-80% speed): ~50ms per step
const long ACCEL_INTERVAL_HIGH = 25;  // Ease to top speed (81-100% speed): ~125ms per step
const int ACCEL_LOGGING_THRESHOLD = 5; // Log if pedal intent is > 5

// Active Braking - Deceleration when pedal is released (single-pedal driving)
const long BRAKING_INTERVAL = 5;      // Fast deceleration: ~25ms per step for responsive braking

// --- PEDAL CALIBRATION (NEW) --------------------------------------------------
// Calibrated ADC range of the accelerator pedal. ESP32-S3 analogRead is 12-bit
// (0..4095). The defaults below are the full range (previous behavior).
//
// CALIBRATION PROCEDURE:
//   1. Power the car and watch the Serial Monitor (115200). At boot a
//      "Pedal Calibration Diagnostic" prints raw pedal samples.
//   2. Fully RELEASE the pedal, then fully PRESS and hold it when prompted,
//      and note the lowest and highest raw values printed.
//   3. Set PEDAL_ADC_MIN / PEDAL_ADC_MAX below to those observed values and
//      rebuild. This removes any rest-position offset and unused travel so
//      0% and 100% pedal feel exact.
const int PEDAL_ADC_MIN = 0;          // Pedal fully released (raw ADC, 0..4095)
const int PEDAL_ADC_MAX = 4095;       // Pedal fully pressed  (raw ADC, 0..4095)

// RC Receiver Calibration - Dead zones to ignore small stick movements (µs)
const int REMOTE_THROTTLE_NEUTRAL_MIN = 1490; // Start of Throttle dead zone (µs)
const int REMOTE_THROTTLE_NEUTRAL_MAX = 1530; // End of Throttle dead zone (µs)
const int REMOTE_STEERING_NEUTRAL_MIN = 1480; // Start of Steering dead zone (µs)
const int REMOTE_STEERING_NEUTRAL_MAX = 1520; // End of Steering dead zone (µs)

// --- RC LINK TIMEOUT (NEW) ----------------------------------------------------
// A channel is considered LOST after this many ms without a valid pulse.
// Lost channels report neutral (0) and never trigger an override (fail-safe).
// Standard RC frame period is 20 ms (50 Hz); 60 ms = 3 missed frames, which
// tolerates one glitched frame while still failing safe within ~60 ms.
const unsigned long RC_SIGNAL_TIMEOUT_MS = 60;

// Collision Avoidance - Safety distance settings
const int MIN_SAFETY_DISTANCE_CM = 20;  // Minimum safe distance at 0 speed
const int MAX_SAFETY_DISTANCE_CM = 80;  // Maximum safe distance at max speed (linear interpolation used)
const int SENSOR_LOGGING_THRESHOLD = MAX_SAFETY_DISTANCE_CM + 20; // Log if object is within 100cm
const int SENSOR_SMOOTHING_WINDOW = 5;  // Average the last 5 readings for stability

// Drive Motor - PWM configuration
const int MOTOR_PWM_CHANNEL = 0;        // ESP32 LEDC PWM channel (0-15) for the drive motor
const int MOTOR_PWM_FREQUENCY = 5000;   // PWM frequency in Hz (5kHz for quiet operation)
const int MOTOR_PWM_RESOLUTION = 8;     // PWM resolution in bits (0-255 duty cycle)
