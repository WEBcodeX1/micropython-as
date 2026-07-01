idf_component_register(
    SRCS Micropython.cpp
    PRIV_REQUIRES main
    INCLUDE_DIRS "."
)

target_compile_options(${COMPONENT_LIB} PUBLIC -std=gnu++11)

add_library(micropython_lib STATIC IMPORTED)
set_property(TARGET micropython_lib PROPERTY IMPORTED_LOCATION /usr/local/lib/esp32c3/libmicropython.a)

target_link_libraries(${COMPONENT_LIB} PRIVATE micropython_lib)
