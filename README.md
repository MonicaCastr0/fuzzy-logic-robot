# Fuzzy Logic Robot

Autonomous mobile robot project based on an **ESP32 DevKit V1**, **ESP-IDF**, **C++**, **PlatformIO**, **Wokwi**, DC motors, an **HC-SR04 ultrasonic distance sensor**, a **TB6612FNG HS-166 motor driver**, and a fuzzy logic decision module for obstacle avoidance.

The final goal is to deliver a functional autonomous robot capable of perceiving obstacles and adjusting movement using fuzzy logic.

---

## Academic Project Requirements

The final project is expected to present:

- A functional mobile robot with a 3D-modeled and 3D-printed structure.
- ESP32 as the main control unit.
- Integration of distance sensors for environment perception.
- Locomotion control using motors and a motor driver.
- Implementation of a fuzzy logic decision module.
- Definition of fuzzy input and output variables.
- Construction of fuzzy rules for speed and direction adjustment.
- Autonomous navigation with obstacle avoidance in a controlled environment.
- Modular software organization.
- Practical tests and behavior analysis.
- Technical documentation and a functional prototype demonstration.

---

## Current Project Status

The project is currently in the **firmware validation and hardware bench-test preparation stage**.

The current repository already contains:

- ESP-IDF project configured through PlatformIO.
- C++ firmware entry point using `extern "C" void app_main()`.
- Modular component-based firmware structure under `components/`.
- `RobotController` coordinating the main control flow.
- `AppConfig` centralizing pin mapping, PWM constants, base speeds, and test flags.
- `MotorDriver` implemented to control the TB6612FNG through GPIO and LEDC PWM.
- Wokwi LEDs representing the TB6612FNG control signals.
- `DistanceSensor` implemented for HC-SR04 trigger/echo reading.
- `FuzzyController` present as a template; real fuzzy rules are not implemented yet.
- A motor driver test mode that validates forward, reverse, turning, stop, disable, and re-enable behavior.

The project does not yet implement autonomous obstacle avoidance or fuzzy inference. The next development phase is to validate the physical components and then implement a simple obstacle-avoidance baseline before adding fuzzy logic.

---

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

---

## Development Environment

Recommended environment:

| Tool | Purpose |
|---|---|
| VS Code | Code editor |
| PlatformIO | Embedded project/build management |
| ESP-IDF | Native ESP32 framework |
| C++ | Firmware implementation language |
| Wokwi | Firmware and circuit simulation |
| Git/GitHub | Version control and collaboration |

---

## PlatformIO Configuration

The project targets a generic ESP32 DevKit-compatible board:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = espidf
monitor_speed = 115200
```

The `esp32dev` board is being used for the ESP32 DevKit V1 / ESP-WROOM-32 style board.

---

## Current Repository Structure

```text
fuzzy-logic-robot/
│
├── CMakeLists.txt
├── platformio.ini
├── wokwi.toml
├── diagram.json
├── README.md
│
├── include/
├── lib/
├── test/
│
├── src/
│   ├── CMakeLists.txt
│   └── main.cpp
│
└── components/
    ├── app_config/
    │   ├── CMakeLists.txt
    │   └── include/
    │       └── AppConfig.hpp
    │
    ├── distance_sensor/
    │   ├── CMakeLists.txt
    │   ├── DistanceSensor.cpp
    │   └── include/
    │       └── DistanceSensor.hpp
    │
    ├── fuzzy_controller/
    │   ├── CMakeLists.txt
    │   ├── FuzzyController.cpp
    │   └── include/
    │       └── FuzzyController.hpp
    │
    ├── motor_driver/
    │   ├── CMakeLists.txt
    │   ├── MotorDriver.cpp
    │   └── include/
    │       └── MotorDriver.hpp
    │
    └── robot_controller/
        ├── CMakeLists.txt
        ├── RobotController.cpp
        └── include/
            └── RobotController.hpp
```

---

## Current Firmware Flow

The application entry point is located in:

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

The `extern "C"` declaration is required because ESP-IDF expects `app_main()` to use C linkage, even when the firmware is written in C++.

---

## Modular Architecture

### `app_config`

Centralizes project constants:

- HC-SR04 pin mapping.
- TB6612FNG pin mapping.
- PWM frequency and resolution.
- LEDC timer/channel configuration.
- Base motor speed values.
- Motor driver test mode flag.

Current pins:

```text
HC-SR04:
TRIG → GPIO19
ECHO → GPIO18

