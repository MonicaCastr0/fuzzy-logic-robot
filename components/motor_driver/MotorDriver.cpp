#include "MotorDriver.hpp"

#include "esp_err.h"
#include "esp_log.h"
#include "AppConfig.hpp"

static const char* TAG = "MotorDriver";

MotorDriver::MotorDriver() = default;

void MotorDriver::init() {
    ESP_LOGI(TAG, "Initializing motor driver TB6612FNG");

    setupGpio();
    setupPwm();

    stop();

    ESP_LOGI(TAG, "Motor driver initialized successfully");
}

void MotorDriver::enable() {
    gpio_set_level(AppConfig::STBY_PIN, 1);
    ESP_LOGI(TAG, "Motor driver enabled");
}

void MotorDriver::disable() {
    stop();
    gpio_set_level(AppConfig::STBY_PIN, 0);
    ESP_LOGI(TAG, "Motor driver disabled");
}

void MotorDriver::drive(int motorASpeed, int motorBSpeed) {
    const MotorPins motorA{
        AppConfig::AIN1_PIN,
        AppConfig::AIN2_PIN,
        AppConfig::PWMA_PIN,
        AppConfig::MOTOR_A_CHANNEL
    };

    const MotorPins motorB{
        AppConfig::BIN1_PIN,
        AppConfig::BIN2_PIN,
        AppConfig::PWMB_PIN,
        AppConfig::MOTOR_B_CHANNEL
    };

    setMotor(motorA, motorASpeed);
    setMotor(motorB, motorBSpeed);

    ESP_LOGI(TAG, "Driver command applied | motor A speed: %d | motor B speed: %d", 
            motorASpeed, 
            motorBSpeed);
}

void MotorDriver::stop() {
    drive(AppConfig::SPEED_STOP, AppConfig::SPEED_STOP);
}

// private function that sets the digital pins that control the direction 
// of the motors and the standby pin as outputs
// this function DO NOT set the PWM pins, that is handled inside the setupPwm() function
void MotorDriver::setupGpio(){
    gpio_config_t outputConfig{};

    // every pin is initialized as zero by default, so we can set them all at once with a bitmask and avoid memory
    // garbage issues by not initializing each pin separately
    outputConfig.pin_bit_mask =
    (1ULL << AppConfig::AIN1_PIN) |
    (1ULL << AppConfig::AIN2_PIN) |
    (1ULL << AppConfig::BIN1_PIN) |
    (1ULL << AppConfig::BIN2_PIN) |
    (1ULL << AppConfig::STBY_PIN);

    outputConfig.mode = GPIO_MODE_OUTPUT;
    outputConfig.pull_up_en = GPIO_PULLUP_DISABLE;
    outputConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    outputConfig.intr_type = GPIO_INTR_DISABLE;

    ESP_ERROR_CHECK(gpio_config(&outputConfig));

    // now every pin is forced to be LOW, so we can be sure that the motors are stopped and in a known state when initialized
    gpio_set_level(AppConfig::AIN1_PIN, 0);
    gpio_set_level(AppConfig::AIN2_PIN, 0);
    gpio_set_level(AppConfig::BIN1_PIN, 0);
    gpio_set_level(AppConfig::BIN2_PIN, 0);
    gpio_set_level(AppConfig::STBY_PIN, 0);

    ESP_LOGI(TAG, "motor GPIO pins configured");
}
// LEDC time configuration
//   ↓
// LEDC channel configuration
//   ↓
// GPIO
void MotorDriver::setupPwm() {
    //LEDC timer configuration structure, used to configure the timer that will be used by the PWM channels for the motors
    ledc_timer_config_t timerConfig{};

    // LEDC component configuration
    timerConfig.speed_mode = AppConfig::MOTOR_PWM_MODE;
    timerConfig.timer_num = AppConfig::MOTOR_PWM_TIMER;
    timerConfig.duty_resolution = AppConfig::PWM_RESOLUTION;
    timerConfig.freq_hz = AppConfig::PWM_FREQUENCY_HZ;
    timerConfig.clk_cfg = LEDC_AUTO_CLK;

    // configure the timer with the specified configuration, this will be used by the PWM channels for the motors
    ESP_ERROR_CHECK(ledc_timer_config(&timerConfig));

    // now we can configure the PWM channels for the motors, this will set the GPIO pins for the PWM signals and associate them with the timer configured above
    setupPwmChannel(AppConfig::PWMA_PIN, AppConfig::MOTOR_A_CHANNEL);
    setupPwmChannel(AppConfig::PWMB_PIN, AppConfig::MOTOR_B_CHANNEL);

    ESP_LOGI(TAG, "PWM configured | frequency: %d Hz | max duty %d",
             AppConfig::PWM_FREQUENCY_HZ,
             AppConfig::PWM_MAX_DUTY);
}

