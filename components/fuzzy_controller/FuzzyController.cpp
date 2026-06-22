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

    float leftShoulder(float x, float fulluntil, float zeroAt)
    {
        if (x <= fulluntil)
        {
            return 1.0f;
        }
        if (x >= zeroAt)
        {
            return 0.0f;
        }
        return clamp01((zeroAt - x) / (zeroAt - fulluntil));
    }

    float rightShoulder(float x, float zeroUntil, float fullAt)
    {
        if (x <= zeroUntil)
        {
            return 1.0f;
        }
        if (x >= fullAt)
        {
            return 0.0f;
        }
        return clamp01((x - zeroUntil) / (fullAt - zeroUntil));
    }

    float triangle(float x, float a, float b, float c)
    {
        if (x <= a || x >= c)
            return 0.0f;
        if (x == b)
            return 1.0f;

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

    int clampMotorCommand(int value)
    {
        return std::max(
            -AppConfig::PWM_MAX_DUTY,
            std::min(AppConfig::PWM_MAX_DUTY, value));
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

    struct FuzzyAccumulator
    {
        float tractionWeightedSum{0.0f};
        float steeringWeightedSum{0.0f};
        float weightSum{0.0f};

        void addRule(float strength, float traction, float steering)
        {
            if (strength <= 0.0f)
                return;

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

    const bool rearValid = rear >= 0.0f;

    ESP_LOGI(TAG,
             "Input | front=%.2f cm | rear=%.2f cm",
             front, rear);

    // ===============================
    // FUZZIFICATION (FRONT)
    // ===============================
    const float frontVeryClose = leftShoulder(front, 10.0f, 20.0f);
    const float frontNear = triangle(front, 15.0f, 30.0f, 45.0f);
    const float frontAvoid = triangle(front, 35.0f, 60.0f, 80.0f);
    const float frontSlow = triangle(front, 65.0f, 85.0f, 105.0f);

    // FIX: OPEN WORLD SAFE REGION (NO DEAD ZONE)
    const float frontClear =
        (front >= 110.0f)
            ? 1.0f
            : rightShoulder(front, 80.0f, 110.0f);

    // ===============================
    // FUZZIFICATION (REAR)
    // ===============================
    const float rearBlocked = rearValid
                                  ? leftShoulder(rear, 20.0f, 35.0f)
                                  : 0.0f;

    const float rearFree = rearValid
                               ? rightShoulder(rear, 25.0f, 45.0f)
                               : 0.0f;

    // ===============================
    // RULES
    // ===============================
    const float ruleStop = frontVeryClose;
    const float ruleReverseLeft = fuzzyAnd(frontNear, rearFree);
    const float ruleBlockedStop = fuzzyAnd(frontNear, rearBlocked);
    const float ruleAvoidRight = frontAvoid;
    const float ruleSlow = frontSlow;
    const float ruleCruise = frontClear;

    FuzzyAccumulator acc{};

    acc.addRule(ruleStop,
                AppConfig::TRACTION_STOP,
                AppConfig::STEERING_STOP);

    acc.addRule(ruleReverseLeft,
                -AppConfig::TRACTION_REVERSE_ESCAPE,
                AppConfig::STEERING_LEFT);

    acc.addRule(ruleBlockedStop,
                AppConfig::TRACTION_STOP,
                AppConfig::STEERING_STOP);

    acc.addRule(ruleAvoidRight,
                AppConfig::TRACTION_PREVENTIVE_AVOID,
                AppConfig::STEERING_RIGHT);

    acc.addRule(ruleSlow,
                AppConfig::TRACTION_SLOW,
                AppConfig::STEERING_STOP);

    acc.addRule(ruleCruise,
                AppConfig::TRACTION_CRUISE,
                AppConfig::STEERING_STOP);

    // ===============================
    // IMPORTANT SAFETY FALLBACK (FIXED)
    // ===============================
    if (acc.weightSum <= EPSILON)
    {

        ESP_LOGW(TAG, "Fuzzy dead zone -> safe cruise fallback");

        output.motorASpeed = AppConfig::TRACTION_SLOW;
        output.motorBSpeed = AppConfig::STEERING_STOP;

        return output;
    }

    const float traction =
        acc.tractionWeightedSum / acc.weightSum;

    const float steering =
        acc.steeringWeightedSum / acc.weightSum;

    output.motorASpeed = clampMotorCommand(roundToInt(traction));
    output.motorBSpeed = quantizeSteering(steering);

    ESP_LOGI(TAG,
             "Output | traction=%.2f -> %d | steering=%.2f -> %d",
             traction,
             output.motorASpeed,
             steering,
             output.motorBSpeed);

    return output;
}