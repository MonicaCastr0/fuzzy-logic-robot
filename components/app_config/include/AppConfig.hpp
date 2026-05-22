#pragma once

#include "driver/gpio.h"
#include "driver/ledc.h"

namespace AppConfig {

// ===============================
// HC-SR04 - distance sensor - GPIO configuration
// ===============================
static constexpr gpio_num_t TRIG_PIN = GPIO_NUM_19;
static constexpr gpio_num_t ECHO_PIN = GPIO_NUM_18;

// ===============================
// TB6612FNG - motor driver - A Channel / A Motor
// ===============================
static constexpr gpio_num_t AIN1_PIN = GPIO_NUM_26; // GPIO pin for controlling the direction of motor A (forward/reverse)
static constexpr gpio_num_t AIN2_PIN = GPIO_NUM_27; // GPIO pin for controlling the direction of motor A (forward/reverse)
static constexpr gpio_num_t PWMA_PIN = GPIO_NUM_25; // GPIO pin for controlling the speed of motor A

// ===============================
// TB6612FNG - motor driver - B Channel / B Motor
// ===============================
static constexpr gpio_num_t BIN1_PIN = GPIO_NUM_32; // GPIO pin for controlling the direction of motor B (forward/reverse)
static constexpr gpio_num_t BIN2_PIN = GPIO_NUM_33; // GPIO pin for controlling the direction of motor B (forward/reverse)
static constexpr gpio_num_t PWMB_PIN = GPIO_NUM_14; // GPIO pin for controlling the speed of motor B

// ===============================
// TB6612FNG - motor driver - Standby
// ===============================
static constexpr gpio_num_t STBY_PIN = GPIO_NUM_13;

// ===============================
// PWM - LEDC
// ===============================
static constexpr int PWM_FREQUENCY_HZ = 5000;
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
static constexpr int SPEED_LOW = 80;
static constexpr int SPEED_MEDIUM = 140;
static constexpr int SPEED_HIGH = 200;

// ===============================
// test flag for the RobotController, if true it will run a motor test
// ===============================
static constexpr bool MOTOR_DRIVER_TEST_MODE = false;

}