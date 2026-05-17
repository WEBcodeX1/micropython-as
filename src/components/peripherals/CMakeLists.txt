set(component_srcs "ssd1306.c" "ssd1306_i2c_new.c" "Display.cpp")
set(component_requires "main driver esp_driver_i2c esp_driver_gpio")

idf_component_register(
    SRCS "${component_srcs}"
    PRIV_REQUIRES "${component_requires}"
    INCLUDE_DIRS "."
)

target_compile_options(${COMPONENT_LIB} PUBLIC -std=gnu++11)
