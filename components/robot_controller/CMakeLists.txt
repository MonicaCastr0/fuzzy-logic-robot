idf_component_register(
    SRCS "RobotController.cpp"
    INCLUDE_DIRS "include"
    REQUIRES
        log
        motor_driver
        distance_sensor
        fuzzy_controller
)

target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++17)