#include "FuzzyController.hpp"

#include "esp_log.h"
#include "AppConfig.hpp"

static const char* TAG = "FuzzyController";

FuzzyController::FuzzyController() = default;

void FuzzyController::init() {
    ESP_LOGI(TAG, "Initializing fuzzy controller - template");
}

FuzzyOutput FuzzyController::evaluate(const FuzzyInput& input) {
    ESP_LOGI(TAG, "Fuzzy input - front distance: %.2f cm",
             input.frontDistanceCm);

    FuzzyOutput output{};
    output.motorASpeed = AppConfig::SPEED_STOP;
    output.motorBSpeed = AppConfig::SPEED_STOP;

    return output;
}