// this function configures a single PWM channel with the specified GPIO pin and channel number, 
// it is used to configure both motor A and motor B channels
void MotorDriver::setupPwmChannel(gpio_num_t pwmPin, ledc_channel_t channel){
    ledc_channel_config_t channelConfig{};

    // LEDC channel configuration structure, used to configure the PWM channel for a motor
    channelConfig.gpio_num = pwmPin;
    channelConfig.speed_mode = AppConfig::MOTOR_PWM_MODE;
    channelConfig.channel = channel;
    channelConfig.intr_type = LEDC_INTR_DISABLE;
    channelConfig.timer_sel = AppConfig::MOTOR_PWM_TIMER;
    channelConfig.duty = 0;
    channelConfig.hpoint = 0;

    // configure the PWM channel with the specified configuration, 
    // this will set the GPIO pin for the PWM signal and associate it with the timer configured above
    ESP_ERROR_CHECK(ledc_channel_config(&channelConfig));
}

// this function sets the direction and speed of a motor based on the specified MotorPins structure and speed value, 
// it is used to control both motor A and motor B
void MotorDriver::setMotor(const MotorPins& motor, int speed) {
    const int clampedSpeed = clampSpeed(speed);

    if (clampedSpeed > 0) {
        gpio_set_level(motor.in1, 1);
        gpio_set_level(motor.in2, 0);
        setPwmDuty(motor.channel, clampedSpeed);
        return;
    }

    if (clampedSpeed < 0) {
        gpio_set_level(motor.in1, 0);
        gpio_set_level(motor.in2, 1);
        setPwmDuty(motor.channel, absoluteValue(clampedSpeed));
        return;
    }

    gpio_set_level(motor.in1, 0);
    gpio_set_level(motor.in2, 0);
    setPwmDuty(motor.channel, 0);
}

// this function sets the duty cycle of a PWM channel based on the specified channel number and duty value, 
// it is used to control the speed of the motors by setting the appropriate duty cycle for the PWM signal
void MotorDriver::setPwmDuty(ledc_channel_t channel, int duty) {
    int safeDuty = duty;

    if (safeDuty < 0) {
        safeDuty = 0;
    }

    if (safeDuty > AppConfig::PWM_MAX_DUTY) {
        safeDuty = AppConfig::PWM_MAX_DUTY;
    }

    ESP_ERROR_CHECK(ledc_set_duty(
        AppConfig::MOTOR_PWM_MODE,
        channel,
        safeDuty
    ));

    ESP_ERROR_CHECK(ledc_update_duty(
        AppConfig::MOTOR_PWM_MODE,
        channel
    ));
}

// this function clamps the speed value to be within the range of -PWM_MAX_DUTY to PWM_MAX_DUTY,
int MotorDriver::clampSpeed(int speed) const {
    if (speed > AppConfig::PWM_MAX_DUTY) {
        return AppConfig::PWM_MAX_DUTY;
    }

    if (speed < -AppConfig::PWM_MAX_DUTY) {
        return -AppConfig::PWM_MAX_DUTY;
    }

    return speed;
}

// this function returns the absolute value of an integer, 
// it is used to convert negative speed values to positive duty cycle values for the PWM signal
int MotorDriver::absoluteValue(int value) const {
    return value < 0 ? -value : value;
}