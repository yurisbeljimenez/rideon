# Project Q&A: ESP32-S3 Ride-On Car

This document answers common questions about the design, functionality, and tuning of the ride-on car's control system.

---

### **Q1: What is the core design philosophy of the software?**

**A:** The software is built on a professional, modular architecture. The main goal is to separate responsibilities, making the system robust, safe, and easy to maintain. It uses a non-blocking state machine in `main.cpp` as a high-level coordinator, which delegates all hardware-specific tasks to self-contained, documented modules (e.g., `Accelerator`, `DriveController`).

---

### **Q2: How does the safety system work? What is the "Pyramid of Control"?**

**A:** The safety system is built on a strict hierarchy of control, ensuring that the most critical safety features always have the highest priority. The pyramid is:

1.  **Collision Avoidance (Highest Authority):** If the ultrasonic sensors detect an obstacle in the path of travel, the system will override _all_ other inputs (both the child's and the parent's) and initiate a smooth, automatic "Soft Stop."
2.  **Parental RC Override:** If there is no immediate collision danger, the parent's RC remote has the next highest priority. It instantly takes control from the child's inputs.
3.  **Child's Normal Input:** If no safety systems are active, the car operates based on the child's accelerator pedal and gear shifter inputs.

---

### **Q3: What happens if the car is about to hit a wall? Can the driver recover?**

**A:** The car will **not** hit the wall. The collision avoidance system will detect the obstacle based on a dynamic safety distance that increases with speed. It will then enter the `AVOIDING_OBSTACLE` state and bring the car to a safe, smooth stop.

The system will **not** freeze. Once stopped, it will wait for two conditions to be met before handing back control:

1.  The obstacle is no longer in the path.
2.  The child's foot is off the accelerator pedal.

Once these are met, the car returns to the `STOPPED` state, and the driver can then choose to shift into reverse and safely back away.

---

### **Q4: How does the "single-pedal driving" feature work?**

**A:** The `Accelerator` module is programmed for an intuitive single-pedal feel.

- **Acceleration:** When the pedal is pressed, the car accelerates using a 3-stage "ease-in" curve for a smooth, natural feel.
- **Braking:** When the pedal is released, the system uses a much faster deceleration rate, causing the car to brake actively and come to a quick but controlled stop. This acts as the primary braking system and is a key safety feature.

---

### **Q5: How do I change how the car feels to drive?**

**A:** All performance tuning is handled by changing a few clearly-labeled constants at the top of the **`main.cpp`** file. You do not need to edit the logic in the modules.

- **To change acceleration:** Adjust the `ACCEL_INTERVAL_LOW`, `_MID`, and `_HIGH` values. A smaller number means faster acceleration.
- **To change braking strength:** Adjust the `BRAKING_INTERVAL`. A smaller number means a faster, more aggressive stop.
- **To change safety distance:** Adjust the `MIN_SAFETY_DISTANCE_CM` and `MAX_SAFETY_DISTANCE_CM` values.

---

### **Q6: What do the colors on the onboard LED mean?**

**A:** The onboard RGB LED provides an at-a-glance diagnostic of the car's current state, managed by the `SystemStatus` module.

- **Solid Blue:** The car is `STOPPED` and ready.
- **Solid Green:** The car is moving `FORWARD` under normal control.
- **Solid Yellow:** The car is moving in `REVERSE`.
- **Pulsing Red:** The collision avoidance system is active (`AVOIDING_OBSTACLE`).
- **Pulsing White:** The parent has taken control via the RC remote (`MANUAL_OVERRIDE`).
