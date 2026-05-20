# Robo Fuzzy ESP32

Mobile robotics project based on an ESP32 microcontroller, obstacle detection, motor control, and a future fuzzy logic decision module.

This repository is currently in the **initial firmware setup stage**. The current goal is not to control the physical robot yet, but to establish a clean, modular, and reproducible development environment using:

- ESP32 DevKit V1
- ESP-IDF
- C++
- PlatformIO
- Wokwi Simulator
- Git/GitHub

---

## Current Project Status

The project currently contains a minimal ESP-IDF C++ firmware template.

The firmware successfully:

- Builds using PlatformIO.
- Boots in the Wokwi simulator.
- Enters the ESP-IDF `app_main()` entry point.
- Instantiates a `RobotController` object.
- Calls `RobotController::init()` once.
- Calls `RobotController::update()` continuously every 1 second.

Example Wokwi serial output:

```text
I (...) main_task: Calling app_main()
I (...) RobotController: RobotController init
I (...) RobotController: RobotController update
I (...) RobotController: RobotController update
I (...) RobotController: RobotController update
```

This confirms that the basic firmware structure is working.

At this stage, the project does **not** yet implement:

- Real motor control.
- Ultrasonic sensor reading.
- PWM output for the motor driver.
- Fuzzy logic behavior.
- Full Wokwi circuit simulation.
- Physical robot movement.

---

## Hardware Target

The project is being designed for the following hardware:

| Component | Purpose |
|---|---|
| ESP32 DevKit V1 | Main microcontroller |
| HC-SR04 5V | Ultrasonic distance sensor |
| TB6612FNG HS-166 | Dual DC motor driver |
| 2 DC motors | Robot locomotion |
| External motor power supply | Power source for the motors |
| Voltage divider or logic level shifter | Protection for the ESP32 input when reading HC-SR04 ECHO |
| 3D printed chassis | Physical structure of the robot |

---

## Development Environment

Recommended development setup:

| Tool | Purpose |
|---|---|
| VS Code | Code editor |
| PlatformIO | Build system and ESP32 project management |
| ESP-IDF | Native ESP32 framework |
| Wokwi | Firmware and circuit simulation |
| Git | Version control |
| GitHub | Repository hosting and collaboration |

---

## PlatformIO Board Configuration

The current target board is configured as a generic ESP32 development board:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = espidf
monitor_speed = 115200
```

The `esp32dev` board option is used because it is compatible with generic ESP32 DevKit V1 / ESP-WROOM-32 boards.

---

## Current Repository Structure

The current expected structure is:

```text
robo-fuzzy-esp32/
│
├── platformio.ini
├── CMakeLists.txt
├── sdkconfig.defaults
├── wokwi.toml
├── diagram.json
│
├── src/
│   ├── CMakeLists.txt
│   └── main.cpp
│
└── components/
    └── robot_controller/
        ├── CMakeLists.txt
        ├── RobotController.cpp
        └── include/
            └── RobotController.hpp
```

Planned structure as the project evolves:

```text
robo-fuzzy-esp32/
│
├── src/
│   └── main.cpp
│
└── components/
    ├── app_config/
    ├── distance_sensor/
    ├── motor_driver/
    ├── fuzzy_controller/
    └── robot_controller/
```

---

## Current Firmware Flow

The current firmware flow is:

```text
app_main()
   ↓
Create RobotController
   ↓
RobotController::init()
   ↓
loop:
   RobotController::update()
   delay 1000 ms
```

Current `main.cpp` behavior:

```cpp
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "RobotController.hpp"

