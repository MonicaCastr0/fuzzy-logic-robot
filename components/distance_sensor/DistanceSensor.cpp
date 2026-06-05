#include "DistanceSensor.hpp"
#include "esp_log.h"

#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"

static const char *TAG = "DistanceSensor";

namespace
{
    constexpr int64_t ECHO_TIMEOUT_US = 30000;
    constexpr float INVALID_DISTANCE_CM = -1.0f;
}

DistanceSensor::DistanceSensor(gpio_num_t trigPin, gpio_num_t echoPin, const char *name)
    : trigPin_(trigPin),
      echoPin_(echoPin),
      name_(name) {}

void DistanceSensor::init()
{
    gpio_config_t trigConfig{};
    trigConfig.pin_bit_mask = (1ULL << trigPin_); // set the bit corresponding to the trigger pin
    trigConfig.mode = GPIO_MODE_OUTPUT;           // sets the trigger pin as output
    trigConfig.pull_up_en = GPIO_PULLUP_DISABLE;
    trigConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    trigConfig.intr_type = GPIO_INTR_DISABLE;

    ESP_ERROR_CHECK(gpio_config(&trigConfig));
    gpio_set_level(trigPin_, 0); // ensure the trigger pin is low

    gpio_config_t echoConfig{};
    echoConfig.pin_bit_mask = (1ULL << echoPin_);
    echoConfig.mode = GPIO_MODE_INPUT; // sets the echo pin as input
    echoConfig.pull_up_en = GPIO_PULLUP_DISABLE;
    echoConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    echoConfig.intr_type = GPIO_INTR_DISABLE;

    ESP_ERROR_CHECK(gpio_config(&echoConfig));

    ESP_LOGI(
        TAG,
        "%s distance sensor initialized | TRIG GPIO: %d | ECHO GPIO: %d",
        name_,
        trigPin_,
        echoPin_);
}

float DistanceSensor::readDistanceCm()
{
    gpio_set_level(trigPin_, 0);
    esp_rom_delay_us(2); // short delay to ensure the trigger pin is low

    gpio_set_level(trigPin_, 1); // set the trigger pin high to start the measurement
    esp_rom_delay_us(10);
    gpio_set_level(trigPin_, 0); // set the trigger pin low to end the pulse

    const int64_t waitStartUs = esp_timer_get_time();

    while (gpio_get_level(echoPin_) == 0)
    {
        if ((esp_timer_get_time() - waitStartUs) > ECHO_TIMEOUT_US)
        {
            ESP_LOGW(TAG, "Timeout waiting for echo to go HIGH");
            return INVALID_DISTANCE_CM;
        }
    }

    const int64_t pulseStartUs = esp_timer_get_time();

    while (gpio_get_level(echoPin_) == 1)
    {
        if ((esp_timer_get_time() - pulseStartUs) > ECHO_TIMEOUT_US)
        {
            ESP_LOGW(TAG, "Timeout waiting for echo to go LOW");
            return INVALID_DISTANCE_CM;
        }
    }

    const int64_t pulseEndUs = esp_timer_get_time();
    const int64_t pulseDurationUs = pulseEndUs - pulseStartUs;

    const float distanceCm = static_cast<float>(pulseDurationUs) / 58.0f; // convert pulse duration to distance in cm

    ESP_LOGI(
        TAG,
        "%s distance read: %.2f cm | pulse: %lld us",
        name_,
        distanceCm,
        pulseDurationUs
    );
    return distanceCm;
}