idf_component_register(
    SRCS "DistanceSensor.cpp"
    INCLUDE_DIRS "include"
    REQUIRES
        app_config
        esp_driver_gpio
        esp_timer
        esp_rom
        log
)

target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++17)