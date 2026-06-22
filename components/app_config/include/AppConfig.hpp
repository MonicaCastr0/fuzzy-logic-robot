#pragma once

#include "driver/gpio.h"
#include "driver/ledc.h"

namespace AppConfig
{
    // ===============================
    // HC-SR04 - FRONT
    // ===============================
    static constexpr gpio_num_t FRONT_TRIG_PIN = GPIO_NUM_19;
    static constexpr gpio_num_t FRONT_ECHO_PIN = GPIO_NUM_18;

    // ===============================
    // HC-SR04 - REAR
    // ===============================
    static constexpr gpio_num_t REAR_TRIG_PIN = GPIO_NUM_21;
    static constexpr gpio_num_t REAR_ECHO_PIN = GPIO_NUM_22;

    // ===============================
    // TB6612FNG
    // ===============================
    static constexpr gpio_num_t AIN1_PIN = GPIO_NUM_26;
    static constexpr gpio_num_t AIN2_PIN = GPIO_NUM_25;
    static constexpr gpio_num_t PWMA_PIN = GPIO_NUM_27;

    static constexpr gpio_num_t BIN1_PIN = GPIO_NUM_32;
    static constexpr gpio_num_t BIN2_PIN = GPIO_NUM_33;
    static constexpr gpio_num_t PWMB_PIN = GPIO_NUM_13;

    static constexpr gpio_num_t STBY_PIN = GPIO_NUM_14;

    // ===============================
    // PWM
    // ===============================
    static constexpr int PWM_FREQUENCY_HZ = 1000;
    static constexpr ledc_timer_bit_t PWM_RESOLUTION = LEDC_TIMER_8_BIT;
    static constexpr int PWM_MAX_DUTY = 255;

    static constexpr ledc_channel_t MOTOR_A_CHANNEL = LEDC_CHANNEL_0;
    static constexpr ledc_channel_t MOTOR_B_CHANNEL = LEDC_CHANNEL_1;
    static constexpr ledc_timer_t MOTOR_PWM_TIMER = LEDC_TIMER_0;
    static constexpr ledc_mode_t MOTOR_PWM_MODE = LEDC_LOW_SPEED_MODE;

    // ===============================
    // SPEEDS
    // ===============================
    static constexpr int SPEED_STOP = 0;
    static constexpr int SPEED_LOW = 120;
    static constexpr int SPEED_MEDIUM = 160;
    static constexpr int SPEED_HIGH = 180;

    // ===============================
    // CALIBRATED THRESHOLDS
    // ===============================
    static constexpr float FRONT_TOO_CLOSE_CM = 10.0f;
    static constexpr float FRONT_REVERSE_CM = 15.0f;

    // 🔥 HISTERese adicionada (IMPORTANTE)
    static constexpr float FRONT_PREVENTIVE_AVOID_CM = 60.0f;
    static constexpr float FRONT_PREVENTIVE_EXIT_CM = 75.0f;

    static constexpr float FRONT_SLOWDOWN_CM = 90.0f;
    static constexpr float REAR_SAFE_REVERSE_CM = 25.0f;

    // ===============================
    // TRACTION / STEERING
    // ===============================
    static constexpr int TRACTION_STOP = SPEED_STOP;
    static constexpr int TRACTION_SLOW = SPEED_LOW;
    static constexpr int TRACTION_CRUISE = SPEED_HIGH;

    static constexpr int TRACTION_PREVENTIVE_AVOID = 130;
    static constexpr int TRACTION_REVERSE_ESCAPE = 220;

    static constexpr int STEERING_STOP = SPEED_STOP;
    static constexpr int STEERING_SPEED = PWM_MAX_DUTY;

    static constexpr int STEERING_RIGHT = STEERING_SPEED;
    static constexpr int STEERING_LEFT = -STEERING_SPEED;

    // ===============================
    // LOOP CONTROL (ajustado)
    // ===============================
    static constexpr int ROBOT_UPDATE_INTERVAL_MS = 100;

    // ===============================
    // STEERING CONTROL (IMPORTANTE)
    // ===============================
    static constexpr int STEERING_PULSE_DURATION_MS = 1000;
    static constexpr int STEERING_PULSE_COOLDOWN_MS = 600;

    // novo: manutenção mínima de curva
    static constexpr int STEERING_HOLD_MIN_MS = 800;

    // ===============================
    // TEST MODE
    // ===============================
    static constexpr bool MOTOR_DRIVER_TEST_MODE = false;
}