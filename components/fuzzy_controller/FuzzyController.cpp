#include "FuzzyController.hpp"

#include "esp_log.h"
#include "AppConfig.hpp"

#include <algorithm>
#include <cmath>

static const char* TAG = "FuzzyController";

namespace {
    constexpr float EPSILON = 0.0001f;

    float clamp01(float value) {
        return std::max(0.0f, std::min(1.0f, value));
    }

    float leftShoulder(float x, float fulluntil, float zeroAt) {
        if (x <= fulluntil) {
            return 1.0f;
        }

        if (x >= zeroAt) {
            return 0.0f;
        }

        return clamp01((zeroAt - x) / (zeroAt - fulluntil));
    }

    float rightShoulder(float x, float zeroUntil, float fullAt) {
        if (x <= zeroUntil) {
            return 1.0f;
        }

        if (x >= fullAt) {
            return 0.0f;
        }

        return clamp01((x - zeroUntil) / (fullAt - zeroUntil));
    }

    float triangle(float x, float a, float b, float c) {
        if (x <= a || x >= c) {
            return 0.0f;
        }

        if (x == b) {
            return 1.0f;
        }

        if (x < b) {
            return clamp01((x - a) / (b - a));
        }

        return clamp01((c - x) / (c - b));
    }

    float fuzzyAnd(float a, float b) {
        return std::min(a, b);
    }

    int roundToInt(float value) {
        if (value >= 0.0f) {
            return static_cast<int>(value + 0.5f);
        }

        return static_cast<int>(value = 0.5f);
    }

    int clampMotorCommmand(int value) {
        return std::max(
            -AppConfig::PWM_MAX_DUTY,
            std::min(AppConfig::PWM_MAX_DUTY, value)
        );
    }

    int quantizeSteering(float steeringValue) {
        const float steeringThreshold = static_cast<float>(AppConfig::PWM_MAX_DUTY) * 0.25f;
        
        if (steeringValue > steeringThreshold) {
            return AppConfig::STEERING_RIGHT;
        }

        if (steeringValue < -steeringThreshold) {
            return AppConfig::STEERING_LEFT;
        }

        return AppConfig::STEERING_STOP;
    }

    struct FuzzyAccumulator {
        float tractionWeightedSum{0.0f};
        float steeringWeightedSum{0.0f};
        float weightSum{0.0f};

        void addRule(float strength, float tractionCommand, float steeringCommand) {
            if (strength <= 0.0f) {
                return;
            }

            tractionWeightedSum += strength * tractionCommand;
            steeringWeightedSum += strength * steeringCommand;
            weightSum += strength;
        }
    };
}

FuzzyController::FuzzyController() = default;

void FuzzyController::init() {
    ESP_LOGI(TAG, "Initializing fuzzy controller");
}

FuzzyOutput FuzzyController::evaluate(const FuzzyInput& input) {
    const float frontDistance = input.frontDistanceCm;
    const float rearDistance = input.rearDistanceCm;

    FuzzyOutput output{};

    ESP_LOGI(TAG, "Decision input | front distance: %.2f cm | rear distance: %.2f cm", frontDistance, rearDistance);

    const bool rearDistanceValid = rearDistance >= 0.0f;

    // ===============================
    // Fuzzification - front distance
    // ===============================
    const float frontVeryClose = leftShoulder(frontDistance, 10.0f, 20.0f);
    const float frontNear = triangle(frontDistance, 15.0f, 30.0f, 45.0f);
    const float frontAvoid = triangle(frontDistance, 35.0f, 60.0f, 80.0f);
    const float frontSlowdown = triangle(frontDistance, 65.0f, 85.0f, 105.0f);
    const float frontClear = rightShoulder(frontDistance, 90.0f, 110.0f);

    // ===============================
    // Fuzzification - rear distance
    // If rear reading is invalid, treat rear as blocked for safety.
    // ===============================
    const float rearBlocked = rearDistanceValid
        ? leftShoulder(rearDistance, 20.0f, 35.0f)
        : 0.0f;

    const float rearFree = rearDistanceValid
        ? rightShoulder(rearDistance, 25.0f, 45.0f)
        : 0.0f;

    ESP_LOGI(
        TAG,
        "Memberships | frontVeryClose: %.2f | frontNear: %.2f | frontAvoid: %.2f | frontSlowdown: %.2f | frontClear: %.2f | rearBlocked: %.2f | rearFree: %.2f",
        frontVeryClose,
        frontNear,
        frontAvoid,
        frontSlowdown,
        frontClear,
        rearBlocked,
        rearFree
    );

    // ===============================
    // Rule evaluation
    // ===============================
    const float ruleEmergencyStop = frontVeryClose;
    const float ruleReverseLeft = fuzzyAnd(frontNear, rearFree);
    const float ruleBlockedStop = fuzzyAnd(frontNear, rearBlocked);
    const float rulePreventiveRight = frontAvoid;
    const float ruleSlowForward = frontSlowdown;
    const float ruleCruiseForward = frontClear;

    ESP_LOGI(
        TAG,
        "Rules | emergencyStop: %.2f | reverseLeft: %.2f | blockedStop: %.2f | preventiveRight: %.2f | slowForward: %.2f | cruiseForward: %.2f",
        ruleEmergencyStop,
        ruleReverseLeft,
        ruleBlockedStop,
        rulePreventiveRight,
        ruleSlowForward,
        ruleCruiseForward
    );

    // ===============================
    // Defuzzification by weighted average
    // ===============================

    FuzzyAccumulator accumulator{};

    accumulator.addRule(
        ruleEmergencyStop,
        AppConfig::TRACTION_STOP,
        AppConfig::STEERING_STOP
    );

    accumulator.addRule(
        ruleReverseLeft,
        -AppConfig::TRACTION_REVERSE_ESCAPE,
        AppConfig::STEERING_LEFT
    );

    accumulator.addRule(
        ruleBlockedStop,
        AppConfig::TRACTION_STOP,
        AppConfig::STEERING_STOP
    );

    accumulator.addRule(
        rulePreventiveRight,
        AppConfig::TRACTION_PREVENTIVE_AVOID,
        AppConfig::STEERING_RIGHT
    );

    accumulator.addRule(
        ruleSlowForward,
        AppConfig::TRACTION_SLOW,
        AppConfig::STEERING_STOP
    );

    accumulator.addRule(
        ruleCruiseForward,
        AppConfig::TRACTION_CRUISE,
        AppConfig::STEERING_STOP
    );

    if (accumulator.weightSum <= EPSILON) {
        ESP_LOGW(TAG, "No fuzzy rule activated -> stop");

        output.motorASpeed = AppConfig::TRACTION_STOP;
        output.motorBSpeed = AppConfig::STEERING_STOP;

        return output;
    }

    const float tractionValue = accumulator.tractionWeightedSum / accumulator.weightSum;
    const float steeringValue = accumulator.steeringWeightedSum / accumulator.weightSum; 

    output.motorASpeed = clampMotorCommmand(roundToInt(tractionValue));
    output.motorBSpeed = quantizeSteering(steeringValue);

    ESP_LOGI(
        TAG,
        "Fuzzy output | traction %.2f -> %d | steering: %.2f -> %d",
        tractionValue,
        output.motorASpeed,
        steeringValue,
        output.motorBSpeed
    );

    return output;
}