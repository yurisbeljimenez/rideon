/**
 * @file test_control_logic.cpp
 * @brief Host-side unit tests for the pure control logic state machine.
 *     g++ -std=c++17 -I src -o test/test_control_logic test/test_control_logic.cpp && ./test/test_control_logic
 */
#include "Shared/ControlLogic.h"
#include <cassert>
#include <cstdio>

ControlInputs safeInputs() {
    ControlInputs in;
    in.frontCm = 200; in.frontValid = true;
    in.backCm = 200; in.backValid = true;
    in.rcSignalValid = true; in.rcOverrideActive = false;
    in.dtMs = 10;
    return in;
}
TuneParams fastTune() {
    TuneParams t;
    t.accelIntervalLow = 1; t.accelIntervalMid = 1;
    t.accelIntervalHigh = 1; t.brakingInterval = 1;
    return t;
}
void test_estop_hard_stop() {
    TuneParams t = fastTune();
    CarControlState st;
    st.state = CarState::FORWARD; st.currentSpeed = 50;
    ControlInputs in = safeInputs();
    in.estopPressed = true; in.pedalIntent = 100;
    ControlOutputs out = control::stepControl(in, st, t);
    assert(st.state == CarState::STOPPED);
    assert(st.currentSpeed == 0);
    assert(out.motorSpeed == 0);
    printf("[PASS] test_estop_hard_stop\n");
}
void test_estop_overrides_rc() {
    TuneParams t = fastTune();
    CarControlState st; st.state = CarState::MANUAL_OVERRIDE;
    ControlInputs in = safeInputs();
    in.estopPressed = true; in.rcOverrideActive = true; in.rcThrottle = 100;
    ControlOutputs out = control::stepControl(in, st, t);
    assert(st.state == CarState::STOPPED);
    assert(out.motorSpeed == 0);
    printf("[PASS] test_estop_overrides_rc\n");
}
void test_collision_avoidance_forward() {
    TuneParams t = fastTune();
    CarControlState st;
    st.state = CarState::FORWARD; st.currentSpeed = 50;
    ControlInputs in = safeInputs();
    in.pedalIntent = 50; in.frontCm = 15; in.frontValid = true;
    ControlOutputs out = control::stepControl(in, st, t);
    assert(st.state == CarState::AVOIDING_OBSTACLE);
    assert(out.fault == true);
    assert(st.avoidDir == 1);
    printf("[PASS] test_collision_avoidance_forward\n");
}
void test_collision_avoidance_reverse() {
    TuneParams t = fastTune();
    CarControlState st;
    st.state = CarState::REVERSE; st.currentSpeed = 30;
    ControlInputs in = safeInputs();
    in.pedalIntent = 50; in.gear = Gear::REVERSE;
    in.backCm = 10; in.backValid = true;
    ControlOutputs out = control::stepControl(in, st, t);
    assert(st.state == CarState::AVOIDING_OBSTACLE);
    assert(st.avoidDir == -1);
    printf("[PASS] test_collision_avoidance_reverse\n");
}
void test_fail_closed_sensor_invalid() {
    TuneParams t = fastTune();
    CarControlState st;
    st.state = CarState::FORWARD; st.currentSpeed = 40;
    ControlInputs in = safeInputs();
    in.pedalIntent = 50; in.frontCm = -1; in.frontValid = false;
    ControlOutputs out = control::stepControl(in, st, t);
    assert(st.state == CarState::AVOIDING_OBSTACLE);
    assert(out.fault == true);
    printf("[PASS] test_fail_closed_sensor_invalid\n");
}
void test_recovery_from_avoidance() {
    TuneParams t = fastTune();
    CarControlState st;
    st.state = CarState::AVOIDING_OBSTACLE;
    st.currentSpeed = 0; st.avoidDir = 1;
    ControlInputs in = safeInputs();
    in.pedalIntent = 0; in.frontCm = 200;
    ControlOutputs out = control::stepControl(in, st, t);
    assert(st.state == CarState::STOPPED);
    assert(out.motorSpeed == 0);
    assert(st.avoidDir == 0);
    printf("[PASS] test_recovery_from_avoidance\n");
}
void test_avoidance_holds_while_pedal_held() {
    TuneParams t = fastTune();
    CarControlState st;
    st.state = CarState::AVOIDING_OBSTACLE;
    st.currentSpeed = 0; st.avoidDir = 1;
    ControlInputs in = safeInputs();
    in.pedalIntent = 50; in.frontCm = 200;
    ControlOutputs out = control::stepControl(in, st, t);
    assert(st.state == CarState::AVOIDING_OBSTACLE);
    assert(out.motorSpeed == 0);
    assert(out.fault == true);
    printf("[PASS] test_avoidance_holds_while_pedal_held\n");
}
void test_rc_override_priority() {
    TuneParams t = fastTune();
    CarControlState st;
    ControlInputs in = safeInputs();
    in.rcOverrideActive = true; in.rcThrottle = 80;
    in.rcSteering = -40; in.pedalIntent = 50;
    ControlOutputs out = control::stepControl(in, st, t);
    assert(st.state == CarState::MANUAL_OVERRIDE);
    assert(out.motorSpeed > 0);
    assert(out.steering == -40);
    printf("[PASS] test_rc_override_priority\n");
}

