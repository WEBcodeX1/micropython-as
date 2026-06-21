set(component_srcs "Display.cpp" "TitleScreen.cpp" "GameScreen.cpp" "LEDPwm.cpp" "LEDGpio.cpp" "LED3Color.cpp")
set(component_requires "main" "esp_driver_ledc")

idf_component_register(
    SRCS "${component_srcs}"
    PRIV_REQUIRES "${component_requires}"
    REQUIRES "ssd1306"
    INCLUDE_DIRS "."
)

target_compile_options(${COMPONENT_LIB} PUBLIC -std=gnu++11)
