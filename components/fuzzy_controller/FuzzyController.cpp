#include "FuzzyController.hpp"

#include "esp_log.h"
#include "AppConfig.hpp"

#include <algorithm>
#include <cmath>

static const char *TAG = "FuzzyController";

namespace
{
    constexpr float EPSILON = 0.0001f;

    float clamp01(float value)
    {
        return std::max(0.0f, std::min(1.0f, value));
    }

    float leftShoulder(float x, float fullUntil, float zeroAt)
    {
        if (x <= fullUntil)
        {
            return 1.0f;
        }

        if (x >= zeroAt)
        {
            return 0.0f;
        }

        return clamp01((zeroAt - x) / (zeroAt - fullUntil));
    }

    float rightShoulder(float x, float zeroUntil, float fullAt)
    {
        if (x <= zeroUntil)
        {
            return 0.0f;
        }

        if (x >= fullAt)
        {
            return 1.0f;
        }

        return clamp01((x - zeroUntil) / (fullAt - zeroUntil));
    }

    float triangle(float x, float a, float b, float c)
    {
        if (x <= a || x >= c)
        {
            return 0.0f;
        }

        if (x == b)
        {
            return 1.0f;
        }

        if (x < b)
        {
            return clamp01((x - a) / (b - a));
        }

        return clamp01((c - x) / (c - b));
    }

    float fuzzyAnd(float a, float b)
    {
        return std::min(a, b);
    }

    int roundToInt(float value)
    {
        if (value >= 0.0f)
        {
            return static_cast<int>(value + 0.5f);
        }

        return static_cast<int>(value - 0.5f);
    }

    int clampTractionCommand(int value)
    {
        return std::max(
            -AppConfig::TRACTION_REVERSE_MAX_OUTPUT,
            std::min(AppConfig::TRACTION_FORWARD_MAX_OUTPUT, value));
    }

    int quantizeSteering(float steeringValue)
    {
        const float threshold =
            static_cast<float>(AppConfig::PWM_MAX_DUTY) * 0.25f;

        if (steeringValue > threshold)
        {
            return AppConfig::STEERING_RIGHT;
        }

        if (steeringValue < -threshold)
        {
            return AppConfig::STEERING_LEFT;
        }

        return AppConfig::STEERING_STOP;
    }

    int applyMinimumTraction(int value)
    {
        if (value > 0 && value < AppConfig::TRACTION_MIN_FORWARD_OUTPUT)
        {
            return AppConfig::TRACTION_MIN_FORWARD_OUTPUT;
        }
        if (value < 0 && -AppConfig::TRACTION_MIN_REVERSE_OUTPUT)
        {
            return AppConfig::TRACTION_MIN_REVERSE_OUTPUT;
        }

        return value;
    }

    struct FuzzyAccumulator
    {
        float tractionWeightedSum{0.0f};
        float steeringWeightedSum{0.0f};
        float weightSum{0.0f};

        void addRule(float strength, float traction, float steering)
        {
            if (strength <= 0.0f)
            {
                return;
            }

            tractionWeightedSum += strength * traction;
            steeringWeightedSum += strength * steering;
            weightSum += strength;
        }
    };
}

FuzzyController::FuzzyController() = default;

void FuzzyController::init()
{
    ESP_LOGI(TAG, "Initializing fuzzy controller");
}

