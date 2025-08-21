#include <Arduino.h>
#include "./modules/Accelerator/Accelerator.h"
#include "./modules/ProximitySensor/ProximitySensor.h"
#include "./modules/Logger/Logger.h"
#include "./modules/RCReceiver/RCReceiver.h"
#include "./modules/GearShifter/GearShifter.h"
#include "./modules/DriveController/DriveController.h"
#include "./modules/SteeringController/SteeringController.h"
#include "./modules/SystemStatus/SystemStatus.h"
#include "./Shared/CarState.h" // CORRECTED: Path from main.cpp to sibling folder Shared

//================================================================================
// --- PIN DEFINITIONS ---
// A central place to define all hardware connections to the ESP32.
//================================================================================
// Inputs
#define RGB_PIN 48              // Onboard RGB LED light.
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

//================================================================================
// --- TUNING CONSTANTS ---
// A central place for all performance tuning.
//================================================================================
// Acceleration Curve
const long ACCEL_INTERVAL_LOW = 30;   // 30ms: Very gentle start from 0-25% speed
const long ACCEL_INTERVAL_MID = 10;   // 10ms: Responsive acceleration from 26-80% speed
const long ACCEL_INTERVAL_HIGH = 25;  // 25ms: Eases into the final top speed (81-100%)
// Active Braking
const long BRAKING_INTERVAL = 5;      // 5ms: Very fast deceleration for single-pedal feel

const int MOTOR_PWM_CHANNEL = 0;        // ESP32 LEDC PWM channel (0-15) for the drive motor
const int MIN_SAFETY_DISTANCE_CM = 20;  // Safety distance at 0 speed
const int MAX_SAFETY_DISTANCE_CM = 80;  // Safety distance at max speed

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
Accelerator accelerator(PEDAL_PIN, &accelLogger, ACCEL_INTERVAL_LOW, ACCEL_INTERVAL_MID, ACCEL_INTERVAL_HIGH, BRAKING_INTERVAL); 
ProximitySensor frontSensor(FRONT_TRIG_PIN, FRONT_ECHO_PIN, &frontSensorLogger);
ProximitySensor backSensor(BACK_TRIG_PIN, BACK_ECHO_PIN, &backSensorLogger);
RCReceiver remoteControl(REMOTE_THROTTLE_PIN, REMOTE_STEERING_PIN, &remoteLogger);
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
  
  Serial.println("\n--- Single-Pedal Driving Initialized ---");
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
