#include <Arduino.h>
#include "./modules/Accelerator/Accelerator.h"
#include "./modules/ProximitySensor/ProximitySensor.h"
#include "./modules/Logger/Logger.h"
#include "./modules/RCReceiver/RCReceiver.h"
#include "./modules/GearShifter/GearShifter.h"
#include "./modules/DriveController/DriveController.h"
#include "./modules/SteeringController/SteeringController.h"
#include "./modules/SystemStatus/SystemStatus.h"
#include "./Shared/CarState.h"

//================================================================================
// --- PIN DEFINITIONS ---
// A central place to define all hardware connections to the ESP32.
//================================================================================
// Inputs
#define RGB_PIN 15              // Onboard RGB LED light (moved from pin 48 for better signal integrity).
#define PEDAL_PIN 4             // Analog pin for the accelerator pedal
#define SHIFTER_PIN 16          // Digital pin for the FWD/REV gear shifter button
#define REMOTE_THROTTLE_PIN 15  // Digital pin for RC receiver throttle (CH2)
#define REMOTE_STEERING_PIN 14  // Digital pin for RC receiver steering (CH1)
// Proximity Sensors
#define FRONT_TRIG_PIN 12       // Digital pin for Front Sensor Trig
#define FRONT_ECHO_PIN 13       // Digital pin for Front Sensor Echo
#define BACK_TRIG_PIN 35        // Digital pin for Back Sensor Trig
#define BACK_ECHO_PIN 36        // Digital pin for Back Sensor Echo
// Outputs
#define MOTOR_DIR_PIN 1         // Digital pin to Cytron MDDS30 DIR1/DIR2
#define MOTOR_PWM_PIN 2         // Digital pin to Cytron MDDS30 PWM1/PWM2
#define STEERING_SERVO_PIN 17   // Digital pin to Radiolink CL9030 Steering ESC Signal
// I2S Sound
#define I2S_BCLK_PIN 5          // I2S Bit Clock pin
#define I2S_LRC_PIN 3           // I2S Left/Right Clock pin
#define I2S_DIN_PIN 6           // I2S Data In pin

//================================================================================
// --- TUNING CONSTANTS ---
// A central place for all performance tuning.
//================================================================================
// Acceleration Curve
const long ACCEL_INTERVAL_LOW = 30;   // 30ms: Very gentle start from 0-25% speed
const long ACCEL_INTERVAL_MID = 10;   // 10ms: Responsive acceleration from 26-80% speed
const long ACCEL_INTERVAL_HIGH = 25;  // 25ms: Eases into the final top speed (81-100%)
const int ACCEL_LOGGING_THRESHOLD = 5; // Log if accelerator output is > 5

// Active Braking
const long BRAKING_INTERVAL = 5;      // 5ms: Very fast deceleration for single-pedal feel

// RC Receiver Calibration
const int REMOTE_THROTTLE_NEUTRAL_MIN = 1490; // Start of Throttle dead zone
const int REMOTE_THROTTLE_NEUTRAL_MAX = 1530; // End of Throttle dead zone
const int REMOTE_STEERING_NEUTRAL_MIN = 1480; // Start of Steering dead zone
const int REMOTE_STEERING_NEUTRAL_MAX = 1520; // End of Steering dead zone

// Collision avoidance
const int MIN_SAFETY_DISTANCE_CM = 20;  // Safety distance at 0 speed
const int MAX_SAFETY_DISTANCE_CM = 80;  // Safety distance at max speed
const int SENSOR_LOGGING_THRESHOLD = MAX_SAFETY_DISTANCE_CM + 20; // Log if object is within 100cm
const int SENSOR_SMOOTHING_WINDOW = 5; // Average the last 5 readings

// Drive Motor
const int MOTOR_PWM_CHANNEL = 0;        // ESP32 LEDC PWM channel (0-15) for the drive motor

//================================================================================
// --- GLOBAL OBJECTS ---
// Here we create the "virtual" versions of all our hardware components.
//================================================================================
// Loggers (one for each component/system for clean, stateful debugging)
Logger systemStatusLogger("System Status");
Logger accelLogger("Accelerator");
Logger frontSensorLogger("Front Sensor");
Logger backSensorLogger("Back Sensor");
Logger stateLogger("Car State");
Logger remoteLogger("Remote Control");
Logger shifterLogger("Gear Shifter");
Logger driveLogger("Drive Controller");
Logger steeringLogger("Steering");

