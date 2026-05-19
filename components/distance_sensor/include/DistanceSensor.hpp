#pragma once

class DistanceSensor {
public:
    DistanceSensor();

    void init();
    float readDistanceCm();
};