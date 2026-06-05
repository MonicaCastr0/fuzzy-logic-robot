#pragma once

#include "driver/gpio.h"

class DistanceSensor {
public:
    DistanceSensor(gpio_num_t trigPin, gpio_num_t echoPin, const char* name);

    void init();
    float readDistanceCm();

private:
    gpio_num_t trigPin_;
    gpio_num_t echoPin_;
    const char* name_;
};