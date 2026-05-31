#include "FuzzyController.hpp"

#include "esp_log.h"
#include "AppConfig.hpp"

static const char* TAG = "FuzzyController";

FuzzyController::FuzzyController() = default;

void FuzzyController::init() {
    ESP_LOGI(TAG, "Initializing fuzzy controller - template");
}

FuzzyOutput FuzzyController::evaluate(const FuzzyInput& input) {
    const float distance = input.frontDistanceCm;
    
    FuzzyOutput output{};
    
    ESP_LOGI(TAG, "Decision input | front distance: %.2f cm", distance);

    if (distance < 0.0f){
        ESP_LOGW(TAG, "Decision: invalid distance -> stop");
        output.motorASpeed = AppConfig::TRACTION_STOP;
        output.motorBSpeed = AppConfig::STEERING_STOP;
        
        return output;
    }

    if (distance <= AppConfig::OBSTACLE_TOO_CLOSE_CM) {
        ESP_LOGI(TAG, "Decision: obstacle too close -> stop");
        output.motorASpeed = AppConfig::TRACTION_STOP;
        output.motorBSpeed = AppConfig::STEERING_STOP;
        return output;
    }

    if (distance <= AppConfig::OBSTACLE_NEAR_CM) {
        ESP_LOGI(TAG, "Decision: obstacle near -> reverse and turn right");
        output.motorASpeed = -AppConfig::TRACTION_SLOW;
        output.motorBSpeed = AppConfig::STEERING_RIGHT;
        return output;
    }

    if (distance <= AppConfig::OBSTACLE_CAUTION_CM) {
        ESP_LOGI(TAG, "Decision: caution zone -> move slowly");
        output.motorASpeed = AppConfig::TRACTION_SLOW;
        output.motorBSpeed = AppConfig::STEERING_STOP;
        return output;
    }

    output.motorASpeed = AppConfig::TRACTION_CRUISE;
    output.motorBSpeed = AppConfig::STEERING_STOP;
    ESP_LOGI(TAG, "Decision: path clear -> move forward");

    return output;
}