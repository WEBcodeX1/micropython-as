idf_component_register(
    SRCS "micropython_as.cpp" "TitleScreen.cpp" "GameScreen.cpp"
    PRIV_REQUIRES esp_wifi esp_driver_ledc pthread network_oop micropython peripherals filesystem
    INCLUDE_DIRS "."
)

target_compile_options(${COMPONENT_LIB} PUBLIC -std=gnu++26)