// Components (instantiate all our hardware modules with their pins and loggers)
Accelerator accelerator(PEDAL_PIN, &accelLogger, ACCEL_INTERVAL_LOW, ACCEL_INTERVAL_MID, ACCEL_INTERVAL_HIGH, BRAKING_INTERVAL, ACCEL_LOGGING_THRESHOLD);
ProximitySensor frontSensor(FRONT_TRIG_PIN, FRONT_ECHO_PIN, &frontSensorLogger, SENSOR_LOGGING_THRESHOLD, SENSOR_SMOOTHING_WINDOW);
ProximitySensor backSensor(BACK_TRIG_PIN, BACK_ECHO_PIN, &backSensorLogger, SENSOR_LOGGING_THRESHOLD, SENSOR_SMOOTHING_WINDOW);
RCReceiver remoteControl(REMOTE_THROTTLE_PIN, REMOTE_STEERING_PIN, &remoteLogger, REMOTE_THROTTLE_NEUTRAL_MIN, REMOTE_THROTTLE_NEUTRAL_MAX, REMOTE_STEERING_NEUTRAL_MIN, REMOTE_STEERING_NEUTRAL_MAX);
GearShifter shifter(SHIFTER_PIN, &shifterLogger);
DriveController driveController(MOTOR_DIR_PIN, MOTOR_PWM_PIN, MOTOR_PWM_CHANNEL, &driveLogger);
SteeringController steeringController(STEERING_SERVO_PIN, &steeringLogger);
SystemStatus systemStatus(RGB_PIN, &systemStatusLogger);

// State Machine: The current state is now defined in the shared header.
CarState currentState = CarState::STOPPED;

//================================================================================
// --- SETUP ---
// This function runs once at boot to initialize everything.
//================================================================================
void setup() {
  Serial.begin(115200);
  
  // Call the setup method for each of our component modules
  systemStatus.setup();
  accelerator.setup();
  frontSensor.setup();
  backSensor.setup();
  remoteControl.setup();
  shifter.setup();
  driveController.setup();
  steeringController.setup();
  
  // Check if all critical components initialized properly
  bool initializationSuccess = true;
  
  if (!systemStatus.isWorking()) {
    Serial.println("ERROR: SystemStatus failed to initialize");
    initializationSuccess = false;
  }
  
  if (!driveController.isInitialized()) {
    Serial.println("ERROR: DriveController failed to initialize");
    initializationSuccess = false;
  }
  
  if (initializationSuccess) {
    Serial.println("\n--- Ride-On Car Final Version Initialized ---");
    // Diagnostic information
    Serial.println("Diagnostic Info:");
    Serial.print("Motor PWM Channel: ");
    Serial.println(MOTOR_PWM_CHANNEL);
    Serial.print("Front Sensor Trig Pin: ");
    Serial.println(FRONT_TRIG_PIN);
    Serial.print("Front Sensor Echo Pin: ");
    Serial.println(FRONT_ECHO_PIN);
    Serial.print("Back Sensor Trig Pin: ");
    Serial.println(BACK_TRIG_PIN);
    Serial.print("Back Sensor Echo Pin: ");
    Serial.println(BACK_ECHO_PIN);
  } else {
    Serial.println("\n--- Ride-On Car Initialization FAILED ---");
  }
}

// Diagnostic function to check system status
void printSystemStatus() {
  Serial.println("--- System Status ---");
  Serial.print("Current State: ");
  switch(currentState) {
    case CarState::STOPPED:
      Serial.println("STOPPED");
      break;
    case CarState::FORWARD:
      Serial.println("FORWARD");
      break;
    case CarState::REVERSE:
      Serial.println("REVERSE");
      break;
    case CarState::AVOIDING_OBSTACLE:
      Serial.println("AVOIDING_OBSTACLE");
      break;
    case CarState::MANUAL_OVERRIDE:
      Serial.println("MANUAL_OVERRIDE");
      break;
  }
  
  // Check sensor status
  long frontDistance = frontSensor.getDistanceCm();
  long backDistance = backSensor.getDistanceCm();
  
  Serial.print("Front Distance: ");
  if (frontDistance < 0) {
    Serial.println("ERROR");
  } else {
    Serial.print(frontDistance);
    Serial.println(" cm");
  }
  
  Serial.print("Back Distance: ");
  if (backDistance < 0) {
    Serial.println("ERROR");
  } else {
    Serial.print(backDistance);
    Serial.println(" cm");
  }
  
  // Check motor status
  Serial.print("Motor Speed: ");
  Serial.println(accelerator.getMotorOutput());
  
  // Check LED status
  Serial.print("LED Status: ");
  if (systemStatus.isWorking()) {
    Serial.println("OK");
  } else {
    Serial.println("ERROR");
  }
  
  // Check motor controller status
  Serial.print("Motor Controller: ");
  if (driveController.isInitialized()) {
    Serial.println("OK");
  } else {
    Serial.println("ERROR");
  }
  Serial.println("---------------------");
}

