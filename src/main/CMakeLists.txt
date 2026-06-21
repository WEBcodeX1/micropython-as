idf_component_register(
    SRCS "micropython_as.cpp"
    PRIV_REQUIRES esp_wifi esp_driver_ledc pthread network_oop micropython peripherals
    INCLUDE_DIRS "."
)
