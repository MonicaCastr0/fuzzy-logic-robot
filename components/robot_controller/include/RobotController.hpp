#pragma once

#include "DistanceSensor.hpp"
#include "MotorDriver.hpp"
#include "FuzzyController.hpp"

class RobotController {
public:
    RobotController();

    void init();
    void update();

private:
    DistanceSensor distanceSensor_;
    MotorDriver motorDriver_;
    FuzzyController fuzzyController_;

};