//================================================================================
// --- MAIN LOOP ---
// This function runs continuously, as fast as possible.
//================================================================================
void loop() {
  //----------------------------------------------------------------
  // PHASE 1: DATA GATHERING
  // Always read the latest status from all inputs at the start of the loop.
  //----------------------------------------------------------------
  accelerator.update();
  remoteControl.update();
  shifter.update();
  frontSensor.update();
  backSensor.update();
  long frontDistance = frontSensor.getDistanceCm();
  long backDistance = backSensor.getDistanceCm();
  int currentCarSpeed = accelerator.getCurrentSpeed();
  Gear currentGear = shifter.getGear();

  //----------------------------------------------------------------
  // PHASE 2: DYNAMIC SAFETY CALCULATION
  // Calculate the required stopping distance based on current speed.
  //----------------------------------------------------------------
  long dynamicSafetyDistance = map(currentCarSpeed, 0, 100, MIN_SAFETY_DISTANCE_CM, MAX_SAFETY_DISTANCE_CM);

  //----------------------------------------------------------------
  // PHASE 3: HIERARCHICAL STATE CHECKS
  // This is the "Pyramid of Control" where we check for high-priority events.
  //----------------------------------------------------------------
  // Level 1: Collision Avoidance (Highest Priority)
  bool collision_hazard = (currentState == CarState::FORWARD && frontDistance < dynamicSafetyDistance && frontDistance > 0) ||
                          (currentState == CarState::REVERSE && backDistance < dynamicSafetyDistance && backDistance > 0);

  if (collision_hazard) {
    currentState = CarState::AVOIDING_OBSTACLE;
  }
  // Level 2: Manual RC Override
  else if (remoteControl.isOverriding() && currentState != CarState::MANUAL_OVERRIDE) {
    currentState = CarState::MANUAL_OVERRIDE;
  }
  // Level 3: Normal State Machine Logic (only runs if no overrides are active)
  else if (currentState != CarState::MANUAL_OVERRIDE) {
    // This simple switch just handles transitions based on the pedal and gear.
    switch (currentState) {
      case CarState::STOPPED:
        if (accelerator.getMotorOutput() > 5) {
          if (currentGear == Gear::FORWARD) { currentState = CarState::FORWARD; }
          else { currentState = CarState::REVERSE; }
        }
        break;
      case CarState::FORWARD:
      case CarState::REVERSE:
        if (accelerator.getMotorOutput() < 5) {
          currentState = CarState::STOPPED;
        }
        break;
      case CarState::AVOIDING_OBSTACLE:
        // This state is handled in the action phase below. This case just prevents other logic from running.
        break;
    }
  }

  //----------------------------------------------------------------
  // PHASE 4: STATE-BASED ACTION
  // This is the main state machine where we define what happens in each state.
  //----------------------------------------------------------------
  int finalMotorSpeed = 0;
  int finalSteering = 0;

  switch (currentState) {
    case CarState::STOPPED:
      stateLogger.log("STOPPED");
      finalMotorSpeed = 0;
      finalSteering = 0;
      break; 
    case CarState::FORWARD:
      stateLogger.log("FORWARD");
      finalMotorSpeed = accelerator.getMotorOutput();
      finalSteering = 0;
      break;
    case CarState::REVERSE:
      stateLogger.log("REVERSE");
      finalMotorSpeed = -accelerator.getMotorOutput();
      finalSteering = 0;
      break;
    case CarState::AVOIDING_OBSTACLE:
      stateLogger.log("AVOIDING_OBSTACLE");
      accelerator.overrideSpeed(0);
      finalMotorSpeed = accelerator.getMotorOutput();
      if (!collision_hazard && accelerator.getMotorOutput() < 5) {
          currentState = CarState::STOPPED;
      }
      break;
    case CarState::MANUAL_OVERRIDE:
      stateLogger.log("MANUAL_OVERRIDE");
      finalMotorSpeed = map(remoteControl.getThrottle(), -100, 100, -255, 255);
      finalSteering = map(remoteControl.getSteering(), -100, 100, -100, 100);
      if (!remoteControl.isOverriding()) {
        currentState = CarState::STOPPED;
      }
      break;
  }

  //----------------------------------------------------------------
  // PHASE 5: EXECUTION
  // Send the final commands to the hardware controllers.
  //----------------------------------------------------------------
  driveController.setSpeed(finalMotorSpeed);
  steeringController.setSteering(finalSteering);
  systemStatus.update(currentState);

}