void test_rc_override_loss_stops() {
    TuneParams t = fastTune();
    CarControlState st;
    st.state = CarState::MANUAL_OVERRIDE; st.currentSpeed = 50;
    ControlInputs in = safeInputs();
    in.rcOverrideActive = false; in.rcSignalValid = false;
    ControlOutputs out = control::stepControl(in, st, t);
    assert(st.state == CarState::STOPPED);
    assert(out.motorSpeed == 0);
    assert(out.fault == true);
    printf("[PASS] test_rc_override_loss_stops\n");
}
void test_pedal_forward() {
    TuneParams t = fastTune();
    CarControlState st;
    ControlInputs in = safeInputs();
    in.pedalIntent = 100; in.gear = Gear::FORWARD;
    for (int i = 0; i < 100; i++) control::stepControl(in, st, t);
    assert(st.state == CarState::FORWARD);
    assert(st.currentSpeed > 0);
    printf("[PASS] test_pedal_forward\n");
}
void test_pedal_reverse() {
    TuneParams t = fastTune();
    CarControlState st;
    ControlInputs in = safeInputs();
    in.pedalIntent = 100; in.gear = Gear::REVERSE;
    for (int i = 0; i < 100; i++) control::stepControl(in, st, t);
    assert(st.state == CarState::REVERSE);
    printf("[PASS] test_pedal_reverse\n");
}
void test_active_braking() {
    TuneParams t = fastTune();
    CarControlState st;
    ControlInputs in = safeInputs();
    in.pedalIntent = 100;
    for (int i = 0; i < 200; i++) control::stepControl(in, st, t);
    int speedBefore = st.currentSpeed;
    assert(speedBefore > 10);
    in.pedalIntent = 0;
    for (int i = 0; i < 200; i++) control::stepControl(in, st, t);
    assert(st.currentSpeed < speedBefore);
    printf("[PASS] test_active_braking\n");
}
void test_dynamic_safety_distance() {
    assert(control::dynamicSafetyDistance(0, 20, 80) == 20);
    assert(control::dynamicSafetyDistance(100, 20, 80) == 80);
    long mid = control::dynamicSafetyDistance(50, 20, 80);
    assert(mid >= 45 && mid <= 55);
    printf("[PASS] test_dynamic_safety_distance\n");
}
void test_collision_during_override() {
    TuneParams t = fastTune();
    CarControlState st; st.state = CarState::MANUAL_OVERRIDE;
    ControlInputs in = safeInputs();
    in.rcOverrideActive = true; in.rcThrottle = 100;
    in.frontCm = 5; in.frontValid = true;
    ControlOutputs out = control::stepControl(in, st, t);
    assert(st.state == CarState::AVOIDING_OBSTACLE);
    assert(out.fault == true);
    printf("[PASS] test_collision_during_override\n");
}
int main() {
    printf("=== Ride-On Car Control Logic Unit Tests ===\n\n");
    test_estop_hard_stop();
    test_estop_overrides_rc();
    test_collision_avoidance_forward();
    test_collision_avoidance_reverse();
    test_fail_closed_sensor_invalid();
    test_recovery_from_avoidance();
    test_avoidance_holds_while_pedal_held();
    test_rc_override_priority();
    test_rc_override_loss_stops();
    test_pedal_forward();
    test_pedal_reverse();
    test_active_braking();
    test_dynamic_safety_distance();
    test_collision_during_override();
    printf("\n=== ALL 14 TESTS PASSED ===\n");
    return 0;
}