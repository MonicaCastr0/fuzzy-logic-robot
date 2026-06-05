#include "FuzzyController.hpp"

#include "esp_log.h"
#include "AppConfig.hpp"

static const char* TAG = "FuzzyController";

FuzzyController::FuzzyController() = default;

void FuzzyController::init() {
    ESP_LOGI(TAG, "Initializing fuzzy controller - template");
}

FuzzyOutput FuzzyController::evaluate(const FuzzyInput& input) {
    const float frontDistance = input.frontDistanceCm;
    const float rearDistance = input.rearDistanceCm;

    FuzzyOutput output{};

    ESP_LOGI(TAG, "Decision input | front distance: %.2f cm | rear distance: %.2f cm", frontDistance, rearDistance);

    if (frontDistance < 0.0f){
        ESP_LOGW(TAG, "Decision: invalid distance -> stop");
        output.motorASpeed = AppConfig::TRACTION_STOP;
        output.motorBSpeed = AppConfig::STEERING_STOP;
        
        return output;
    }

    if (frontDistance <= AppConfig::OBSTACLE_TOO_CLOSE_CM) {
        ESP_LOGI(TAG, "Decision: obstacle too close -> stop");
        output.motorASpeed = AppConfig::TRACTION_STOP;
        output.motorBSpeed = AppConfig::STEERING_STOP;
        return output;
    }

    if (frontDistance <= AppConfig::OBSTACLE_NEAR_CM) {
        if (rearDistance < 0.0f) {
            ESP_LOGW(TAG, "Decision: front obstacle near but rear distance invalid -> stop");
            output.motorASpeed = AppConfig::TRACTION_STOP;
            output.motorBSpeed = AppConfig::STEERING_STOP;
            return output;
        }

        if (rearDistance <= AppConfig::REAR_SAFE_REVERSE_CM) {
            ESP_LOGI(TAG, "Decision: front obstacle near but rear blocked -> stop");
            output.motorASpeed = AppConfig::TRACTION_STOP;
            output.motorBSpeed = AppConfig::STEERING_STOP;
            return output;
        }

        ESP_LOGI(TAG, "Decision: front obstacle near and rear clear -> reverse and turn right");
        output.motorASpeed = -AppConfig::TRACTION_SLOW;
        output.motorBSpeed = AppConfig::STEERING_RIGHT;
        return output;
    }

    if (frontDistance <= AppConfig::OBSTACLE_CAUTION_CM) {
        ESP_LOGI(TAG, "Decision: caution zone -> move slowly");
        output.motorASpeed = AppConfig::TRACTION_SLOW;
        output.motorBSpeed = AppConfig::STEERING_STOP;
        return output;
    }

    ESP_LOGI(TAG, "Decision: path clear -> move forward");
    output.motorASpeed = AppConfig::TRACTION_CRUISE;
    output.motorBSpeed = AppConfig::STEERING_STOP;

    return output;
}