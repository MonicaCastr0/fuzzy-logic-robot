idf_component_register(
    SRCS "MotorDriver.cpp"
    INCLUDE_DIRS "include"
    REQUIRES
        app_config
        esp_driver_gpio
        esp_driver_ledc
        log
)

target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++17)