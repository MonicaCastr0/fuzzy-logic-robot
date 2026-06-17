#pragma once

#include "driver/gpio.h"
#include "driver/ledc.h"

namespace AppConfig {

// ===============================
// HC-SR04 - FRONT distance sensor - GPIO configuration
// ===============================
static constexpr gpio_num_t FRONT_TRIG_PIN = GPIO_NUM_19;
static constexpr gpio_num_t FRONT_ECHO_PIN = GPIO_NUM_18;

// ===============================
// HC-SR04 - REAR distance sensor - GPIO configuration
// ===============================
static constexpr gpio_num_t REAR_TRIG_PIN = GPIO_NUM_21;
static constexpr gpio_num_t REAR_ECHO_PIN = GPIO_NUM_22;

// ===============================
// TB6612FNG - motor driver - A Channel / A Motor
// ===============================
static constexpr gpio_num_t AIN1_PIN = GPIO_NUM_26; // GPIO pin for controlling the direction of motor A (forward/reverse)
static constexpr gpio_num_t AIN2_PIN = GPIO_NUM_25; // GPIO pin for controlling the direction of motor A (forward/reverse)
static constexpr gpio_num_t PWMA_PIN = GPIO_NUM_27; // GPIO pin for controlling the speed of motor A

// ===============================
// TB6612FNG - motor driver - B Channel / B Motor
// ===============================
static constexpr gpio_num_t BIN1_PIN = GPIO_NUM_32; // GPIO pin for controlling the direction of motor B (forward/reverse)
static constexpr gpio_num_t BIN2_PIN = GPIO_NUM_33; // GPIO pin for controlling the direction of motor B (forward/reverse)
static constexpr gpio_num_t PWMB_PIN = GPIO_NUM_13; // GPIO pin for controlling the speed of motor B

// ===============================
// TB6612FNG - motor driver - Standby
// ===============================
static constexpr gpio_num_t STBY_PIN = GPIO_NUM_14;

// ===============================
// PWM - LEDC
// ===============================
static constexpr int PWM_FREQUENCY_HZ = 1000;
static constexpr ledc_timer_bit_t PWM_RESOLUTION = LEDC_TIMER_8_BIT;
static constexpr int PWM_MAX_DUTY = 255;

static constexpr ledc_channel_t MOTOR_A_CHANNEL = LEDC_CHANNEL_0;
static constexpr ledc_channel_t MOTOR_B_CHANNEL = LEDC_CHANNEL_1;
static constexpr ledc_timer_t MOTOR_PWM_TIMER = LEDC_TIMER_0;
static constexpr ledc_mode_t MOTOR_PWM_MODE = LEDC_LOW_SPEED_MODE;

// ===============================
// base speeds for the motors (0-255)
// ===============================
static constexpr int SPEED_STOP = 0;
static constexpr int SPEED_LOW = 120;
static constexpr int SPEED_MEDIUM = 180;
static constexpr int SPEED_HIGH = 230;

// ===============================
// Calibrated obstacle avoidance thresholds
// ===============================
static constexpr float FRONT_TOO_CLOSE_CM = 15.0f;
static constexpr float FRONT_REVERSE_CM = 35.0f;
static constexpr float FRONT_PREVENTIVE_AVOID_CM = 70.0f;
static constexpr float FRONT_SLOWDOWN_CM = 100.0f;

// ===============================
// Rear safety threshold
// ===============================
static constexpr float REAR_SAFE_REVERSE_CM = 25.0f;

// ===============================
// Traction/steering semantic speeds
// Motor A = rear traction
// Motor B = front steering
// ===============================
static constexpr int TRACTION_STOP = SPEED_STOP;
static constexpr int TRACTION_SLOW = SPEED_LOW;
static constexpr int TRACTION_CRUISE = SPEED_HIGH;

static constexpr int TRACTION_PREVENTIVE_AVOID = 180;
static constexpr int TRACTION_REVERSE_ESCAPE = 180;

static constexpr int STEERING_STOP = SPEED_STOP;
static constexpr int STEERING_SPEED = PWM_MAX_DUTY;

static constexpr int STEERING_RIGHT = STEERING_SPEED;
static constexpr int STEERING_LEFT = -STEERING_SPEED;

// ===============================
// test flag for the RobotController, if true it will run a motor test
// ===============================
static constexpr bool MOTOR_DRIVER_TEST_MODE = false;

// ===============================
// Main control loop
// ===============================
static constexpr int ROBOT_UPDATE_INTERVAL_MS = 200;

// ===============================
// Steering pulse control
// ===============================
static constexpr int STEERING_PULSE_DURATION_MS = 1500;
static constexpr int STEERING_PULSE_COOLDOWN_MS = 600;

}