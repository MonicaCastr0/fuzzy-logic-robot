#include "RobotController.hpp"
#include "AppConfig.hpp"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"

static const char* TAG = "RobotController";


RobotController::RobotController() = default;

void RobotController::init() {
    distanceSensor_.init();
    motorDriver_.init();
    fuzzyController_.init();

    motorDriver_.enable();

    ESP_LOGI(TAG, "RobotController initialized successfully");
}

void RobotController::update() {

    // running the motor in test mode with different speeds to verify that the motor driver is 
    // working correctly without needing to rely on the distance sensor and fuzzy controller logic
    if (AppConfig::MOTOR_DRIVER_TEST_MODE) {
        ESP_LOGI(TAG, "Running motor driver test mode"); // here, STDBY, AIN1, PWMA, BIN1 AND PWMB should be sending signals
        motorDriver_.drive(AppConfig::SPEED_LOW, AppConfig::SPEED_LOW);
        vTaskDelay(pdMS_TO_TICKS(2000));
        motorDriver_.drive(AppConfig::SPEED_MEDIUM, AppConfig::SPEED_MEDIUM);
        vTaskDelay(pdMS_TO_TICKS(2000));
        motorDriver_.drive(AppConfig::SPEED_HIGH, AppConfig::SPEED_HIGH);
        vTaskDelay(pdMS_TO_TICKS(2000));
        ESP_LOGI(TAG, "Testing reverse");
        motorDriver_.drive(-(AppConfig::SPEED_LOW), -(AppConfig::SPEED_LOW)); // reverse, here, AIN2 and BIN2 should be sending signals instead of AIN1 and BIN1
        vTaskDelay(pdMS_TO_TICKS(2000));
        ESP_LOGI(TAG, "Testing right turn");
        motorDriver_.drive(-(AppConfig::SPEED_LOW), (AppConfig::SPEED_LOW)); // turn right, here, BIN1 should be sending signal but not AIN1, and AIN2 should be sending signal but not BIN2
        vTaskDelay(pdMS_TO_TICKS(2000));
        ESP_LOGI(TAG, "Testing left turn");
        motorDriver_.drive((AppConfig::SPEED_LOW), -(AppConfig::SPEED_LOW)); // turn left, here, AIN1 should be sending signal but not BIN1, and BIN2 should be sending signal but not AIN2
        vTaskDelay(pdMS_TO_TICKS(2000));
        ESP_LOGI(TAG, "Motor driver test completed, stopping motors");
        motorDriver_.stop(); // here, all signals should stop, only STDBY should remain HIGH
        vTaskDelay(pdMS_TO_TICKS(2000));
        ESP_LOGI(TAG, "Disabling motor driver after test");
        motorDriver_.disable(); // here, all signals should stop, including STDBY, the motors should be completely disabled
        vTaskDelay(pdMS_TO_TICKS(2000));
        motorDriver_.enable(); // re-enable the motor driver, only STDBY should be high again
        vTaskDelay(pdMS_TO_TICKS(2000));
        return;
    }

    ESP_LOGI(TAG, "Running robot control cycle");
    const float frontCmDistance = distanceSensor_.readDistanceCm();

    FuzzyInput input{};
    input.frontDistanceCm = frontCmDistance;

    const FuzzyOutput output = fuzzyController_.evaluate(input);

    motorDriver_.drive(output.motorASpeed, output.motorBSpeed);
}