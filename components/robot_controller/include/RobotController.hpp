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
    enum class ManeuverState
    {
        IDLE,
        STEERING_ACTIVE,
        COOLDOWN
    };

    DistanceSensor frontDistanceSensor_;
    DistanceSensor rearDistanceSensor_;
    MotorDriver motorDriver_;
    FuzzyController fuzzyController_;

    ManeuverState maneuverState_{ManeuverState::IDLE};
    int64_t maneuverStartMs_{0};
    int activeSteering_{0};

    // ===============================
    // Traction profile state
    // ===============================
    int lastAppliedTraction_{0};
    bool startBoostActive_{false};
    int startBoostDirection_{0};
    int64_t startBoostStartMs_{0};

    int64_t nowMs() const;

    void resetManeuver();

    FuzzyOutput applySteeringControl(const FuzzyOutput &input);
    int applyTractionProfile(int targetTraction, float frontDistanceCm);
};