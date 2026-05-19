#include "RobotController.hpp"

#include "esp_log.h"

static const char* TAG = "RobotController";

RobotController::RobotController() = default;

void RobotController::init() {
    ESP_LOGI(TAG, "RobotController init");
}

void RobotController::update() {
    ESP_LOGI(TAG, "RobotController update");
}