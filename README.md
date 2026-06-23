# Fuzzy Logic Robot

Autonomous mobile robot project based on an ESP32 DevKit V1, ESP-IDF, C++, PlatformIO, Wokwi, DC motors, an HC-SR04 ultrasonic distance sensor, a TB6612FNG HS-166 motor driver, and a hybrid fuzzy logic + motion control system for obstacle avoidance and autonomous navigation.

The system evolved from a rule-based controller into a fuzzy inference engine combined with a motion stabilization layer to ensure physically consistent behavior on real hardware.

---

## Project Objective

The objective of this project is to develop a functional autonomous mobile robot capable of:

- Perceiving obstacles using ultrasonic sensors  
- Making continuous navigation decisions using fuzzy logic  
- Executing stable steering maneuvers on real hardware  
- Maintaining consistent motion during obstacle avoidance  
- Operating reliably under sensor noise and mechanical constraints  

---

## Current System Architecture

The robot uses a two-layer control system:

### 1. Fuzzy Logic Controller (Decision Layer)

Responsible for transforming sensor inputs into continuous movement intentions.

Inputs:
- Front distance sensor (primary navigation)
- Rear distance sensor (safety constraint)

Processing:
- Fuzzification using membership functions (triangle and shoulder functions)
- Rule evaluation using fuzzy AND logic
- Defuzzification using weighted average method

Outputs:
- Traction command (forward / reverse intensity)
- Steering command (left / right intensity)

The fuzzy layer defines what the robot intends to do, not how the motion is physically executed.

---

### 2. Motion Control Layer (Steering Stabilization)

A state machine inside RobotController ensures physically stable movement execution.

This layer implements:

- Steering pulse duration control
- Steering cooldown mechanism
- Steering persistence across control cycles
- Prevention of oscillatory behavior caused by rapid updates

States:
- IDLE
- STEERING_ACTIVE
- COOLDOWN

This layer defines how the robot executes and sustains turns in physical space.

---

## System Flow

Sensor Input (HC-SR04 Front and Rear)
        |
        v
FuzzyController (decision engine)
        |
        v
RobotController (motion stabilization layer)
        |
        v
MotorDriver (TB6612FNG PWM control)
        |
        v
Physical robot motion

---

## Current Robot Behavior

### Open Path
- High or medium forward speed
- Steering centered
- Continuous forward motion

### Distant Obstacle
- Reduced speed
- Mild steering correction
- Early trajectory adjustment

### Near Obstacle
- Strong steering activation
- Reduced forward speed
- Avoidance maneuver initiated

### Immediate Obstacle
- Reverse motion activated
- Steering applied for escape direction
- Transition into recovery behavior

---

## Fuzzy Logic Design

### Membership Functions

The system defines fuzzy sets for distance evaluation:

- Very Close
- Near
- Avoid
- Slowdown
- Clear (open-ended safe region)

The Clear set is intentionally open-ended to avoid dead zones in open environments.

---

### Rule Base

- IF front is Very Close THEN Stop or Reverse
- IF front is Near AND rear is Free THEN Reverse Left
- IF front is Avoid THEN Turn Right
- IF front is Slowdown THEN Reduce Speed
- IF front is Clear THEN Cruise Forward

---

## Motion Control Layer

The RobotController enforces physical stability through a finite state machine:

States:
- IDLE
- STEERING_ACTIVE
- COOLDOWN

Behavior:
- Steering is sustained for a minimum duration
- Steering changes reset timing windows
- Cooldown prevents rapid oscillation
- Ensures stable turning behavior on real hardware

---

## Safety Fallback Behavior

If no fuzzy rule is activated due to edge cases:

- The system defaults to a safe cruise state
- Forward motion is maintained at reduced speed

This prevents complete stopping in undefined fuzzy regions.

---

## Hardware Platform

- ESP32 DevKit V1  
- HC-SR04 ultrasonic sensors  
- TB6612FNG motor driver  
- DC motors  
- External motor power supply  
- 3D printed chassis  

---

## Motor Driver Behavior

Speed range:
-255 to +255

Mapping:
- Positive values: forward rotation  
- Negative values: reverse rotation  
- Zero: stop  

Direction logic:
- IN1/IN2 define direction  
- PWM defines speed  

---

## Motor Driver Test Mode

A built-in test mode validates motor driver behavior without sensors or fuzzy logic.

It tests:
- Forward motion at different speeds  
- Reverse motion  
- Turning behavior  
- Stop and disable states  

This mode is used for hardware validation and Wokwi simulation.

---

## Development Environment

- ESP-IDF framework  
- PlatformIO build system  
- C++ implementation  
- Wokwi simulation support  
- Git version control  

---

## System Evolution

1. Direct motor control (manual PWM)  
2. Threshold-based logic (if/else rules)  
3. Fuzzy logic decision system  
4. Hybrid fuzzy + motion stabilization system (current)  

---

## Current Status

The system is currently in a validated hybrid control stage.

The robot is capable of:

- Autonomous forward navigation  
- Obstacle detection and avoidance  
- Reverse escape maneuvers  
- Sustained steering behavior  
- Stable real-world motion under sensor noise  

The fuzzy system is fully integrated with a motion execution layer that ensures physically consistent behavior.

---

## Next Improvements

- Sensor noise filtering (moving average or Kalman filter)  
- Fuzzy membership tuning for smoother transitions  
- Steering vectorization (-1 to +1 continuous model)  
- Behavior-based navigation layer  
- Improved trajectory prediction using rear sensor fusion  