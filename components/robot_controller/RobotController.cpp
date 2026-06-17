#include "RobotController.hpp"
#include "AppConfig.hpp"

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"

static const char* TAG = "RobotController";


RobotController::RobotController()
    : frontDistanceSensor_(
        AppConfig::FRONT_TRIG_PIN,
        AppConfig::FRONT_ECHO_PIN, 
        "FrontSensor"
    ),
      rearDistanceSensor_(
        AppConfig::REAR_TRIG_PIN,
        AppConfig::REAR_ECHO_PIN,
        "RearSensor"
      ) {}

void RobotController::init() {
    frontDistanceSensor_.init();
    rearDistanceSensor_.init();
    motorDriver_.init();
    fuzzyController_.init();

    motorDriver_.enable();

    ESP_LOGI(TAG, "RobotController initialized successfully");
}

void RobotController::update() {
    if (AppConfig::MOTOR_DRIVER_TEST_MODE) {
    ESP_LOGI(TAG, "Running physical steering test mode");

    // motorDriver_.drive(180, AppConfig::STEERING_SPEED);
    // vTaskDelay(pdMS_TO_TICKS(500));
    // motorDriver_.drive(180, -AppConfig::STEERING_SPEED);
    // vTaskDelay(pdMS_TO_TICKS(500));
    // motorDriver_.disable();
    // vTaskDelay(pdMS_TO_TICKS(1500));

    // ESP_LOGI(TAG, "Test 1: traction forward only");
    // motorDriver_.drive(180, 0);
    // vTaskDelay(pdMS_TO_TICKS(2000));

    // ESP_LOGI(TAG, "Test 2: stop and wait");
    // motorDriver_.stop();
    // vTaskDelay(pdMS_TO_TICKS(1500));

    ESP_LOGI(TAG, "Test 3: sustained right curve");

    // Pre-steer right
    motorDriver_.drive(0, AppConfig::STEERING_RIGHT);
    vTaskDelay(pdMS_TO_TICKS(200));

    // Move while keeping steering active
    motorDriver_.drive(140, AppConfig::STEERING_RIGHT);
    vTaskDelay(pdMS_TO_TICKS(3000));

    // Release steering and keep moving forward
    motorDriver_.drive(140, 0);
    vTaskDelay(pdMS_TO_TICKS(700));

    ESP_LOGI(TAG, "Test 4: stop and wait");
    motorDriver_.stop();
    vTaskDelay(pdMS_TO_TICKS(1500));

    ESP_LOGI(TAG, "Test 5: sustained left curve");

    // Pre-steer left
    motorDriver_.drive(0, AppConfig::STEERING_LEFT);
    vTaskDelay(pdMS_TO_TICKS(200));

    // Move while keeping steering active
    motorDriver_.drive(140, AppConfig::STEERING_LEFT);
    vTaskDelay(pdMS_TO_TICKS(3000));

    // Release steering and keep moving forward
    motorDriver_.drive(140, 0);
    vTaskDelay(pdMS_TO_TICKS(700));

    ESP_LOGI(TAG, "Test 6: stop and wait");
    motorDriver_.stop();
    vTaskDelay(pdMS_TO_TICKS(1500));

    ESP_LOGI(TAG, "Test 7: sustained reverse right curve");

    // Pre-steer right
    motorDriver_.drive(0, AppConfig::STEERING_RIGHT);
    vTaskDelay(pdMS_TO_TICKS(200));

    // Reverse while keeping steering active
    motorDriver_.drive(-140, AppConfig::STEERING_RIGHT);
    vTaskDelay(pdMS_TO_TICKS(3000));

    // Release steering and keep reversing
    motorDriver_.drive(-140, 0);
    vTaskDelay(pdMS_TO_TICKS(700));

    ESP_LOGI(TAG, "Test completed, stopping motors");
    motorDriver_.stop();
    vTaskDelay(pdMS_TO_TICKS(3000));

    return;
}
    ESP_LOGI(TAG, "Running robot control cycle");
    const float frontCmDistance = frontDistanceSensor_.readDistanceCm();

    vTaskDelay(pdMS_TO_TICKS(50)); // small delay between sensor readings

    const float rearCmDistance = rearDistanceSensor_.readDistanceCm();

    FuzzyInput input{};
    input.frontDistanceCm = frontCmDistance;
    input.rearDistanceCm = rearCmDistance;

    const FuzzyOutput requestedOutput = fuzzyController_.evaluate(input);
    const FuzzyOutput safeOutput = applySteeringPulseControl(requestedOutput);

    motorDriver_.drive(safeOutput.motorASpeed, safeOutput.motorBSpeed);
}

