#include "FuzzyController.hpp"

#include "esp_log.h"
#include "AppConfig.hpp"

static const char* TAG = "FuzzyController";

FuzzyController::FuzzyController() = default;

void FuzzyController::init() {
    ESP_LOGI(TAG, "Inicializando controlador fuzzy - template");
}

FuzzyOutput FuzzyController::evaluate(const FuzzyInput& input) {
    ESP_LOGI(TAG, "Entrada fuzzy - distancia frontal: %.2f cm",
             input.frontDistanceCm);

    FuzzyOutput output{};
    output.motorASpeed = AppConfig::SPEED_STOP;
    output.motorBSpeed = AppConfig::SPEED_STOP;

    return output;
}