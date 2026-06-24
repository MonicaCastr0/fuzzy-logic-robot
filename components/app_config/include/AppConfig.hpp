#pragma once

#include "driver/gpio.h"
#include "driver/ledc.h"

namespace AppConfig
{

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
    // Motor A = rear traction motor
    // ===============================
    static constexpr gpio_num_t AIN1_PIN = GPIO_NUM_26;
    static constexpr gpio_num_t AIN2_PIN = GPIO_NUM_25;
    static constexpr gpio_num_t PWMA_PIN = GPIO_NUM_27;

    // ===============================
    // TB6612FNG - motor driver - B Channel / B Motor
    // Motor B = front steering motor
    // ===============================
    static constexpr gpio_num_t BIN1_PIN = GPIO_NUM_32;
    static constexpr gpio_num_t BIN2_PIN = GPIO_NUM_33;
    static constexpr gpio_num_t PWMB_PIN = GPIO_NUM_13;

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
    // Base speeds for manual tests
    // ===============================
    static constexpr int SPEED_STOP = 0;
    static constexpr int SPEED_LOW = 145;
    static constexpr int SPEED_MEDIUM = 180;
    static constexpr int SPEED_HIGH = 230;

    // ===============================
    // Traction output limits
    // ===============================
    // These values define the normal navigation range after the start boost.
    // Do not use 255 as normal cruise speed, because it makes the robot too fast.
    static constexpr int TRACTION_FORWARD_MAX_OUTPUT = 170;
    static constexpr int TRACTION_REVERSE_MAX_OUTPUT = 180;

    // ===============================
    // Traction minimum operational output
    // ===============================
    // These values prevent weak fuzzy outputs from being too low to move the robot.
    // They are not start-boost values.
    static constexpr int TRACTION_MIN_FORWARD_OUTPUT = 120;
    static constexpr int TRACTION_MIN_REVERSE_OUTPUT = 140;

    // ===============================
    // Traction start boost
    // ===============================
    // The robot receives a short 255 PWM pulse only when leaving rest.
    // After that, it returns to the fuzzy target speed.
    static constexpr bool TRACTION_START_BOOST_ENABLED = true;
    static constexpr int TRACTION_START_BOOST_DUTY = PWM_MAX_DUTY;
    static constexpr int TRACTION_START_BOOST_DURATION_MS = 90;

    // ===============================
    // Traction ramp profile
    // ===============================
    // Acceleration should be slower than deceleration.
    // This helps the robot avoid obstacles without carrying too much speed.
    static constexpr int TRACTION_ACCEL_STEP = 15;
    static constexpr int TRACTION_DECEL_STEP = 45;

    // ===============================
    // Boost and emergency safety distances
    // ===============================
    // Forward boost is only allowed if the front is reasonably free.
    // Reverse boost is allowed when escaping.
    static constexpr float TRACTION_BOOST_MIN_FRONT_DISTANCE_CM = 55.0f;
    static constexpr float TRACTION_EMERGENCY_STOP_DISTANCE_CM = 18.0f;

    // ===============================
    // Calibrated obstacle avoidance thresholds
    // ===============================
    static constexpr float FRONT_TOO_CLOSE_CM = 15.0f;
    static constexpr float FRONT_REVERSE_CM = 35.0f;
    static constexpr float FRONT_PREVENTIVE_AVOID_CM = 70.0f;
    static constexpr float FRONT_SLOWDOWN_CM = 90.0f;

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

    static constexpr int TRACTION_SLOW = 120;
    static constexpr int TRACTION_CRUISE = 160;

    static constexpr int TRACTION_PREVENTIVE_AVOID = 135;
    static constexpr int TRACTION_REVERSE_ESCAPE = 160;

    static constexpr int STEERING_STOP = SPEED_STOP;
    static constexpr int STEERING_SPEED = PWM_MAX_DUTY;

    static constexpr int STEERING_RIGHT = STEERING_SPEED;
    static constexpr int STEERING_LEFT = -STEERING_SPEED;

    // ===============================
    // Test flag for the RobotController
    // ===============================
    static constexpr bool MOTOR_DRIVER_TEST_MODE = false;

    // ===============================
    // Main control loop
    // ===============================
    // Important: the boost duration depends on the update frequency.
    // Use 100 ms or less for better behavior.
    static constexpr int ROBOT_UPDATE_INTERVAL_MS = 100;

    // ===============================
    // Steering pulse control
    // ===============================
    static constexpr int STEERING_PULSE_DURATION_MS = 1200;
    static constexpr int STEERING_PULSE_COOLDOWN_MS = 500;

}