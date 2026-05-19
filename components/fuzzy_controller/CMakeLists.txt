idf_component_register(
    SRCS "FuzzyController.cpp"
    INCLUDE_DIRS "include"
    REQUIRES
        app_config
        log
)

target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++17)