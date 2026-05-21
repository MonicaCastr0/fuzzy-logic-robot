#include "distance_sensor/DistanceSensor.hpp"

#include "esp_log.h"
#include "AppConfig.hpp"

static const char* TAG = "DistanceSensor";

DistanceSensor::DistanceSensor() = default;

void DistanceSensor::init() {
    ESP_LOGI(TAG, "Initializing distance sensor");
    ESP_LOGI(TAG, "TRIG GPIO: %d | ECHO GPIO: %d",
             AppConfig::TRIG_PIN,
             AppConfig::ECHO_PIN);
}

float DistanceSensor::readDistanceCm() {
    ESP_LOGI(TAG, "Simulated reading from distance sensor");
    return 100.0f;
}