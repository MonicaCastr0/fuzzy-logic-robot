#pragma once

#include "DistanceSensor.hpp"
#include "MotorDriver.hpp"
#include "FuzzyController.hpp"

#include <cstdint>

class RobotController
{
public:
    RobotController();

    void init();
    void update();

private:
    // ===============================
    // Sensors
    // ===============================
    DistanceSensor frontDistanceSensor_;
    DistanceSensor rearDistanceSensor_;

    // ===============================
    // Core modules
    // ===============================
    MotorDriver motorDriver_;
    FuzzyController fuzzyController_;

    // ===============================
    // Behavior state machine
    // ===============================
    enum class ManeuverState
    {
        IDLE,
        FORWARD,
        AVOIDING_FORWARD,
        REVERSING
    };

    ManeuverState state_{ManeuverState::IDLE};
    int64_t stateStartMs_{0};

    // ===============================
    // Control pipeline
    // ===============================
    FuzzyOutput postProcessControl(const FuzzyOutput &input);

    void updateState(const FuzzyOutput &input);

    // ===============================
    // Utilities
    // ===============================
    int64_t nowMs() const;
};