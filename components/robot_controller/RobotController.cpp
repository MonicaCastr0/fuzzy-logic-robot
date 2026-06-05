#include "RobotController.hpp"
#include "AppConfig.hpp"

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"

static const char* TAG = "RobotController";


RobotController::RobotController() = default;

void RobotController::init() {
    distanceSensor_.init();
    motorDriver_.init();
    fuzzyController_.init();

    motorDriver_.enable();

    ESP_LOGI(TAG, "RobotController initialized successfully");
}

void RobotController::update() {
    if (AppConfig::MOTOR_DRIVER_TEST_MODE) {
    ESP_LOGI(TAG, "Running physical steering test mode");

    motorDriver_.enable();

    ESP_LOGI(TAG, "Test 1: traction forward only");
    motorDriver_.drive(180, 0);
    vTaskDelay(pdMS_TO_TICKS(2000));

    ESP_LOGI(TAG, "Test 2: stop and wait");
    motorDriver_.stop();
    vTaskDelay(pdMS_TO_TICKS(1500));

    ESP_LOGI(TAG, "Test 3: forward with steering right pulse");
    motorDriver_.drive(160, AppConfig::STEERING_RIGHT);
    vTaskDelay(pdMS_TO_TICKS(250));
    motorDriver_.drive(160, 0);
    vTaskDelay(pdMS_TO_TICKS(2000));

    ESP_LOGI(TAG, "Test 4: stop and wait");
    motorDriver_.stop();
    vTaskDelay(pdMS_TO_TICKS(1500));

    ESP_LOGI(TAG, "Test 5: forward with steering left pulse");
    motorDriver_.drive(160, AppConfig::STEERING_LEFT);
    vTaskDelay(pdMS_TO_TICKS(250));
    motorDriver_.drive(160, 0);
    vTaskDelay(pdMS_TO_TICKS(2000));

    ESP_LOGI(TAG, "Test 6: reverse with steering right pulse");
    motorDriver_.drive(-160, AppConfig::STEERING_RIGHT);
    vTaskDelay(pdMS_TO_TICKS(250));
    motorDriver_.drive(-160, 0);
    vTaskDelay(pdMS_TO_TICKS(1500));

    ESP_LOGI(TAG, "Test completed, stopping motors");
    motorDriver_.stop();
    vTaskDelay(pdMS_TO_TICKS(3000));

    return;
}
    ESP_LOGI(TAG, "Running robot control cycle");
    const float frontCmDistance = distanceSensor_.readDistanceCm();

    FuzzyInput input{};
    input.frontDistanceCm = frontCmDistance;

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