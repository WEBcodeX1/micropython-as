idf_component_register(
    SRCS "micropython_as.c"
    PRIV_REQUIRES esp_wifi falcon_as
    INCLUDE_DIRS "."
)
