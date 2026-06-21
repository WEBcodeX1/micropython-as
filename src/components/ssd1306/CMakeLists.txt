set(component_srcs "ssd1306.c" "ssd1306_i2c_new.c")
set(component_priv_requires "main driver")
set(component_requires "esp_driver_i2c esp_driver_gpio")

idf_component_register(
    SRCS "${component_srcs}"
    PRIV_REQUIRES "${component_prov_requires}"
    REQUIRES "${component_requires}"
    INCLUDE_DIRS "."
)

target_compile_options(${COMPONENT_LIB} PUBLIC)
