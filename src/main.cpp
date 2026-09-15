#include <Arduino.h>
#include "esp_task_wdt.h"  // ESP-IDF task watchdog timer
#include "pins.h"

#include "./modules/Accelerator/Accelerator.h"
#include "./modules/ProximitySensor/ProximitySensor.h"
#include "./modules/Logger/Logger.h"
#include "./modules/RCReceiver/RCReceiver.h"
#include "./modules/GearShifter/GearShifter.h"
#include "./modules/DriveController/DriveController.h"
#include "./modules/SteeringController/SteeringController.h"
#include "./modules/SystemStatus/SystemStatus.h"
#include "./Shared/CarState.h"
#include "./Shared/ControlLogic.h"

// All pin assignments and hardware tuning constants come from pins.h
// (single source of truth — do not re-declare them here).

// Watchdog timer: resets the ESP32 if loop() stops running (system hang).
// The loop must call esp_task_wdt_feed() at least once per timeout period.
// 5 seconds (5000 ms) is generous for normal operation but catches infinite loops.
#define WDT_TIMEOUT_MS 5000

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
Logger estopLogger("E-Stop");

// Hardware module instances — every pin and calibration constant from pins.h
Accelerator accelerator(PEDAL_PIN, &accelLogger, PEDAL_ADC_MIN, PEDAL_ADC_MAX);
ProximitySensor frontSensor(FRONT_TRIG_PIN, FRONT_ECHO_PIN, &frontSensorLogger, SENSOR_LOGGING_THRESHOLD, SENSOR_SMOOTHING_WINDOW);
ProximitySensor backSensor(BACK_TRIG_PIN, BACK_ECHO_PIN, &backSensorLogger, SENSOR_LOGGING_THRESHOLD, SENSOR_SMOOTHING_WINDOW);
RCReceiver remoteControl(REMOTE_THROTTLE_PIN, REMOTE_STEERING_PIN, &remoteLogger,
                         REMOTE_THROTTLE_NEUTRAL_MIN, REMOTE_THROTTLE_NEUTRAL_MAX,
                         REMOTE_STEERING_NEUTRAL_MIN, REMOTE_STEERING_NEUTRAL_MAX,
                         RC_SIGNAL_TIMEOUT_MS);
GearShifter shifter(SHIFTER_PIN, &shifterLogger);
DriveController driveController(MOTOR_DIR_PIN, MOTOR_PWM_PIN, MOTOR_PWM_CHANNEL, &driveLogger);
SteeringController steeringController(STEERING_SERVO_PIN, &steeringLogger);
SystemStatus systemStatus(RGB_PIN, &systemStatusLogger);

// Control state machine (pure logic in Shared/ControlLogic.h)
CarControlState controlState;
TuneParams tune;

unsigned long lastLoopMs = 0;

// E-stop debounce bookkeeping
bool estopStable = false;
unsigned long estopLastChangeMs = 0;

/**
 * @brief Read the e-stop button with debounce (active-low, internal pull-up).
 *
 * Engages on the first sampled press (safety first), and requires
 * ESTOP_DEBOUNCE_MS of stability on either transition to reject contact
 * chatter and EMI from the motor driver.
 */
bool readEstop() {
  bool raw = (digitalRead(ESTOP_PIN) == ESTOP_ACTIVE_LEVEL);
  if (raw != estopStable) {
    unsigned long now = millis();
    if (now - estopLastChangeMs >= ESTOP_DEBOUNCE_MS) {
      estopStable = raw;
    }
    estopLastChangeMs = now;
  }
  return estopStable;
}

/**
 * @brief Boot-time diagnostic that supports pedal calibration.
 *
 * Prints raw ADC samples so PEDAL_ADC_MIN / PEDAL_ADC_MAX in pins.h can be
 * set to the pedal's actual rest/pressed range (see the CALIBRATION
 * PROCEDURE comment in pins.h).
 */
void pedalCalibrationDiagnostic() {
  Serial.println("--- Pedal Calibration Diagnostic ---");
  Serial.println("Release the pedal fully, then press it fully and hold for 3 seconds...");
  delay(3000);

  int minVal = 4096, maxVal = -1;
  Serial.println("Reading 10 raw samples:");
  for (int i = 0; i < 10; i++) {
    int v = analogRead(PEDAL_PIN);
    minVal = min(minVal, v);
    maxVal = max(maxVal, v);
    Serial.print("  Raw pedal ADC: ");
    Serial.println(v);
  }
  Serial.print("Observed range: ");
  Serial.print(minVal);
  Serial.print(" .. ");
  Serial.println(maxVal);
  if (minVal != PEDAL_ADC_MIN || maxVal != PEDAL_ADC_MAX) {
    Serial.println("NOTE: observed range differs from PEDAL_ADC_MIN/PEDAL_ADC_MAX in pins.h.");
    Serial.println("Update those two constants and rebuild for exact 0-100% pedal travel.");
  }
  Serial.println("Calibration diagnostic complete.");
}

