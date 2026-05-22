# Fuzzy Logic Robot

Autonomous mobile robot project based on an **ESP32 DevKit V1**, **ESP-IDF**, **C++**, **PlatformIO**, **Wokwi**, DC motors, an ultrasonic distance sensor, a TB6612FNG motor driver, and a future fuzzy logic decision module for obstacle avoidance.

The goal of this project is to build a functional autonomous mobile robot capable of detecting obstacles and adjusting its movement using fuzzy logic.

## Academic Project Requirements

The final project is expected to present:

- A functional mobile robot with a 3D-modeled and 3D-printed structure.
- ESP32 as the main control unit.
- Integration of distance sensors for environment perception.
- Locomotion control using DC motors and a motor driver.
- A fuzzy logic decision module.
- Definition of fuzzy input and output variables.
- Construction of fuzzy rules for speed and direction adjustment.
- Autonomous navigation with obstacle avoidance in a controlled environment.
- Modular software organization.
- Practical tests and behavior analysis.
- Technical documentation and a functional prototype demonstration.

## Current Project Status

The project is currently in the **firmware architecture and motor driver preparation stage**.

The following parts are already established:

- ESP-IDF project configured through PlatformIO.
- C++ firmware entry point using `extern "C" void app_main()`.
- Modular structure under `components/`.
- `RobotController` acting as the main application coordinator.
- `AppConfig` centralizing pin mapping and PWM constants.
- `DistanceSensor` component created as a template with simulated distance readings.
- `FuzzyController` component created as a template with simple input/output structures.
- `MotorDriver` component partially implemented for the TB6612FNG motor driver.
- Wokwi simulation used to validate firmware boot and serial logs.

At this moment, the firmware is not yet a complete robot control system. It already has the modular architecture needed for the final project, but some modules are still in template or partial implementation stages.

## Hardware Target

| Component | Purpose |
|---|---|
| ESP32 DevKit V1 | Main microcontroller |
| HC-SR04 5V | Ultrasonic distance sensor |
| TB6612FNG HS-166 | Dual DC motor driver |
| 2 DC motors | Robot locomotion |
| External motor power supply | Motor power source connected to TB6612FNG `VM` |
| Voltage divider or logic level shifter | Protects ESP32 GPIO when reading HC-SR04 `ECHO` |
| 3D-printed chassis | Physical robot structure |

## Development Environment

Recommended environment:

| Tool | Purpose |
|---|---|
| VS Code | Code editor |
| PlatformIO | Embedded project/build management |
| ESP-IDF | Native ESP32 framework |
| C++ | Firmware implementation language |
| Wokwi | Firmware/circuit simulation |
| Git/GitHub | Version control and collaboration |

## PlatformIO Configuration

The project currently targets a generic ESP32 DevKit-compatible board:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = espidf
monitor_speed = 115200
```

The `esp32dev` board is being used for the ESP32 DevKit V1 / ESP-WROOM-32 style board.

## Current Firmware Flow

The current application entry point is located in:

```text
src/main.cpp
```

Current flow:

```text
app_main()
   ↓
create RobotController
   ↓
RobotController::init()
   ↓
loop:
   RobotController::update()
   delay 1000 ms
```

Conceptually:

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

The `extern "C"` is required because ESP-IDF expects `app_main()` to use C linkage, even when the application is written in C++.

## Current Modular Architecture

The project is organized around ESP-IDF components:

```text
src/
  main.cpp
  CMakeLists.txt

components/
  app_config/
  distance_sensor/
  motor_driver/
  fuzzy_controller/
  robot_controller/
```

### `app_config`

Centralizes project constants such as:

- HC-SR04 pins.
- TB6612FNG pins.
- PWM frequency.
- PWM resolution.
- LEDC timer/channel configuration.
- Base motor speeds.

### `robot_controller`

Coordinates the main robot control cycle.

Current responsibility:

```text
DistanceSensor → FuzzyController → MotorDriver
```

Current `update()` flow:

```text
read front distance
   ↓
create fuzzy input
   ↓
evaluate fuzzy controller
   ↓
