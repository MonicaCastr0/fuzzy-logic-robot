#include "RobotController.hpp"
#include "AppConfig.hpp"

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "RobotController";

// ======================================================
// Constructor
// ======================================================
RobotController::RobotController()
    : frontDistanceSensor_(AppConfig::FRONT_TRIG_PIN, AppConfig::FRONT_ECHO_PIN, "FrontSensor"),
      rearDistanceSensor_(AppConfig::REAR_TRIG_PIN, AppConfig::REAR_ECHO_PIN, "RearSensor") {}

// ======================================================
// Initialization
// ======================================================

void RobotController::init()
{
    frontDistanceSensor_.init();
    rearDistanceSensor_.init();

    motorDriver_.init();
    fuzzyController_.init();

    motorDriver_.enable();

    state_ = ManeuverState::IDLE;
    stateStartMs_ = 0;

    ESP_LOGI(TAG, "RobotController initialized");
}

// ======================================================
// Main control loop
// ======================================================
void RobotController::update()
{

    ESP_LOGI(TAG, "Control cycle");

    const float front = frontDistanceSensor_.readDistanceCm();
    vTaskDelay(pdMS_TO_TICKS(50));
    const float rear = rearDistanceSensor_.readDistanceCm();

    FuzzyInput input{front, rear};

    // Step 1: fuzzy decision (intention layer)
    const FuzzyOutput fuzzyOutput = fuzzyController_.evaluate(input);

    // Step 2: state machine (trajectory layer)
    updateState(fuzzyOutput);

    // Step 3: execution layer
    const FuzzyOutput finalOutput = postProcessControl(fuzzyOutput);

    motorDriver_.drive(finalOutput.motorASpeed, finalOutput.motorBSpeed);
}

// ======================================================
// State machine update
// ======================================================
void RobotController::updateState(const FuzzyOutput &input)
{

    const int64_t now = nowMs();
    const bool steeringRequested = input.motorBSpeed != AppConfig::STEERING_STOP;
    const bool movingForward = input.motorASpeed > AppConfig::TRACTION_STOP;

    switch (state_)
    {

    case ManeuverState::IDLE:
        if (steeringRequested)
        {
            state_ = movingForward
                         ? ManeuverState::AVOIDING_FORWARD
                         : ManeuverState::REVERSING;

            stateStartMs_ = now;
        }
        break;

    case ManeuverState::AVOIDING_FORWARD:
        if (!steeringRequested)
        {
            state_ = ManeuverState::IDLE;
        }
        break;

    case ManeuverState::REVERSING:
        if (now - stateStartMs_ > AppConfig::STEERING_PULSE_DURATION_MS)
        {
            state_ = ManeuverState::IDLE;
        }
        break;

    default:
        state_ = ManeuverState::IDLE;
        break;
    }
}

// ======================================================
// Execution layer (trajectory enforcement)
// ======================================================
FuzzyOutput RobotController::postProcessControl(const FuzzyOutput &input)
{

    FuzzyOutput output = input;

    const int64_t now = nowMs();

    const bool isForward = input.motorASpeed > AppConfig::TRACTION_STOP;
    const bool steeringActive = input.motorBSpeed != AppConfig::STEERING_STOP;

    // ===============================
    // Forward sustained steering (no interruption)
    // ===============================
    if (state_ == ManeuverState::AVOIDING_FORWARD)
    {

        const int64_t elapsed = now - stateStartMs_;

        if (elapsed < AppConfig::STEERING_HOLD_MIN_MS)
        {
            return output; // keep curve stable
        }

        return output;
    }

    // ===============================
    // Reverse steering protection (pulse behavior)
    // ===============================
    if (state_ == ManeuverState::REVERSING)
    {

        const int64_t elapsed = now - stateStartMs_;

        if (elapsed < AppConfig::STEERING_PULSE_DURATION_MS)
        {
            return output;
        }

        output.motorBSpeed = AppConfig::STEERING_STOP;
        return output;
    }

    // ===============================
    // Safe default behavior
    // ===============================
    if (!steeringActive)
    {
        state_ = ManeuverState::IDLE;
    }

    return output;
}

// ======================================================
// Utility
// ======================================================
int64_t RobotController::nowMs() const
{
    return esp_timer_get_time() / 1000;
}