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

    enum class SteeringPulseState{
        Idle,
        Pulsing,
        Cooldown
    };

    DistanceSensor frontDistanceSensor_;
    DistanceSensor rearDistanceSensor_;
    MotorDriver motorDriver_;
    FuzzyController fuzzyController_;

    SteeringPulseState steeringPulseState_{SteeringPulseState::Idle};
    int activeSteeringSpeed_{0};
    int64_t steeringStateStartTimeMs_{0};

    FuzzyOutput applySteeringPulseControl(const FuzzyOutput& fuzzyOutput);
    void resetSteeringPulse();
    int64_t nowMs() const;
};