send motor speeds to motor driver
```

### `distance_sensor`

Template module for the HC-SR04 ultrasonic sensor.

Current status:

- `init()` logs the configured TRIG/ECHO pins.
- `readDistanceCm()` returns a simulated value.
- Real HC-SR04 timing logic is not implemented yet.

### `fuzzy_controller`

Template module for fuzzy logic decision-making.

Current status:

- Defines `FuzzyInput`.
- Defines `FuzzyOutput`.
- Receives the front distance as input.
- Currently returns stopped motor speeds.
- Real fuzzy membership functions and rules are not implemented yet.

### `motor_driver`

Module responsible for the TB6612FNG motor driver.

Current status:

- Public interface exists:
  - `init()`
  - `enable()`
  - `disable()`
  - `drive(int motorASpeed, int motorBSpeed)`
  - `stop()`
- Private implementation methods already exist for:
  - GPIO setup.
  - PWM setup.
  - PWM channel setup.
  - Per-motor speed handling.
  - PWM duty update.
  - Speed clamping.
  - Absolute value conversion.

The GPIO/PWM internal structure is being implemented, but the public methods still need to be fully connected to the private movement logic.

## Planned Pin Mapping

### HC-SR04

| Function | ESP32 GPIO | Direction | Description |
|---|---:|---|---|
| TRIG | GPIO19 | Output | ESP32 sends trigger pulse to sensor |
| ECHO | GPIO18 | Input | ESP32 receives echo pulse from sensor |

Important: the HC-SR04 is a 5V sensor. The `ECHO` pin should not be connected directly to an ESP32 GPIO. Use a voltage divider or logic level shifter.

### TB6612FNG

| TB6612FNG Pin | ESP32 GPIO | Direction | Description |
|---|---:|---|---|
| STBY | GPIO13 | Output | Enables/disables the motor driver |
| AIN1 | GPIO26 | Output | Motor A direction control |
| AIN2 | GPIO27 | Output | Motor A direction control |
| PWMA | GPIO25 | PWM output | Motor A speed control |
| BIN1 | GPIO32 | Output | Motor B direction control |
| BIN2 | GPIO33 | Output | Motor B direction control |
| PWMB | GPIO14 | PWM output | Motor B speed control |

### TB6612FNG Physical Connections

These connections are not controlled directly by firmware, but are required in the physical circuit:

| TB6612FNG Pin | Connection |
|---|---|
| VCC | ESP32 3.3V logic supply |
| VM | External motor power supply |
| GND | Common ground with ESP32 and motor power supply |
| AO1 / AO2 | Motor A terminals |
| BO1 / BO2 | Motor B terminals |

## Motor Speed Convention

The `MotorDriver` is being designed to accept speeds in the range:

```text
-255 to 255
```

Expected behavior:

| Speed value | Meaning |
|---:|---|
| `255` | Full speed in one direction |
| `140` | Medium speed in one direction |
| `80` | Low speed in one direction |
| `0` | Stop/coast |
| `-80` | Low speed in the opposite direction |
| `-255` | Full speed in the opposite direction |

Internal logic:

```text
speed > 0  → IN1 = HIGH, IN2 = LOW, PWM = speed
speed < 0  → IN1 = LOW, IN2 = HIGH, PWM = abs(speed)
speed == 0 → IN1 = LOW, IN2 = LOW, PWM = 0
```

## Build Instructions

To build the firmware:

```bash
pio run
```

Expected result:

```text
SUCCESS
```

If CMake or PlatformIO cache causes issues, remove the `.pio` directory and build again:

```powershell
Remove-Item -Recurse -Force .pio
pio run
```

## Wokwi Simulation

The project includes Wokwi configuration files:

```text
wokwi.toml
diagram.json
```

Current Wokwi usage:

1. Build the project:

```bash
pio run
```

2. Start the Wokwi simulator from VS Code:

```text
Ctrl + Shift + P
Wokwi: Start Simulator
```

Current simulation goal:

- Validate firmware boot.
- Validate ESP-IDF logs.
- Validate modular control cycle.
- Later, visualize TB6612FNG control signals using LEDs.
- Later, simulate HC-SR04 distance input.

## Current Expected Runtime Logs

The expected runtime logs should show the modular cycle:

```text
RobotController initialized successfully
Running robot control cycle
Simulated reading from distance sensor
Fuzzy input - front distance: ...
Drive command - Motor A: ... | Motor B: ...
```

As the project evolves, logs should also show:

```text
Motor GPIO pins configured
PWM configured
Motor driver initialized successfully
```

## Current Limitations

The project does not yet:

- Read the real HC-SR04 sensor.
- Use actual echo pulse timing.
- Fully apply motor commands through the public `drive()` method.
- Simulate TB6612FNG with visual Wokwi LEDs.
- Run a real fuzzy inference system.
- Control the physical robot.
- Include physical test documentation.
- Include 3D chassis documentation.

## Development Roadmap

### Stage 1 — Development Environment

Status: completed.

- Configure PlatformIO.
- Configure ESP-IDF.
- Use C++ firmware entry point.
- Validate Wokwi boot/log output.

### Stage 2 — Modular Architecture

Status: completed/in progress.

- Create component-based firmware structure.
- Add `RobotController`.
- Add `AppConfig`.
- Add `DistanceSensor`.
- Add `MotorDriver`.
- Add `FuzzyController`.
- Connect `RobotController` to the main modules.

### Stage 3 — Motor Driver

Status: in progress.

Next tasks:

- Connect `enable()` to `STBY = HIGH`.
- Connect `disable()` to stop motors and set `STBY = LOW`.
- Connect `drive()` to `setMotor()` for Motor A and Motor B.
- Connect `stop()` to `drive(0, 0)`.
- Validate PWM and direction signals in Wokwi.

### Stage 4 — Wokwi Signal Visualization

Planned.

- Add LEDs for:
  - `STBY`
  - `AIN1`
  - `AIN2`
  - `BIN1`
  - `BIN2`
  - `PWMA`
  - `PWMB`
- Use Wokwi to visually inspect motor driver control signals.

### Stage 5 — Distance Sensor

Planned.

- Configure HC-SR04 GPIOs.
- Send trigger pulse.
- Measure echo pulse duration.
- Convert duration to distance in centimeters.
- Validate readings in Wokwi.

### Stage 6 — Basic Obstacle Avoidance

Planned.

- Implement simple threshold-based behavior before fuzzy logic.
- Example:
  - Far obstacle: move forward.
  - Medium distance: slow down.
  - Very close obstacle: stop or rotate.

### Stage 7 — Fuzzy Logic

Planned.

- Define fuzzy input variables.
- Define fuzzy output variables.
- Define membership functions.
- Define fuzzy rules.
- Implement defuzzification.
- Replace basic obstacle avoidance with fuzzy decision-making.

### Stage 8 — Physical Prototype

Planned.

- Wire ESP32, HC-SR04, TB6612FNG, and motors.
- Add voltage divider/level shifter to HC-SR04 ECHO.
- Test motor driver and sensor separately.
- Assemble robot on 3D-printed chassis.

### Stage 9 — Tests and Documentation

Planned.

- Create test plan.
- Record practical results.
- Analyze robot behavior.
- Document fuzzy rules and observed behavior.
- Prepare final demonstration.

## Suggested Git Workflow

Use one branch per task:

```bash
git checkout -b feature/motor-driver
```

After changes:

```bash
pio run
git status
git add .
git commit -m "Implement motor driver GPIO and PWM setup"
git push -u origin feature/motor-driver
```

Recommended branch examples:

```text
feature/app-config
feature/consolidate-modular-structure
feature/motor-driver
feature/wokwi-motor-signals
feature/distance-sensor
feature/basic-obstacle-avoidance
feature/fuzzy-controller
docs/pinout
docs/test-plan
```

## Final Project Goal

The final goal is to deliver a functional autonomous mobile robot with:

- ESP32-based firmware.
- Modular C++ architecture.
- HC-SR04-based distance sensing.
- TB6612FNG-based DC motor control.
- Fuzzy logic decision-making.
- Autonomous obstacle avoidance.
- 3D-printed structure.
- Practical tests and technical documentation.
