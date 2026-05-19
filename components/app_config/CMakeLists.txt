idf_component_register(
    INCLUDE_DIRS "include"
    REQUIRES
        esp_driver_gpio
        esp_driver_ledc
)

target_compile_options(${COMPONENT_LIB} INTERFACE -std=gnu++17)