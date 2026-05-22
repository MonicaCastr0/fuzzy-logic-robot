#pragma once

#include "driver/gpio.h"
#include "driver/ledc.h"

class MotorDriver {
public:
    //simple public interface to control the motors without exposing GPIO and PWM details
    MotorDriver();

    void init();
    void enable();
    void disable();

    void drive(int motorASpeed, int motorBSpeed);
    void stop();

private:
    struct MotorPins {
        gpio_num_t in1;
        gpio_num_t in2;
        gpio_num_t pwm;
        ledc_channel_t channel;
    };

    //private methods to hide GPIO and PWM details
    void setupGpio();
    void setupPwm();

    void setupPwmChannel(gpio_num_t pwmPin, ledc_channel_t channel);
    void setMotor(const MotorPins& motor, int speed);
    void setPwmDuty(ledc_channel_t channel, int duty);

    int clampSpeed(int speed) const;
    int absoluteValue(int value) const;
};