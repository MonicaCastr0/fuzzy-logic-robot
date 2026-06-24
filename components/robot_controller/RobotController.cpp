#include "RobotController.hpp"
#include "AppConfig.hpp"

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <algorithm>

static const char *TAG = "RobotController";

namespace
{
    int signOf(int value)
    {
        if (value > 0)
        {
            return 1;
        }

        if (value < 0)
        {
            return -1;
        }

        return 0;
    }

    int absInt(int value)
    {
        return value < 0 ? -value : value;
    }

    int rampTowards(int current, int target)
    {
        if (current == target)
        {
            return current;
        }

        const bool increasingMagnitude =
            absInt(target) > absInt(current);

        const int step = increasingMagnitude
            ? AppConfig::TRACTION_ACCEL_STEP
            : AppConfig::TRACTION_DECEL_STEP;

        if (current < target)
        {
            return std::min(current + step, target);
        }

        return std::max(current - step, target);
    }
}

RobotController::RobotController()
    : frontDistanceSensor_(
          AppConfig::FRONT_TRIG_PIN,
          AppConfig::FRONT_ECHO_PIN,
          "FrontSensor"),
      rearDistanceSensor_(
          AppConfig::REAR_TRIG_PIN,
          AppConfig::REAR_ECHO_PIN,
          "RearSensor")
{
}

void RobotController::init()
{
    frontDistanceSensor_.init();
    rearDistanceSensor_.init();
    motorDriver_.init();
    fuzzyController_.init();

    motorDriver_.enable();

    maneuverState_ = ManeuverState::IDLE;
    maneuverStartMs_ = 0;
    activeSteering_ = AppConfig::STEERING_STOP;

    lastAppliedTraction_ = AppConfig::TRACTION_STOP;
    startBoostActive_ = false;
    startBoostDirection_ = 0;
    startBoostStartMs_ = 0;

    ESP_LOGI(TAG, "RobotController initialized");
}

int64_t RobotController::nowMs() const
{
    return esp_timer_get_time() / 1000;
}

void RobotController::resetManeuver()
{
    maneuverState_ = ManeuverState::IDLE;
    activeSteering_ = AppConfig::STEERING_STOP;
    maneuverStartMs_ = 0;
}

// =====================================================
// TRACTION EXECUTION LAYER
// Start boost + ramp profile
// =====================================================
int RobotController::applyTractionProfile(
    int targetTraction,
    float frontDistanceCm)
{
    const int targetDirection = signOf(targetTraction);
    const int currentDirection = signOf(lastAppliedTraction_);

    const bool targetIsStop =
        targetTraction == AppConfig::TRACTION_STOP;

    const bool robotIsStopped =
        lastAppliedTraction_ == AppConfig::TRACTION_STOP;

    const bool frontValid =
        frontDistanceCm >= 0.0f;

    const bool frontEmergency =
        frontValid &&
        frontDistanceCm <= AppConfig::TRACTION_EMERGENCY_STOP_DISTANCE_CM;

    // If the target is stop, stop immediately.
    if (targetIsStop)
    {
        startBoostActive_ = false;
        startBoostDirection_ = 0;
        startBoostStartMs_ = 0;

        lastAppliedTraction_ = AppConfig::TRACTION_STOP;
        return AppConfig::TRACTION_STOP;
    }

    // If the front is too close, do not allow forward traction.
    // Reverse traction is still allowed because it is an escape maneuver.
    if (frontEmergency && targetDirection > 0)
    {
        ESP_LOGW(
            TAG,
            "Front emergency distance %.2f cm -> cancel forward traction",
            frontDistanceCm
        );

        startBoostActive_ = false;
        startBoostDirection_ = 0;
        startBoostStartMs_ = 0;

        lastAppliedTraction_ = AppConfig::TRACTION_STOP;
        return AppConfig::TRACTION_STOP;
    }

    // If direction changes while the robot is moving, stop first.
    // The new direction will be applied in the next control cycle.
    if (currentDirection != 0 &&
        targetDirection != 0 &&
        currentDirection != targetDirection)
    {
        ESP_LOGI(TAG, "Traction direction changed -> stop before reversing");

        startBoostActive_ = false;
        startBoostDirection_ = 0;
        startBoostStartMs_ = 0;

        lastAppliedTraction_ = AppConfig::TRACTION_STOP;
        return AppConfig::TRACTION_STOP;
    }

    const bool forwardBoostAllowed =
        !frontValid ||
        frontDistanceCm >= AppConfig::TRACTION_BOOST_MIN_FRONT_DISTANCE_CM;

    const bool reverseBoostAllowed =
        targetDirection < 0;

    const bool safeToBoost =
        forwardBoostAllowed || reverseBoostAllowed;

    const bool shouldStartBoost =
        AppConfig::TRACTION_START_BOOST_ENABLED &&
        robotIsStopped &&
        targetDirection != 0 &&
        safeToBoost;

    if (shouldStartBoost)
    {
        startBoostActive_ = true;
        startBoostDirection_ = targetDirection;
        startBoostStartMs_ = nowMs();

        ESP_LOGI(
            TAG,
            "Starting traction boost | direction=%d | duty=%d | duration=%d ms",
            startBoostDirection_,
            AppConfig::TRACTION_START_BOOST_DUTY,
            AppConfig::TRACTION_START_BOOST_DURATION_MS
        );
    }

    if (startBoostActive_)
    {
        const int64_t elapsedMs =
            nowMs() - startBoostStartMs_;

        // Cancel boost if the fuzzy output changes direction or asks to stop.
        if (targetDirection != startBoostDirection_ || targetIsStop)
        {
            ESP_LOGI(TAG, "Traction boost canceled");

            startBoostActive_ = false;
            startBoostDirection_ = 0;
            startBoostStartMs_ = 0;

            lastAppliedTraction_ = AppConfig::TRACTION_STOP;
            return AppConfig::TRACTION_STOP;
        }

        if (elapsedMs < AppConfig::TRACTION_START_BOOST_DURATION_MS)
        {
            lastAppliedTraction_ =
                startBoostDirection_ *
                AppConfig::TRACTION_START_BOOST_DUTY;

            return lastAppliedTraction_;
        }

        // After the boost, drop immediately to the fuzzy target.
        startBoostActive_ = false;
        startBoostDirection_ = 0;
        startBoostStartMs_ = 0;

        lastAppliedTraction_ = targetTraction;

        ESP_LOGI(
            TAG,
            "Traction boost finished -> target traction=%d",
            targetTraction
        );

        return lastAppliedTraction_;
    }

    lastAppliedTraction_ =
        rampTowards(lastAppliedTraction_, targetTraction);

    return lastAppliedTraction_;
}