void setup() {
  Serial.begin(115200);

  // Initialize hardware watchdog timer to catch system hangs
  esp_task_wdt_init(WDT_TIMEOUT_MS);
  esp_task_wdt_add(NULL);  // NULL = current task (the Arduino loop task)
  Serial.println("Watchdog timer initialized (" + String(WDT_TIMEOUT_MS) + " ms timeout)");

  pinMode(ESTOP_PIN, INPUT_PULLUP);

  systemStatus.setup();
  accelerator.setup();
  frontSensor.setup();
  backSensor.setup();
  remoteControl.setup();
  shifter.setup();
  driveController.setup();
  steeringController.setup();

  // Populate the control state machine from the centralized tuning constants
  tune.accelIntervalLow = ACCEL_INTERVAL_LOW;
  tune.accelIntervalMid = ACCEL_INTERVAL_MID;
  tune.accelIntervalHigh = ACCEL_INTERVAL_HIGH;
  tune.brakingInterval = BRAKING_INTERVAL;
  tune.minSafetyCm = MIN_SAFETY_DISTANCE_CM;
  tune.maxSafetyCm = MAX_SAFETY_DISTANCE_CM;

  bool initializationSuccess = true;

  if (!systemStatus.isWorking()) {
    Serial.println("ERROR: SystemStatus failed to initialize");
    initializationSuccess = false;
  }

  if (!driveController.isInitialized()) {
    Serial.println("ERROR: DriveController failed to initialize");
    initializationSuccess = false;
  }

  if (readEstop()) {
    Serial.println("WARNING: E-STOP is being held at boot - the car will not drive until released");
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
    Serial.print("E-Stop Pin: ");
    Serial.println(ESTOP_PIN);
    Serial.print("Pedal Calibration Range: ");
    Serial.print(PEDAL_ADC_MIN);
    Serial.print(" .. ");
    Serial.println(PEDAL_ADC_MAX);
    Serial.print("RC Link Timeout (ms): ");
    Serial.println(RC_SIGNAL_TIMEOUT_MS);
    pedalCalibrationDiagnostic();
  } else {
    Serial.println("\n--- Ride-On Car Initialization FAILED ---");
  }
}

void loop() {
  // Feed watchdog to signal the system is alive
  esp_task_wdt_feed();

  // PHASE 1: DATA GATHERING (all non-blocking)
  accelerator.update();
  remoteControl.update();
  shifter.update();
  frontSensor.update();
  backSensor.update();

  // PHASE 2: BUILD CONTROL INPUTS
  unsigned long now = millis();
  unsigned long dtMs = now - lastLoopMs;
  lastLoopMs = now;
  if (dtMs == 0) dtMs = 1;

  ControlInputs in;
  in.estopPressed = readEstop();
  in.pedalIntent = accelerator.getPedalIntent();
  in.gear = shifter.getGear();
  in.rcThrottle = remoteControl.getThrottle();
  in.rcSteering = remoteControl.getSteering();
  in.rcSignalValid = remoteControl.isSignalValid();
  in.rcOverrideActive = remoteControl.isOverriding();
  in.frontCm = frontSensor.getDistanceCm();
  in.frontValid = frontSensor.isValid();
  in.backCm = backSensor.getDistanceCm();
  in.backValid = backSensor.isValid();
  in.dtMs = dtMs;

  // PHASE 3: PURE CONTROL DECISION
  // Priority: E-STOP > collision avoidance (fail-closed) > RC override > pedal.
  ControlOutputs out = control::stepControl(in, controlState, tune);

  // PHASE 4: LOGGING
  stateLogger.log(carStateName(controlState.state));
  estopLogger.log(in.estopPressed ? "ENGAGED" : "released");

  // PHASE 5: ACTUATOR OUTPUT
  driveController.setSpeed(out.motorSpeed);
  steeringController.setSteering(out.steering);
  systemStatus.update(controlState.state);
}
