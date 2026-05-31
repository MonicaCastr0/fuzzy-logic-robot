#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "RobotController.hpp"

extern "C" void app_main() {
    RobotController robot;

    robot.init();

    while (true) {
        robot.update();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}