#pragma once

class MotorDriver {
public:
    MotorDriver();

    void init();
    void enable();
    void disable();

    void drive(int motorASpeed, int motorBSpeed);
    void stop();
};