FuzzyOutput FuzzyController::evaluate(const FuzzyInput &input)
{
    const float front = input.frontDistanceCm;
    const float rear = input.rearDistanceCm;

    FuzzyOutput output{};

    const bool frontValid = front >= 0.0f;
    const bool rearValid = rear >= 0.0f;

    ESP_LOGI(
        TAG,
        "Input | front=%.2f cm | rear=%.2f cm",
        front,
        rear);

    if (!frontValid)
    {
        ESP_LOGW(TAG, "Invalid front distance -> stop");

        output.motorASpeed = AppConfig::TRACTION_STOP;
        output.motorBSpeed = AppConfig::STEERING_STOP;

        return output;
    }

    // ===============================
    // FUZZIFICATION - FRONT DISTANCE
    // ===============================
    const float frontVeryClose = leftShoulder(front, 12.0f, 25.0f);
    const float frontNear      = triangle(front, 20.0f, 40.0f, 65.0f);
    const float frontAvoid     = triangle(front, 50.0f, 90.0f, 130.0f);
    const float frontSlow      = triangle(front, 110.0f, 150.0f, 190.0f);
    
    // Open-ended clear region:
    // once front distance is high enough, the robot should keep cruising.
    const float frontClear     = rightShoulder(front, 170.0f, 210.0f);

    // ===============================
    // FUZZIFICATION - REAR DISTANCE
    // ===============================
    // If rear reading is invalid, rear is treated as blocked for safety.
    const float rearBlocked = rearValid
                                  ? leftShoulder(rear, 20.0f, 35.0f)
                                  : 1.0f;

    const float rearFree = rearValid
                               ? rightShoulder(rear, 25.0f, 45.0f)
                               : 0.0f;

    ESP_LOGI(
        TAG,
        "Memberships | frontVeryClose=%.2f | frontNear=%.2f | frontAvoid=%.2f | frontSlow=%.2f | frontClear=%.2f | rearBlocked=%.2f | rearFree=%.2f",
        frontVeryClose,
        frontNear,
        frontAvoid,
        frontSlow,
        frontClear,
        rearBlocked,
        rearFree);

    // ===============================
    // RULE EVALUATION
    // ===============================
    const float ruleStop = frontVeryClose;
    const float ruleReverseLeft = fuzzyAnd(frontNear, rearFree);
    const float ruleBlockedStop = fuzzyAnd(frontNear, rearBlocked);
    const float ruleAvoidRight = frontAvoid;
    const float ruleSlow = frontSlow;
    const float ruleCruise = frontClear;

    ESP_LOGI(
        TAG,
        "Rules | stop=%.2f | reverseLeft=%.2f | blockedStop=%.2f | avoidRight=%.2f | slow=%.2f | cruise=%.2f",
        ruleStop,
        ruleReverseLeft,
        ruleBlockedStop,
        ruleAvoidRight,
        ruleSlow,
        ruleCruise);

    // ===============================
    // DEFUZZIFICATION BY WEIGHTED AVERAGE
    // ===============================
    FuzzyAccumulator acc{};

    acc.addRule(
        ruleStop,
        AppConfig::TRACTION_STOP,
        AppConfig::STEERING_STOP);

    acc.addRule(
        ruleReverseLeft,
        -AppConfig::TRACTION_REVERSE_ESCAPE,
        AppConfig::STEERING_LEFT);

    acc.addRule(
        ruleBlockedStop,
        AppConfig::TRACTION_STOP,
        AppConfig::STEERING_STOP);

    acc.addRule(
        ruleAvoidRight,
        AppConfig::TRACTION_PREVENTIVE_AVOID,
        AppConfig::STEERING_RIGHT);

    acc.addRule(
        ruleSlow,
        AppConfig::TRACTION_SLOW,
        AppConfig::STEERING_STOP);

    acc.addRule(
        ruleCruise,
        AppConfig::TRACTION_CRUISE,
        AppConfig::STEERING_STOP);

    // ===============================
    // SAFETY FALLBACK
    // ===============================
    if (acc.weightSum <= EPSILON)
    {
        ESP_LOGW(TAG, "No fuzzy rule activated -> safe slow forward fallback");

        output.motorASpeed = AppConfig::TRACTION_SLOW;
        output.motorBSpeed = AppConfig::STEERING_STOP;

        return output;
    }

    const float traction =
        acc.tractionWeightedSum / acc.weightSum;

    const float steering =
        acc.steeringWeightedSum / acc.weightSum;

    const int rawTraction = roundToInt(traction);
    const int tractionWithMinimum = applyMinimumTraction(rawTraction);

    output.motorASpeed = clampTractionCommand(tractionWithMinimum);
    output.motorBSpeed = quantizeSteering(steering);

    ESP_LOGI(
        TAG,
        "Output | traction=%.2f -> %d | steering=%.2f -> %d",
        traction,
        output.motorASpeed,
        steering,
        output.motorBSpeed);

    return output;
}