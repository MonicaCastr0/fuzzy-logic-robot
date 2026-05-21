# Fuzzy Logic Robot

ESP32 firmware project for an autonomous mobile robot with obstacle avoidance based on fuzzy logic.

This repository is part of the development of a mobile robotics prototype with the following final goal:

> Build a functional autonomous mobile robot, with a 3D-printed structure, ESP32-based control, distance sensing, DC motor locomotion through a motor driver, and a fuzzy decision module capable of adjusting speed and direction to avoid obstacles in a controlled environment.

## Current Status

The repository is currently in an early firmware architecture stage.

The current codebase focuses on:

- Setting up an ESP-IDF project with PlatformIO.
- Using C++ for a modular firmware architecture.
- Validating the ESP32 application entry point through `app_main()`.
- Creating initial firmware components as templates.
- Running a minimal Wokwi simulation with serial output.
- Preparing the project structure for future sensor, motor driver, and fuzzy logic implementation.

At this point, the project does **not** yet control the physical robot.

## Current Implementation

The firmware currently boots and executes a minimal `RobotController` template.

Current behavior:

1. ESP-IDF starts the application.
2. `app_main()` is called.
3. A `RobotController` object is created.
4. `RobotController::init()` is called once.
5. `RobotController::update()` is called continuously every 1 second.

Expected Wokwi/serial output pattern:

```text
I (...) main_task: Calling app_main()
I (...) RobotController: RobotController init
I (...) RobotController: RobotController update
I (...) RobotController: RobotController update
I (...) RobotController: RobotController update
```

This confirms that the base firmware flow is working.

## Hardware Target

The planned prototype uses the following hardware:

| Component | Purpose |
|---|---|
| ESP32 DevKit V1 | Main control unit |
| HC-SR04 5V | Distance sensing for obstacle detection |
| TB6612FNG HS-166 | Dual DC motor driver |
| 2 DC motors | Robot locomotion |
| External motor power supply | Power source for the motors |
| Voltage divider or logic level shifter | Protects the ESP32 when reading the HC-SR04 `ECHO` signal |
| 3D-printed chassis | Physical robot structure (in this case a remote control cart chassis) |

## Development Environment

Recommended tools:

| Tool | Purpose |
|---|---|
| VS Code | Code editor |
| PlatformIO | Build system and ESP32 project management |
| ESP-IDF | Native ESP32 firmware framework |
| C++ | Firmware implementation language |
| Wokwi | Firmware and circuit simulation |
| Git/GitHub | Version control and collaboration |

## PlatformIO Configuration

The current PlatformIO environment targets a generic ESP32 development board:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = espidf
monitor_speed = 115200
```

The `esp32dev` board profile is being used for the ESP32 DevKit V1 / ESP-WROOM-32 development board.

## Current Repository Structure

Current repository structure:

```text
fuzzy-logic-robot/
│
├── components/
│   ├── app_config/
│   │   ├── CMakeLists.txt
│   │   └── include/
│   │       └── AppConfig.hpp
│   │
│   ├── distance_sensor/
│   │   ├── CMakeLists.txt
│   │   ├── DistanceSensor.cpp
│   │   └── include/
│   │       └── DistanceSensor.hpp
│   │
│   ├── fuzzy_controller/
│   │   ├── CMakeLists.txt
│   │   ├── FuzzyController.cpp
│   │   └── include/
│   │       └── FuzzyController.hpp
│   │
│   ├── motor_driver/
│   │   ├── CMakeLists.txt
│   │   ├── MotorDriver.cpp
│   │   └── include/
│   │       └── MotorDriver.hpp
│   │
│   └── robot_controller/
│       ├── CMakeLists.txt
│       ├── RobotController.cpp
│       └── include/
│           └── RobotController.hpp
│
├── include/
│   └── README
│
├── lib/
├── src/
│   ├── CMakeLists.txt
│   └── main.cpp
│
├── test/
├── .gitignore
├── CMakeLists.txt
├── README.md
├── diagram.json
├── platformio.ini
└── wokwi.toml
```

## Current Firmware Architecture

The planned firmware architecture is modular:

```text
app_main()
   ↓
RobotController
   ├── DistanceSensor
   ├── MotorDriver
   └── FuzzyController
