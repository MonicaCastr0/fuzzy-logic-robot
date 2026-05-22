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

    if (AppConfig::MOTOR_DRIVER_TEST_MODE) {
        ESP_LOGI(TAG, "Running motor driver test mode");
        motorDriver_.drive(AppConfig::SPEED_LOW, AppConfig::SPEED_LOW);
        vTaskDelay(pdMS_TO_TICKS(2000));
        motorDriver_.drive(AppConfig::SPEED_MEDIUM, AppConfig::SPEED_MEDIUM);
        vTaskDelay(pdMS_TO_TICKS(2000));
        motorDriver_.drive(AppConfig::SPEED_HIGH, AppConfig::SPEED_HIGH);
        vTaskDelay(pdMS_TO_TICKS(2000));
        motorDriver_.stop();
        return;
    }

    ESP_LOGI(TAG, "Running robot control cycle");
    const float frontCmDistance = distanceSensor_.readDistanceCm();

    FuzzyInput input{};
    input.frontDistanceCm = frontCmDistance;

    const FuzzyOutput output = fuzzyController_.evaluate(input);

    motorDriver_.drive(output.motorASpeed, output.motorBSpeed);
}