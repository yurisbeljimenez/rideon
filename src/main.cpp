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

// Pin definitions are included from pins.h to avoid conflicts
#define PEDAL_PIN 4
#define SHIFTER_PIN 16
#define REMOTE_THROTTLE_PIN 15
#define REMOTE_STEERING_PIN 14
#define FRONT_TRIG_PIN 12
#define FRONT_ECHO_PIN 13
#define BACK_TRIG_PIN 35
#define BACK_ECHO_PIN 36
#define MOTOR_DIR_PIN 1
#define MOTOR_PWM_PIN 2
#define STEERING_SERVO_PIN 17
#define RGB_PIN 15

// Tuning constants
const long ACCEL_INTERVAL_LOW = 30;
const long ACCEL_INTERVAL_MID = 10;
const long ACCEL_INTERVAL_HIGH = 25;
const int ACCEL_LOGGING_THRESHOLD = 5;
const long BRAKING_INTERVAL = 5;
const int REMOTE_THROTTLE_NEUTRAL_MIN = 1490;
const int REMOTE_THROTTLE_NEUTRAL_MAX = 1530;
const int REMOTE_STEERING_NEUTRAL_MIN = 1480;
const int REMOTE_STEERING_NEUTRAL_MAX = 1520;
const int MIN_SAFETY_DISTANCE_CM = 20;
const int MAX_SAFETY_DISTANCE_CM = 80;
const int SENSOR_LOGGING_THRESHOLD = 100;
const int SENSOR_SMOOTHING_WINDOW = 5;
const int MOTOR_PWM_CHANNEL = 0;

// Logger instances for each module
Logger systemStatusLogger("System Status");
Logger accelLogger("Accelerator");
Logger frontSensorLogger("Front Sensor");
Logger backSensorLogger("Back Sensor");
Logger stateLogger("Car State");
Logger remoteLogger("Remote Control");
Logger shifterLogger("Gear Shifter");
Logger driveLogger("Drive Controller");
Logger steeringLogger("Steering");

// Hardware module instances
Accelerator accelerator(PEDAL_PIN, &accelLogger, ACCEL_INTERVAL_LOW, ACCEL_INTERVAL_MID, ACCEL_INTERVAL_HIGH, BRAKING_INTERVAL, ACCEL_LOGGING_THRESHOLD);
ProximitySensor frontSensor(FRONT_TRIG_PIN, FRONT_ECHO_PIN, &frontSensorLogger, SENSOR_LOGGING_THRESHOLD, SENSOR_SMOOTHING_WINDOW);
ProximitySensor backSensor(BACK_TRIG_PIN, BACK_ECHO_PIN, &backSensorLogger, SENSOR_LOGGING_THRESHOLD, SENSOR_SMOOTHING_WINDOW);
RCReceiver remoteControl(REMOTE_THROTTLE_PIN, REMOTE_STEERING_PIN, &remoteLogger, REMOTE_THROTTLE_NEUTRAL_MIN, REMOTE_THROTTLE_NEUTRAL_MAX, REMOTE_STEERING_NEUTRAL_MIN, REMOTE_STEERING_NEUTRAL_MAX);
GearShifter shifter(SHIFTER_PIN, &shifterLogger);
DriveController driveController(MOTOR_DIR_PIN, MOTOR_PWM_PIN, MOTOR_PWM_CHANNEL, &driveLogger);
SteeringController steeringController(STEERING_SERVO_PIN, &steeringLogger);
SystemStatus systemStatus(RGB_PIN, &systemStatusLogger);

CarState currentState = CarState::STOPPED;

void setup() {
  Serial.begin(115200);
  
  systemStatus.setup();
  accelerator.setup();
  frontSensor.setup();
  backSensor.setup();
  remoteControl.setup();
  shifter.setup();
  driveController.setup();
  steeringController.setup();
  
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
    Serial.println("Diagnostic Info:");
    Serial.print("Motor PWM Channel: ");
    Serial.println(MOTOR_PWM_CHANNEL);
    Serial.print("Front Sensor Trig Pin: ");
    Serial.println(FRONT_TRIG_PIN);
    Serial.print("Front Sensor Echo Pin: ");
    Serial.println(FRONT_ECHO_PIN);
  } else {
    Serial.println("\n--- Ride-On Car Initialization FAILED ---");
  }
}

void printSystemStatus() {
  Serial.println("--- System Status ---");
  Serial.print("Current State: ");
  switch(currentState) {
    case CarState::STOPPED: Serial.println("STOPPED"); break;
    case CarState::FORWARD: Serial.println("FORWARD"); break;
    case CarState::REVERSE: Serial.println("REVERSE"); break;
    case CarState::AVOIDING_OBSTACLE: Serial.println("AVOIDING_OBSTACLE"); break;
    case CarState::MANUAL_OVERRIDE: Serial.println("MANUAL_OVERRIDE"); break;
  }
  long frontDistance = frontSensor.getDistanceCm();
  long backDistance = backSensor.getDistanceCm();
  Serial.print("Front Distance: ");
  Serial.println(frontDistance < 0 ? "ERROR" : String(frontDistance) + " cm");
  Serial.print("Back Distance: ");
  Serial.println(backDistance < 0 ? "ERROR" : String(backDistance) + " cm");
  Serial.print("Motor Speed: ");
  Serial.println(accelerator.getMotorOutput());
  Serial.print("LED Status: ");
  Serial.println(systemStatus.isWorking() ? "OK" : "ERROR");
  Serial.print("Motor Controller: ");
  Serial.println(driveController.isInitialized() ? "OK" : "ERROR");
  Serial.println("---------------------");
}

void loop() {
  // PHASE 1: DATA GATHERING
  accelerator.update();
  remoteControl.update();
  shifter.update();
  frontSensor.update();
  backSensor.update();
  
  long frontDistance = frontSensor.getDistanceCm();
  long backDistance = backSensor.getDistanceCm();
  int currentCarSpeed = accelerator.getCurrentSpeed();
  Gear currentGear = shifter.getGear();

  // PHASE 2: DYNAMIC SAFETY CALCULATION
  long dynamicSafetyDistance = map(currentCarSpeed, 0, 100, MIN_SAFETY_DISTANCE_CM, MAX_SAFETY_DISTANCE_CM);

  // PHASE 3: HIERARCHICAL STATE CHECKS
  bool collision_hazard = (currentState == CarState::FORWARD && frontDistance < dynamicSafetyDistance && frontDistance > 0) ||
                          (currentState == CarState::REVERSE && backDistance < dynamicSafetyDistance && backDistance > 0);

  if (collision_hazard) {
    currentState = CarState::AVOIDING_OBSTACLE;
  } else if (remoteControl.isOverriding() && currentState != CarState::MANUAL_OVERRIDE) {
    currentState = CarState::MANUAL_OVERRIDE;
  } else if (currentState != CarState::MANUAL_OVERRIDE) {
    switch (currentState) {
      case CarState::STOPPED:
        if (accelerator.getMotorOutput() > 5) {
          currentState = (currentGear == Gear::FORWARD) ? CarState::FORWARD : CarState::REVERSE;
        }
        break;
      case CarState::FORWARD:
      case CarState::REVERSE:
        if (accelerator.getMotorOutput() < 5) {
          currentState = CarState::STOPPED;
        }
        break;
      default:
        break;
    }
  }

  // PHASE 4 & 5: STATE-BASED ACTION AND EXECUTION
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

  driveController.setSpeed(finalMotorSpeed);
  steeringController.setSteering(finalSteering);
  systemStatus.update(currentState);
}