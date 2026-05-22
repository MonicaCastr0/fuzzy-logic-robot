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
    ESP_LOGI(TAG, "Driver enabled - template");
}

void MotorDriver::disable() {
    ESP_LOGI(TAG, "Driver disabled - template");
}

void MotorDriver::drive(int motorASpeed, int motorBSpeed) {
    ESP_LOGI(TAG, "Drive command - Motor A: %d | Motor B: %d",
             motorASpeed,
             motorBSpeed);
}

void MotorDriver::stop() {
    ESP_LOGI(TAG, "Motors stopped - template");
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

void MotorDriver::setupPwm() {
    ledc_timer_config_t timerConfig{};

    timerConfig.speed_mode = AppConfig::MOTOR_PWM_MODE;
    timerConfig.timer_num = AppConfig::MOTOR_PWM_TIMER;
    timerConfig.duty_resolution = AppConfig::PWM_RESOLUTION;
    timerConfig.freq_hz = AppConfig::PWM_FREQUENCY_HZ;
    timerConfig.clk_cfg = LEDC_AUTO_CLK;

    ESP_ERROR_CHECK(ledc_timer_config(&timerConfig));

    setupPwmChannel(AppConfig::PWMA_PIN, AppConfig::MOTOR_A_CHANNEL);
    setupPwmChannel(AppConfig::PWMB_PIN, AppConfig::MOTOR_B_CHANNEL);

    ESP_LOGI(TAG, "PWM configured | frequency: %d Hz | max duty %d",
             AppConfig::PWM_FREQUENCY_HZ,
             AppConfig::PWM_MAX_DUTY);
}

void MotorDriver::setupPwmChannel(gpio_num_t pwmPin, ledc_channel_t channel){
    ledc_channel_config_t channelConfig{};

    channelConfig.gpio_num = pwmPin;
    channelConfig.speed_mode = AppConfig::MOTOR_PWM_MODE;
    channelConfig.channel = channel;
    channelConfig.intr_type = LEDC_INTR_DISABLE;
    channelConfig.timer_sel = AppConfig::MOTOR_PWM_TIMER;
    channelConfig.duty = 0;
    channelConfig.hpoint = 0;

    ESP_ERROR_CHECK(ledc_channel_config(&channelConfig));
}

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

int MotorDriver::clampSpeed(int speed) const {
    if (speed > AppConfig::PWM_MAX_DUTY) {
        return AppConfig::PWM_MAX_DUTY;
    }

    if (speed < -AppConfig::PWM_MAX_DUTY) {
        return -AppConfig::PWM_MAX_DUTY;
    }

    return speed;
}

int MotorDriver::absoluteValue(int value) const {
    return value < 0 ? -value : value;
}