TB6612FNG:
STBY → GPIO13
AIN1 → GPIO26
AIN2 → GPIO27
PWMA → GPIO25
BIN1 → GPIO32
BIN2 → GPIO33
PWMB → GPIO14
```

### `robot_controller`

Coordinates the robot control cycle.

Normal flow:

```text
DistanceSensor → FuzzyController → MotorDriver
```

When `AppConfig::MOTOR_DRIVER_TEST_MODE` is enabled, the controller ignores the sensor/fuzzy path and runs a predefined motor test sequence.

### `distance_sensor`

Responsible for HC-SR04 distance reading.

Current behavior:

- Configures `TRIG` as output.
- Configures `ECHO` as input.
- Sends the trigger pulse.
- Measures the echo pulse duration with ESP-IDF timing utilities.
- Converts the pulse duration to distance in centimeters.
- Returns an invalid/safe value on timeout.

Expected distance conversion:

```text
distance_cm = echo_pulse_duration_us / 58.0
```

Important physical note: the HC-SR04 `ECHO` signal is 5V. It must be reduced before entering the ESP32 GPIO.

### `fuzzy_controller`

Template module for fuzzy logic decision-making.

Current status:

- Defines `FuzzyInput`.
- Defines `FuzzyOutput`.
- Receives the front distance as input.
- Currently returns stopped motor speeds.
- Real fuzzy membership functions, inference rules, and defuzzification are not implemented yet.

### `motor_driver`

Responsible for controlling the TB6612FNG motor driver.

Public interface:

```cpp
void init();
void enable();
void disable();
void drive(int motorASpeed, int motorBSpeed);
void stop();
```

Internal responsibilities:

- Configure motor direction GPIOs.
- Configure `STBY`.
- Configure LEDC PWM timer.
- Configure LEDC PWM channels.
- Clamp motor speed values.
- Convert negative speeds to absolute PWM duty.
- Apply motor direction based on speed sign.
- Update PWM duty cycle.

The motor driver accepts speed values in the range:

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

---

## Motor Driver Test Mode

The firmware includes a test mode controlled by:

```cpp
AppConfig::MOTOR_DRIVER_TEST_MODE
```

When enabled, `RobotController::update()` runs a predefined sequence to validate the TB6612FNG control signals without relying on the distance sensor or fuzzy logic.

Current test sequence:

```cpp
if (AppConfig::MOTOR_DRIVER_TEST_MODE) {
    ESP_LOGI(TAG, "Running motor driver test mode");

    motorDriver_.drive(AppConfig::SPEED_LOW, AppConfig::SPEED_LOW);
    vTaskDelay(pdMS_TO_TICKS(2000));

    motorDriver_.drive(AppConfig::SPEED_MEDIUM, AppConfig::SPEED_MEDIUM);
    vTaskDelay(pdMS_TO_TICKS(2000));

    motorDriver_.drive(AppConfig::SPEED_HIGH, AppConfig::SPEED_HIGH);
    vTaskDelay(pdMS_TO_TICKS(2000));

    ESP_LOGI(TAG, "Testing reverse");
    motorDriver_.drive(-(AppConfig::SPEED_LOW), -(AppConfig::SPEED_LOW));
    vTaskDelay(pdMS_TO_TICKS(2000));

    ESP_LOGI(TAG, "Testing right turn");
    motorDriver_.drive(-(AppConfig::SPEED_LOW), (AppConfig::SPEED_LOW));
    vTaskDelay(pdMS_TO_TICKS(2000));

    ESP_LOGI(TAG, "Testing left turn");
    motorDriver_.drive((AppConfig::SPEED_LOW), -(AppConfig::SPEED_LOW));
    vTaskDelay(pdMS_TO_TICKS(2000));

    ESP_LOGI(TAG, "Motor driver test completed, stopping motors");
    motorDriver_.stop();
    vTaskDelay(pdMS_TO_TICKS(2000));

    ESP_LOGI(TAG, "Disabling motor driver after test");
    motorDriver_.disable();
    vTaskDelay(pdMS_TO_TICKS(2000));

    motorDriver_.enable();
    vTaskDelay(pdMS_TO_TICKS(2000));

    return;
}
```

Expected signal behavior during the test:

| Test step | Expected signal behavior |
|---|---|
| Forward low/medium/high | `STBY`, `AIN1`, `PWMA`, `BIN1`, and `PWMB` active. `AIN2` and `BIN2` inactive. PWM brightness increases by speed level. |
| Reverse | `AIN2` and `BIN2` active instead of `AIN1` and `BIN1`. `PWMA` and `PWMB` active. |
| Right turn | Motor A reverse and Motor B forward: `AIN2`, `PWMA`, `BIN1`, `PWMB` active. |
| Left turn | Motor A forward and Motor B reverse: `AIN1`, `PWMA`, `BIN2`, `PWMB` active. |
| Stop | Direction and PWM signals stop. `STBY` remains HIGH. |
| Disable | All motor signals stop, including `STBY`. |
| Re-enable | Only `STBY` should become HIGH again. |

This test mode is intended for Wokwi LED visualization and physical bench validation with low PWM values.

---

## Wokwi Simulation

The project includes Wokwi configuration files:

```text
wokwi.toml
diagram.json
```

The current Wokwi setup includes:

- ESP32 DevKit V1.
- Serial monitor.
- LEDs representing the TB6612FNG control signals.
- HC-SR04 simulation support, depending on the current `diagram.json`.

Current LED mapping:

| Signal | ESP32 GPIO | Wokwi representation |
|---|---:|---|
| `STBY` | GPIO13 | LED |
| `AIN1` | GPIO26 | LED |
| `AIN2` | GPIO27 | LED |
| `PWMA` | GPIO25 | LED with PWM brightness |
| `BIN1` | GPIO32 | LED |
| `BIN2` | GPIO33 | LED |
| `PWMB` | GPIO14 | LED with PWM brightness |

These LEDs do not simulate motor physics. They only show the logical signals that the ESP32 would send to the TB6612FNG.

---

## Compilation

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

---

## Simulation

After compiling, start the Wokwi simulation from VS Code:

```text
Ctrl + Shift + P
Wokwi: Start Simulator
```

The simulation should open the ESP32 circuit and show serial logs in the Wokwi terminal.

Current simulation goals:

- Validate firmware boot.
- Validate ESP-IDF logs.
- Validate modular control cycle.
- Visualize TB6612FNG control signals using LEDs.
- Validate the motor driver test sequence.

---

## Uploading the Firmware to the ESP32

After the project builds successfully, connect the ESP32 DevKit V1 to the computer using a USB cable.

Upload the firmware with:

```bash
pio run -t upload
```

PlatformIO will try to detect the upload port automatically.

If automatic detection fails, list available devices:

```bash
pio device list
```

Then specify the upload port manually.

Example on Windows:

```bash
pio run -t upload --upload-port COM3
```

Replace `COM3` with the port shown on your machine.

After uploading, open the serial monitor:

```bash
pio device monitor
```

The project is configured with:

```ini
monitor_speed = 115200
```

If needed, specify the port manually:

```bash
pio device monitor --port COM3 --baud 115200
```

### Common ESP32 upload notes

Some ESP32 DevKit V1 boards enter bootloader mode automatically. Others may require holding the **BOOT** button during the upload process.

Recommended upload procedure if upload fails:

```text
1. Start: pio run -t upload
2. When PlatformIO shows "Connecting..."
3. Hold BOOT on the ESP32
4. Release BOOT after upload starts
```

After upload, reset the board if the application does not start automatically.

---

## Physical Wiring Notes

### HC-SR04

| HC-SR04 Pin | Connection |
|---|---|
| VCC | ESP32 5V / VIN |
| GND | ESP32 GND |
| TRIG | GPIO19 |
| ECHO | Voltage divider / level shifter → GPIO18 |

Important: the HC-SR04 is a 5V sensor. The `ECHO` pin should not be connected directly to an ESP32 GPIO. Use a voltage divider or logic level shifter.

### TB6612FNG

| TB6612FNG Pin | ESP32 / Circuit Connection |
|---|---|
| VCC | ESP32 3.3V logic supply |
| VM | External motor power supply |
| GND | Common ground with ESP32 and motor power supply |
| STBY | GPIO13 |
| AIN1 | GPIO26 |
| AIN2 | GPIO27 |
| PWMA | GPIO25 |
| BIN1 | GPIO32 |
| BIN2 | GPIO33 |
| PWMB | GPIO14 |
| AO1 / AO2 | Motor A terminals |
| BO1 / BO2 | Motor B terminals |

Critical rule:

```text
ESP32 GND, TB6612FNG GND, and motor power supply GND must be connected together.
```

Do not power the motors from the ESP32 `3V3` pin.

---

## Physical Bench-Test Order

Before assembling the full robot, test the hardware incrementally:

```text
1. ESP32 alone
2. ESP32 + serial monitor
3. ESP32 + HC-SR04 with voltage divider on ECHO
4. ESP32 + TB6612FNG without motors
5. ESP32 + TB6612FNG + one motor
6. ESP32 + TB6612FNG + two motors
```

Start motor tests with low PWM values to reduce electrical and mechanical risk.

---

## Current Expected Runtime Logs

When motor test mode is enabled, expected logs include:

```text
RobotController initialized successfully
Running motor driver test mode
Driver command applied | motor A speed: 80 | motor B speed: 80
Driver command applied | motor A speed: 140 | motor B speed: 140
Driver command applied | motor A speed: 200 | motor B speed: 200
Testing reverse
Testing right turn
Testing left turn
Motor driver test completed, stopping motors
Disabling motor driver after test
```

When the normal control loop is active, expected logs include:

```text
Running robot control cycle
Distance read: ... cm | pulse: ... us
Fuzzy input - front distance: ...
Driver command applied | motor A speed: ... | motor B speed: ...
```

---

## Current Limitations

The project does not yet fully provide:

- Real fuzzy inference.
- Final autonomous obstacle avoidance behavior.
- Physical robot validation.
- 3D chassis documentation.
- Formal test plan and test results documentation.

The current firmware is a strong foundation for the final robot, but it still needs behavior logic, fuzzy control, physical testing, and final documentation.

---

## Development Roadmap

### Stage 1 — Development Environment

Status: completed.

- Configure PlatformIO.
- Configure ESP-IDF.
- Use C++ firmware entry point.
- Validate Wokwi boot/log output.

### Stage 2 — Modular Architecture

Status: completed.

- Create component-based firmware structure.
- Add `RobotController`.
- Add `AppConfig`.
- Add `DistanceSensor`.
- Add `MotorDriver`.
- Add `FuzzyController`.
- Connect `RobotController` to the main modules.

### Stage 3 — Motor Driver and Wokwi LEDs

Status: completed / validation in progress.

- Create TB6612FNG motor driver abstraction.
- Configure GPIO/PWM internals.
- Add Wokwi LEDs for motor driver control signals.
- Use motor driver test mode to validate direction and PWM signals.

### Stage 4 — Distance Sensor

Status: completed / validation in progress.

- Configure HC-SR04 GPIOs.
- Send trigger pulse.
- Measure echo pulse duration.
- Convert duration to distance in centimeters.
- Validate readings in Wokwi and then physically.

### Stage 5 — Physical Bench Testing

Status: next practical step.

- Upload firmware to ESP32.
- Validate logs through serial monitor.
- Test ESP32 with no external components.
- Test HC-SR04 with voltage divider.
- Test TB6612FNG without motors.
- Test motors with low PWM values.

### Stage 6 — Basic Obstacle Avoidance

Status: planned.

- Implement simple threshold-based behavior before fuzzy logic.
- Example:
  - Far obstacle: move forward.
  - Medium distance: slow down.
  - Very close obstacle: stop or rotate.

### Stage 7 — Fuzzy Logic

Status: planned.

- Define fuzzy input variables.
- Define fuzzy output variables.
- Define membership functions.
- Define fuzzy rules.
- Implement defuzzification.
- Replace basic obstacle avoidance with fuzzy decision-making.

### Stage 8 — Physical Prototype

Status: planned.

- Wire ESP32, HC-SR04, TB6612FNG, and motors.
- Add voltage divider or level shifter to HC-SR04 `ECHO`.
- Test motor driver and sensor separately.
- Assemble robot on 3D-printed chassis.

### Stage 9 — Tests and Documentation

Status: planned.

- Create test plan.
- Record practical results.
- Analyze robot behavior.
- Document fuzzy rules and observed behavior.
- Prepare final demonstration.

---

## Suggested Git Workflow

Use one branch per task:

```bash
git checkout -b feature/basic-obstacle-avoidance
```

After changes:

```bash
pio run
git status
git add .
git commit -m "Implement basic obstacle avoidance"
git push -u origin feature/basic-obstacle-avoidance
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

---

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
