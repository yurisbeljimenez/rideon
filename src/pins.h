#pragma once

//================================================================================
// --- PIN DEFINITIONS ---
// A central place to define all hardware connections to the ESP32.
// This file is included by main.cpp and other modules that need pin access.
//================================================================================

// Inputs - User Interface
#define PEDAL_PIN 4              // Analog pin for the accelerator pedal (ADC1_CH0)
#define SHIFTER_PIN 16           // Digital pin for the FWD/REV gear shifter button
#define REMOTE_THROTTLE_PIN 15   // Digital pin for RC receiver throttle (CH2 - servo signal)
#define REMOTE_STEERING_PIN 14   // Digital pin for RC receiver steering (CH1 - servo signal)

// Proximity Sensors - HC-SR04 Ultrasonic
#define FRONT_TRIG_PIN 12        // Digital pin for Front Sensor Trig
#define FRONT_ECHO_PIN 13        // Digital pin for Front Sensor Echo
#define BACK_TRIG_PIN 35         // Digital pin for Back Sensor Trig (ADC2 - avoid WiFi conflicts)
#define BACK_ECHO_PIN 36         // Digital pin for Back Sensor Echo (ADC2 - avoid WiFi conflicts)

// Outputs - Motor Control
#define MOTOR_DIR_PIN 1          // Digital pin to Cytron MDDS30 DIR1/DIR2
#define MOTOR_PWM_PIN 2          // Digital pin to Cytron MDDS30 PWM1/PWM2 (LEDC channel 0)

// Steering Control - ESC Signal
#define STEERING_SERVO_PIN 17    // Digital pin to Radiolink CL9030 Steering ESC Signal

// Status Indicators
#define RGB_PIN 15               // Onboard RGB LED light (moved from pin 48 for better signal integrity)

//================================================================================
// --- TUNING CONSTANTS ---
// A central place for all performance tuning parameters.
//================================================================================

// Acceleration Curve - Controls how the car accelerates based on pedal input
const long ACCEL_INTERVAL_LOW = 30;   // Gentle start (0-25% speed): ~150ms per step
const long ACCEL_INTERVAL_MID = 10;   // Responsive mid-range (26-80% speed): ~50ms per step  
const long ACCEL_INTERVAL_HIGH = 25;  // Ease to top speed (81-100% speed): ~125ms per step
const int ACCEL_LOGGING_THRESHOLD = 5; // Log if accelerator output is > 5

// Active Braking - Controls deceleration when pedal is released (single-pedal driving)
const long BRAKING_INTERVAL = 5;      // Fast deceleration: ~25ms per step for responsive braking

// RC Receiver Calibration - Dead zones to ignore small stick movements
const int REMOTE_THROTTLE_NEUTRAL_MIN = 1490; // Start of Throttle dead zone (µs)
const int REMOTE_THROTTLE_NEUTRAL_MAX = 1530; // End of Throttle dead zone (µs)
const int REMOTE_STEERING_NEUTRAL_MIN = 1480; // Start of Steering dead zone (µs)
const int REMOTE_STEERING_NEUTRAL_MAX = 1520; // End of Steering dead zone (µs)

// Collision Avoidance - Safety distance settings
const int MIN_SAFETY_DISTANCE_CM = 20;  // Minimum safe distance at 0 speed
const int MAX_SAFETY_DISTANCE_CM = 80;  // Maximum safe distance at max speed (linear interpolation used)
const int SENSOR_LOGGING_THRESHOLD = MAX_SAFETY_DISTANCE_CM + 20; // Log if object is within 100cm
const int SENSOR_SMOOTHING_WINDOW = 5;  // Average the last 5 readings for stability

// Drive Motor - PWM configuration
const int MOTOR_PWM_CHANNEL = 0;        // ESP32 LEDC PWM channel (0-15) for the drive motor
const int MOTOR_PWM_FREQUENCY = 5000;   // PWM frequency in Hz (5kHz for quiet operation)
const int MOTOR_PWM_RESOLUTION = 8;     // PWM resolution in bits (0-255 duty cycle)

//================================================================================
// --- I2S PIN DEFINITIONS (UNUSED - RESERVED FOR FUTURE AUDIO FEATURES) ---
// These pins are defined but not currently used. Reserved for future audio features
// such as horn sounds, engine noise simulation, or voice announcements.
//================================================================================

#define I2S_BCLK_PIN 5   // I2S Bit Clock pin
#define I2S_LRC_PIN 3    // I2S Left/Right Clock (Word Select) pin
#define I2S_DIN_PIN 6    // I2S Data In pin for stereo audio input

// Note: ESP32-S3 I2S peripherals support various configurations. 
// These pins can be reconfigured based on specific audio requirements.