FuzzyOutput RobotController::applySteeringPulseControl(const FuzzyOutput& requestedOutput) {
    FuzzyOutput output = requestedOutput;

    const bool wantsSteering = (requestedOutput.motorBSpeed != AppConfig::STEERING_STOP);

    if (!wantsSteering) {
        resetSteeringPulse();
        return output;
    }

    const bool isMovingForward = requestedOutput.motorASpeed > AppConfig::TRACTION_STOP;

    if (isMovingForward) {
        resetSteeringPulse();
        ESP_LOGI(TAG, "Sustained steering allowed for forward preventive avoidance");
        return output;
    }

    const int64_t currentTimeMs = nowMs();

    if (steeringPulseState_ == SteeringPulseState::Idle) {
        steeringPulseState_ = SteeringPulseState::Pulsing;
        steeringStateStartTimeMs_ = currentTimeMs;
        activeSteeringSpeed_ = requestedOutput.motorBSpeed;

        ESP_LOGI(TAG, "Steering pulse started | steering speed: %d", activeSteeringSpeed_);

        return output;
    }

    if (steeringPulseState_ == SteeringPulseState::Pulsing) {
        if (requestedOutput.motorBSpeed != activeSteeringSpeed_) {
            activeSteeringSpeed_ = requestedOutput.motorBSpeed;
            steeringStateStartTimeMs_ = currentTimeMs;

            ESP_LOGI(TAG, "Steering pulse direction changed | steering speed: %d", activeSteeringSpeed_);
            
            return output;
        }

        const int64_t elapsedMs = currentTimeMs - steeringStateStartTimeMs_;

        if (elapsedMs >= AppConfig::STEERING_PULSE_DURATION_MS) {
            steeringPulseState_ = SteeringPulseState::Cooldown;
            steeringStateStartTimeMs_ = currentTimeMs;

            output.motorBSpeed = AppConfig::STEERING_STOP;

            ESP_LOGI(TAG, "Steering pulse finished | cooldown started");

            return output;
        }

        output.motorBSpeed = activeSteeringSpeed_;
        return output;
    }

    if (steeringPulseState_ == SteeringPulseState::Cooldown) {
        if (requestedOutput.motorBSpeed != activeSteeringSpeed_) {
            steeringPulseState_ = SteeringPulseState::Pulsing;
            steeringStateStartTimeMs_ = currentTimeMs;
            activeSteeringSpeed_ = requestedOutput.motorBSpeed;

            output.motorBSpeed = activeSteeringSpeed_;

            ESP_LOGI(TAG, "Steering cooldown interrupted by direction change | steering speed: %d", activeSteeringSpeed_);
        
            return output;
        }

        const int64_t elapsedMs = currentTimeMs - steeringStateStartTimeMs_;

        if (elapsedMs >= AppConfig::STEERING_PULSE_COOLDOWN_MS) {
            steeringPulseState_ = SteeringPulseState::Pulsing;
            steeringStateStartTimeMs_ = currentTimeMs;
            activeSteeringSpeed_ = requestedOutput.motorBSpeed;

            output.motorBSpeed = activeSteeringSpeed_;

            ESP_LOGI(TAG, "Steering cooldown finished | new pulse started | steering speed: %d", activeSteeringSpeed_);

            return output;
        }

        output.motorBSpeed = AppConfig::STEERING_STOP;
        return output;
    }

    output.motorBSpeed = AppConfig::STEERING_STOP;
    return output;
}

void RobotController::resetSteeringPulse() {
    if (steeringPulseState_ != SteeringPulseState::Idle) {
        ESP_LOGI(TAG, "Steering pulse reset");
    }

    steeringPulseState_ = SteeringPulseState::Idle;
    activeSteeringSpeed_ = AppConfig::STEERING_STOP;
    steeringStateStartTimeMs_ = 0;
}

int64_t RobotController::nowMs() const {
    return esp_timer_get_time() / 1000;
}