#pragma once

#include "FuzzyController.hpp"
#include "MotorDriver.hpp"
#include "DistanceSensor.hpp"

class RobotController
{
public:
    RobotController();

    void init();
    void update();

private:
    FuzzyController fuzzyController_;
    MotorDriver motorDriver_;

    DistanceSensor frontDistanceSensor_;
    DistanceSensor rearDistanceSensor_;

    // ===============================
    // Maneuver state machine (physical layer)
    // ===============================
    enum class ManeuverState
    {
        IDLE,
        STEERING_ACTIVE,
        COOLDOWN
    };

    ManeuverState maneuverState_ = ManeuverState::IDLE;

    int64_t maneuverStartMs_ = 0;
    int activeSteering_ = 0;

    // ===============================
    // Steering persistence layer
    // ===============================
    FuzzyOutput applySteeringControl(const FuzzyOutput &input);
    void resetManeuver();
    int64_t nowMs() const;
};