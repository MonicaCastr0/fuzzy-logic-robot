#include "MotorDriver.hpp"

#include "esp_log.h"
#include "AppConfig.hpp"

static const char* TAG = "MotorDriver";

MotorDriver::MotorDriver() = default;

void MotorDriver::init() {
    ESP_LOGI(TAG, "Initializing motor driver TB6612FNG");

    ESP_LOGI(TAG, "A Motor: AIN1=%d | AIN2=%d | PWMA=%d",
             AppConfig::AIN1_PIN,
             AppConfig::AIN2_PIN,
             AppConfig::PWMA_PIN);

    ESP_LOGI(TAG, "B Motor: BIN1=%d | BIN2=%d | PWMB=%d",
             AppConfig::BIN1_PIN,
             AppConfig::BIN2_PIN,
             AppConfig::PWMB_PIN);

    ESP_LOGI(TAG, "STBY GPIO: %d", AppConfig::STBY_PIN);
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