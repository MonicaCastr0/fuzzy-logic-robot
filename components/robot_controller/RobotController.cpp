#include "RobotController.hpp"
#include "AppConfig.hpp"

#include "esp_log.h"

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
    ESP_LOGI(TAG, "Running robot control cycle");
    const float frontCmDistance = distanceSensor_.readDistanceCm();

    FuzzyInput input{};
    input.frontDistanceCm = frontCmDistance;

    const FuzzyOutput output = fuzzyController_.evaluate(input);

    motorDriver_.drive(output.motorASpeed, output.motorBSpeed);
}