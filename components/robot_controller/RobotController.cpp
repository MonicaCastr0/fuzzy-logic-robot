#include "RobotController.hpp"
#include "AppConfig.hpp"

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "RobotController";

RobotController::RobotController()
    : frontDistanceSensor_(
          AppConfig::FRONT_TRIG_PIN,
          AppConfig::FRONT_ECHO_PIN,
          "FrontSensor"),
      rearDistanceSensor_(
          AppConfig::REAR_TRIG_PIN,
          AppConfig::REAR_ECHO_PIN,
          "RearSensor") {}

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
// STEERING EXECUTION LAYER (FSM - motion stabilizer)
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

        const int64_t elapsed = now - maneuverStartMs_;

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
        const int64_t elapsed = now - maneuverStartMs_;

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
// MAIN CONTROL LOOP (ENTRY POINT)
// =====================================================
void RobotController::update()
{
    ESP_LOGI(TAG, "Running robot control cycle");

    // ===============================
    // 1. SENSOR INPUT
    // ===============================
    const float frontCm = frontDistanceSensor_.readDistanceCm();

    vTaskDelay(pdMS_TO_TICKS(50));

    const float rearCm = rearDistanceSensor_.readDistanceCm();

    // ===============================
    // 2. FUZZY DECISION LAYER
    // ===============================
    FuzzyInput input{};
    input.frontDistanceCm = frontCm;
    input.rearDistanceCm = rearCm;

    const FuzzyOutput fuzzyOut = fuzzyController_.evaluate(input);

    // ===============================
    // 3. MOTION EXECUTOR LAYER
    // ===============================
    const FuzzyOutput stableOut = applySteeringControl(fuzzyOut);

    // ===============================
    // 4. MOTOR OUTPUT
    // ===============================
    motorDriver_.drive(
        stableOut.motorASpeed,
        stableOut.motorBSpeed);
}