// =====================================================
// STEERING EXECUTION LAYER
// FSM - motion stabilizer
// =====================================================
FuzzyOutput RobotController::applySteeringControl(const FuzzyOutput &input)
{
    FuzzyOutput out = input;

    const bool wantsSteering =
        input.motorBSpeed != AppConfig::STEERING_STOP;

    const int64_t now = nowMs();

    if (!wantsSteering)
    {
        resetManeuver();
        return out;
    }

    switch (maneuverState_)
    {
    case ManeuverState::IDLE:
        maneuverState_ = ManeuverState::STEERING_ACTIVE;
        maneuverStartMs_ = now;
        activeSteering_ = input.motorBSpeed;
        return out;

    case ManeuverState::STEERING_ACTIVE:
    {
        const bool directionChanged =
            input.motorBSpeed != activeSteering_;

        if (directionChanged)
        {
            activeSteering_ = input.motorBSpeed;
            maneuverStartMs_ = now;
            return out;
        }

        const int64_t elapsed =
            now - maneuverStartMs_;

        if (elapsed >= AppConfig::STEERING_PULSE_DURATION_MS)
        {
            maneuverState_ = ManeuverState::COOLDOWN;
            maneuverStartMs_ = now;
            out.motorBSpeed = AppConfig::STEERING_STOP;
            return out;
        }

        out.motorBSpeed = activeSteering_;
        return out;
    }

    case ManeuverState::COOLDOWN:
    {
        const int64_t elapsed =
            now - maneuverStartMs_;

        const bool directionChanged =
            input.motorBSpeed != activeSteering_;

        if (directionChanged)
        {
            maneuverState_ = ManeuverState::STEERING_ACTIVE;
            maneuverStartMs_ = now;
            activeSteering_ = input.motorBSpeed;
            return out;
        }

        if (elapsed >= AppConfig::STEERING_PULSE_COOLDOWN_MS)
        {
            maneuverState_ = ManeuverState::STEERING_ACTIVE;
            maneuverStartMs_ = now;
            activeSteering_ = input.motorBSpeed;
            return out;
        }

        out.motorBSpeed = AppConfig::STEERING_STOP;
        return out;
    }
    }

    return out;
}

// =====================================================
// MAIN CONTROL LOOP
// =====================================================
void RobotController::update()
{
    ESP_LOGI(TAG, "Running robot control cycle");

    // ===============================
    // 1. SENSOR INPUT
    // ===============================
    const float frontCm =
        frontDistanceSensor_.readDistanceCm();

    vTaskDelay(pdMS_TO_TICKS(50));

    const float rearCm =
        rearDistanceSensor_.readDistanceCm();

    // ===============================
    // 2. FUZZY DECISION LAYER
    // ===============================
    FuzzyInput input{};
    input.frontDistanceCm = frontCm;
    input.rearDistanceCm = rearCm;

    const FuzzyOutput fuzzyOut =
        fuzzyController_.evaluate(input);

    // ===============================
    // 3. STEERING MOTION EXECUTOR
    // ===============================
    FuzzyOutput stableOut =
        applySteeringControl(fuzzyOut);

    // ===============================
    // 4. TRACTION PROFILE EXECUTOR
    // ===============================
    stableOut.motorASpeed =
        applyTractionProfile(
            stableOut.motorASpeed,
            frontCm
        );

    // ===============================
    // 5. MOTOR OUTPUT
    // ===============================
    ESP_LOGI(
        TAG,
        "Motor output | traction=%d | steering=%d",
        stableOut.motorASpeed,
        stableOut.motorBSpeed
    );

    motorDriver_.drive(
        stableOut.motorASpeed,
        stableOut.motorBSpeed
    );
}