extern "C" void app_main() {
    RobotController robot;

    robot.init();

    while (true) {
        robot.update();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

The `extern "C"` declaration is required because ESP-IDF expects the `app_main()` entry point to use C linkage, even when the application code is written in C++.

---

## Current RobotController Module

The current `RobotController` module is only a minimal template.

Header example:

```cpp
#pragma once

class RobotController {
public:
    RobotController();

    void init();
    void update();
};
```

Current behavior:

| Method | Current behavior |
|---|---|
| `init()` | Prints a log message |
| `update()` | Prints a log message every second |

No hardware control is implemented yet.

---

## Planned Firmware Modules

The firmware will be organized into the following modules:

| Module | Responsibility |
|---|---|
| `app_config` | Centralized pin mapping and configuration constants |
| `distance_sensor` | HC-SR04 trigger/echo handling and distance calculation |
| `motor_driver` | TB6612FNG GPIO/PWM motor control |
| `fuzzy_controller` | Fuzzy logic rules and decision output |
| `robot_controller` | Main coordination between sensors, fuzzy logic, and motors |

---

## Planned Pin Mapping

The following pin mapping is planned for the first hardware prototype.

### HC-SR04

| Function | ESP32 GPIO | Description |
|---|---:|---|
| TRIG | GPIO19 | Output signal from ESP32 to trigger measurement |
| ECHO | GPIO18 | Input signal from HC-SR04 to ESP32 |

Important: the HC-SR04 is a 5V sensor. The ECHO signal should not be connected directly to the ESP32 GPIO. A voltage divider or logic level shifter should be used.

### TB6612FNG Motor Driver

| TB6612FNG Pin | ESP32 GPIO | Description |
|---|---:|---|
| STBY | GPIO13 | Enables or disables the motor driver |
| AIN1 | GPIO26 | Motor A direction control |
| AIN2 | GPIO27 | Motor A direction control |
| PWMA | GPIO25 | Motor A PWM speed control |
| BIN1 | GPIO32 | Motor B direction control |
| BIN2 | GPIO33 | Motor B direction control |
| PWMB | GPIO14 | Motor B PWM speed control |

### TB6612FNG Physical Connections

These pins are not controlled directly by firmware, but are required in the physical circuit:

| TB6612FNG Pin | Connection |
|---|---|
| VM | External motor power supply |
| VCC | ESP32 3.3V logic supply |
| GND | Common ground between ESP32, motor driver, and motor power supply |
| AO1 / AO2 | Motor A terminals |
| BO1 / BO2 | Motor B terminals |

---

## Build Instructions

To build the project:

```bash
pio run
```

Expected result:

```text
SUCCESS
```

---

## Wokwi Simulation

The project includes a minimal Wokwi setup.

Current Wokwi files:

```text
wokwi.toml
diagram.json
```

To run the simulator:

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

Current simulation only validates firmware boot and serial logs.

Sensor and motor simulation will be added later.

---

## Git Recommendations

The repository should track source files and configuration files, but ignore generated build artifacts.

Recommended tracked files:

```text
platformio.ini
CMakeLists.txt
sdkconfig.defaults
wokwi.toml
diagram.json
src/
components/
README.md
.gitignore
```

Recommended ignored files:

```text
.pio/
.vscode/
build/
sdkconfig
sdkconfig.*
compile_commands.json
```

---

## Suggested Development Roadmap

### Stage 1 — Current Stage

Status: completed.

- Create PlatformIO ESP-IDF project.
- Configure C++ entry point.
- Create minimal `RobotController`.
- Validate build with `pio run`.
- Validate boot in Wokwi.

### Stage 2 — Project Structure

Next step.

- Add `app_config` component.
- Add pin mapping constants.
- Add empty template components:
  - `DistanceSensor`
  - `MotorDriver`
  - `FuzzyController`
  - `RobotController`

### Stage 3 — Motor Driver

- Configure GPIO pins for TB6612FNG.
- Configure ESP32 LEDC PWM.
- Implement:
  - `enable()`
  - `disable()`
  - `drive()`
  - `stop()`

### Stage 4 — Distance Sensor

- Configure HC-SR04 TRIG and ECHO pins.
- Implement distance measurement.
- Validate distance readings in Wokwi.
- Validate safe wiring in physical hardware.

### Stage 5 — Basic Robot Behavior

- Implement simple obstacle avoidance without fuzzy logic.
- Example behavior:
  - obstacle far: move forward
  - obstacle near: slow down
  - obstacle too close: turn or stop

### Stage 6 — Fuzzy Logic Controller

- Define fuzzy input variables.
- Define fuzzy output variables.
- Define membership functions.
- Define fuzzy rules.
- Replace the basic decision logic with fuzzy inference.

### Stage 7 — Physical Testing

- Test motor driver wiring.
- Test HC-SR04 readings.
- Test robot movement.
- Collect behavior observations.
- Adjust fuzzy rules based on practical results.

---

## Current Limitations

The current firmware does not yet:

- Read the HC-SR04 sensor.
- Generate PWM signals.
- Control the TB6612FNG motor driver.
- Move real motors.
- Implement fuzzy logic.
- Simulate the full robot circuit.
- Simulate robot physics.

The current goal is only to validate the firmware foundation and modular C++ architecture.

---

## Final Project Goal

The final goal is to build a functional autonomous mobile robot capable of detecting obstacles and adjusting its movement using fuzzy logic.

Expected final features:

- ESP32-based control unit.
- Distance sensing with HC-SR04 or additional sensors.
- DC motor control through TB6612FNG.
- Modular firmware architecture.
- Fuzzy decision module.
- Autonomous obstacle avoidance.
- Wokwi-based firmware simulation.
- Physical prototype with 3D printed chassis.
- Practical tests and behavior analysis.
