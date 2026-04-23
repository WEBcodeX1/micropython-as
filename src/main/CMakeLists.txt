idf_component_register(
    SRCS "micropython_as.cpp"
    PRIV_REQUIRES esp_wifi network_oop micropython
    INCLUDE_DIRS "."
)