```

### Current wiring status

| Module | Exists in repository | Currently integrated into `RobotController` | Current behavior |
|---|---:|---:|---|
| `robot_controller` | Yes | Yes | Prints template logs |
| `app_config` | Yes | Not yet used by `RobotController` | Defines planned pins and constants |
| `distance_sensor` | Yes | yes | Template sensor module |
| `motor_driver` | Yes | yes | Template motor driver module |
| `fuzzy_controller` | Yes | yes | Template fuzzy controller module |

Although the sensor, motor driver, fuzzy controller, and app configuration components already exist, the currently executed firmware still only calls the minimal `RobotController` template.

## Module Overview

### `src/main.cpp`

Main ESP-IDF C++ entry point.

Responsibilities:

- Include FreeRTOS headers.
- Include `RobotController.hpp`.
- Define `extern "C" void app_main()`.
- Create the `RobotController`.
- Run the main update loop.

Current conceptual flow:

```cpp
extern "C" void app_main() {
    RobotController robot;

    robot.init();

    while (true) {
        robot.update();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

The `extern "C"` declaration is required because ESP-IDF expects the `app_main()` symbol to use C linkage, even when the implementation is written in C++.

### `components/robot_controller`

Main application coordinator.

Current responsibilities:

- Provide `RobotController::init()`.
- Provide `RobotController::update()`.
- Print firmware lifecycle logs.

Planned responsibilities:

- Initialize the distance sensor.
- Initialize the motor driver.
- Initialize the fuzzy controller.
- Read sensor data.
- Send sensor data to the fuzzy controller.
- Send fuzzy output commands to the motor driver.

### `components/app_config`

Centralized configuration component.

Current responsibilities:

- Store planned GPIO pin mapping.
- Store planned PWM/LEDC constants.
- Store base speed constants.

This module currently defines the planned pin mapping for:

- HC-SR04
- TB6612FNG
- PWM channels
- Base speed values

### `components/distance_sensor`

Template module for the HC-SR04 ultrasonic sensor.

Current responsibilities:

- Provide a `DistanceSensor` class.
- Provide an `init()` method.
- Provide a placeholder `readDistanceCm()` method.
- Return a simulated distance value.

Planned responsibilities:

- Configure TRIG and ECHO GPIOs.
- Generate the HC-SR04 trigger pulse.
- Measure the ECHO pulse duration.
- Convert pulse duration into distance in centimeters.
- Provide distance readings to the robot controller.

### `components/motor_driver`

Template module for the TB6612FNG motor driver.

Current responsibilities:

- Provide a `MotorDriver` class.
- Provide template methods:
  - `init()`
  - `enable()`
  - `disable()`
  - `drive()`
  - `stop()`

Planned responsibilities:

- Configure motor direction GPIOs.
- Configure PWM generation through ESP32 LEDC.
- Control Motor A and Motor B speed.
- Control Motor A and Motor B direction.
- Enable and disable the TB6612FNG through `STBY`.

### `components/fuzzy_controller`

Template module for fuzzy decision logic.

Current responsibilities:

- Define `FuzzyInput`.
- Define `FuzzyOutput`.
- Provide a `FuzzyController` class.
- Return a placeholder output with stopped motors.

Planned responsibilities:

- Define fuzzy input variables.
- Define fuzzy output variables.
- Define membership functions.
- Apply fuzzy inference rules.
- Convert fuzzy outputs into motor speed/direction commands.

## Planned Pin Mapping

### HC-SR04

| HC-SR04 Pin | ESP32 GPIO | Direction | Description |
|---|---:|---|---|
| `TRIG` | GPIO19 | ESP32 output | Sends the trigger pulse to start a measurement |
| `ECHO` | GPIO18 | ESP32 input | Receives the response pulse used to calculate distance |

Important electrical note:

The HC-SR04 used in this project is a 5V sensor. Its `ECHO` output may return 5V, while the ESP32 GPIOs operate at 3.3V logic level. A voltage divider or logic level shifter should be used between `ECHO` and the ESP32 input pin.

### TB6612FNG HS-166

| TB6612FNG Pin | ESP32 GPIO | Description |
|---|---:|---|
| `STBY` | GPIO13 | Enables or disables the motor driver |
| `AIN1` | GPIO26 | Motor A direction control |
| `AIN2` | GPIO27 | Motor A direction control |
| `PWMA` | GPIO25 | Motor A PWM speed control |
| `BIN1` | GPIO32 | Motor B direction control |
| `BIN2` | GPIO33 | Motor B direction control |
| `PWMB` | GPIO14 | Motor B PWM speed control |

### TB6612FNG Physical Connections

These pins are required in the physical circuit but are not directly represented as ESP32 GPIO definitions:

| TB6612FNG Pin | Connection |
|---|---|
| `VM` | External motor power supply |
| `VCC` | ESP32 3.3V logic supply |
| `GND` | Common ground between ESP32, motor driver, and motor power supply |
| `AO1` / `AO2` | Motor A terminals |
| `BO1` / `BO2` | Motor B terminals |

## Wokwi Simulation

The current Wokwi setup is minimal.

Current files:

```text
wokwi.toml
diagram.json
```

Current simulation:

- ESP32 DevKit V1.
- Serial monitor.
- No HC-SR04 component yet.
- No motor driver representation yet.
- No simulated LEDs/PWM signals yet.

The current Wokwi simulation validates only:

- Firmware boot.
- `app_main()` execution.
- `RobotController` logs.

## Build Instructions

From the repository root:

```bash
pio run
```

Expected result:

```text
SUCCESS
```

## Run in Wokwi

1. Build the firmware:

```bash
pio run
```

2. Open the VS Code command palette:

```text
Ctrl + Shift + P
```

3. Run:

```text
Wokwi: Start Simulator
```

## Upload to Physical ESP32

After connecting the ESP32 DevKit V1 through USB:

```bash
pio run -t upload
```

Open serial monitor:

```bash
pio device monitor
```

## Current Limitations

The current project does not yet:

- Read the HC-SR04 sensor.
- Generate real PWM output.
- Configure TB6612FNG GPIOs.
- Move DC motors.
- Execute fuzzy inference.
- Perform autonomous navigation.
- Simulate the full circuit in Wokwi.
- Include the physical 3D chassis files.
- Include practical test reports.
- Include robot behavior analysis.

## Project Requirements Mapping

The table below maps the assignment requirements to the current repository state.

| Assignment requirement | Current repository state |
|---|---|
| Functional mobile robot with 3D-printed structure | implemented through a remote control cart chassis |
| ESP32 as control unit | Configured as PlatformIO ESP-IDF target |
| Distance sensors for environment perception | Planned; `distance_sensor` template exists |
| Locomotion with motors and motor driver | Planned; `motor_driver` template exists |
| Fuzzy decision module | Planned; `fuzzy_controller` template exists |
| Input and output variables for the fuzzy system | Planned; initial `FuzzyInput` and `FuzzyOutput` structs exist |
| Fuzzy rules for speed and direction adjustment | Not implemented yet |
| Autonomous obstacle avoidance in controlled environment | Not implemented yet |
| Modular software organization | Started; components are being organized |
| Practical tests and behavior analysis | Not documented yet |
| Technical documentation and functional demo | Started with this README; demo not available yet |

## Suggested Development Roadmap

### Stage 1 — Base Firmware Template

Status: completed.

- Create ESP-IDF + C++ PlatformIO project.
- Configure `app_main()` in `main.cpp`.
- Create minimal `RobotController`.
- Validate build with `pio run`.
- Validate boot in Wokwi.

### Stage 2 — Component Integration

Status: in progress.

- Ensure all component `CMakeLists.txt` files are correctly configured.
- Connect `RobotController` to:
  - `DistanceSensor`
  - `MotorDriver`
  - `FuzzyController`
- Validate that all template modules compile together.

### Stage 3 — Motor Driver Implementation

Planned.

- Configure GPIOs for TB6612FNG.
- Configure ESP32 LEDC PWM.
- Implement:
  - `MotorDriver::enable()`
  - `MotorDriver::disable()`
  - `MotorDriver::drive()`
  - `MotorDriver::stop()`
- Validate signals in Wokwi before connecting physical motors.

### Stage 4 — Distance Sensor Implementation

Planned.

- Configure HC-SR04 TRIG and ECHO pins.
- Implement distance measurement.
- Validate distance readings in Wokwi.
- Validate physical wiring using a voltage divider or logic level shifter.

### Stage 5 — Basic Obstacle Avoidance

Planned.

Before implementing fuzzy logic, create a simple rule-based behavior:

| Distance condition | Robot behavior |
|---|---|
| Obstacle far | Move forward |
| Obstacle near | Reduce speed |
| Obstacle too close | Stop, turn, or reverse |

This stage validates sensor and motor integration before adding fuzzy inference.

### Stage 6 — Fuzzy Logic Controller

Planned.

Define fuzzy input variables, for example:

| Input variable | Possible linguistic values |
|---|---|
| Front distance | Near, medium, far |

Define fuzzy output variables, for example:

| Output variable | Possible linguistic values |
|---|---|
| Motor A speed | Reverse, stop, slow, medium, fast |
| Motor B speed | Reverse, stop, slow, medium, fast |

Future versions may include additional sensors and more complete outputs, such as:

- Linear speed
- Steering intensity
- Turn direction
- Left motor speed
- Right motor speed

### Stage 7 — Practical Testing

Planned.

- Test motor driver wiring.
- Test motor direction.
- Test minimum PWM needed to move the robot.
- Test HC-SR04 distance stability.
- Test obstacle avoidance in a controlled environment.
- Register logs, observations, videos, and behavior analysis.

### Stage 8 — Physical Prototype Documentation

Planned.

- Document chassis design.
- Add 3D model files or references.
- Add assembly instructions.
- Add wiring diagrams.
- Add final demonstration evidence.

## Notes for Contributors

Recommended workflow:

```bash
git checkout main
git pull

git checkout -b feature/<feature-name>

pio run

git add .
git commit -m "Describe the change"
git push -u origin feature/<feature-name>
```

Recommended branch examples:

```text
feature/component-integration
feature/motor-driver
feature/distance-sensor
feature/fuzzy-controller
feature/wokwi-circuit
docs/pinout
docs/test-plan
```

## Recommended Next Step

The next recommended development step is:

1. Fix/complete the `CMakeLists.txt` for all existing components.
2. Integrate the template modules into `RobotController`.
3. Make the runtime log show the full architecture:

```text
RobotController init
DistanceSensor init
MotorDriver init
FuzzyController init
RobotController update
DistanceSensor simulated read
FuzzyController evaluate
MotorDriver drive
```

After this, the project will be ready for real implementation of the motor driver and sensor modules.
