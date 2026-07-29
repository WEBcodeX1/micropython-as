set(component_srcs "Display.cpp" "LEDPwm.cpp" "LEDGpio.cpp" "LED3Color.cpp")
set(component_requires "main" "esp_driver_ledc")

idf_component_register(
    SRCS "${component_srcs}"
    PRIV_REQUIRES "${component_requires}"
    REQUIRES "ssd1306"
    INCLUDE_DIRS "."
)
