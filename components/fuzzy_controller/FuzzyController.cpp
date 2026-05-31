#include "FuzzyController.hpp"

#include "esp_log.h"
#include "AppConfig.hpp"

static const char* TAG = "FuzzyController";

FuzzyController::FuzzyController() = default;

void FuzzyController::init() {
    ESP_LOGI(TAG, "Initializing fuzzy controller - template");
}

FuzzyOutput FuzzyController::evaluate(const FuzzyInput& input) {
    FuzzyOutput output{};
    
    const float distance = input.frontDistanceCm;
    
    ESP_LOGI(TAG, "Decision input | front distance: %.2f cm", distance);

    if (distance < 0.0f){
        output.motorASpeed = AppConfig::SPEED_STOP;
        output.motorBSpeed = AppConfig::SPEED_STOP;
        ESP_LOGI(TAG, "Decision: invalid distance -> stop");
        return output;
    }

    if (distance <= 15.0f) {
        output.motorASpeed = AppConfig::SPEED_STOP;
        output.motorBSpeed = AppConfig::SPEED_STOP;
        ESP_LOGI(TAG, "Decision: obstacle too close -> stop");
        return output;
    }

    if (distance <= 30.0f) {
        output.motorASpeed = -AppConfig::SPEED_LOW;
        output.motorBSpeed = AppConfig::SPEED_LOW;
        ESP_LOGI(TAG, "Decision: obstacle near -> turn right");
        return output;
    }

    if (distance <= 70.9f) {
        output.motorASpeed = AppConfig::SPEED_LOW;
        output.motorBSpeed = AppConfig::SPEED_LOW;
        ESP_LOGI(TAG, "Decision: caution zone -> move slowly");
        return output;
    }

    output.motorASpeed = AppConfig::SPEED_MEDIUM;
    output.motorBSpeed = AppConfig::SPEED_MEDIUM;
    ESP_LOGI(TAG, "Decision: path clear -> move forward");

    return output;
}